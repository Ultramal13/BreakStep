#include "AudioEngine.h"

namespace BreakStepAudio
{

AudioEngine::AudioEngine()
{
    formatManager.registerBasicFormats();

    // Create 6 drum tracks
    for (int t = 0; t < StepSequencer::NUM_TRACKS; ++t)
    {
        tracks.push_back(std::make_unique<DrumTrack>(static_cast<DrumTrack::TrackType>(t)));
    }

    masterFilter.setType(juce::dsp::StateVariableTPTFilterType::lowpass);

    // Default reverb settings
    reverbParams.roomSize = 0.6f;
    reverbParams.damping = 0.5f;
    reverbParams.wetLevel = 1.0f;
    reverbParams.dryLevel = 0.0f;
    reverbParams.width = 1.0f;
    globalReverb.setParameters(reverbParams);
}

DrumTrack* AudioEngine::getTrack(int index)
{
    if (index >= 0 && index < static_cast<int>(tracks.size()))
        return tracks[index].get();
    return nullptr;
}

const DrumTrack* AudioEngine::getTrack(int index) const
{
    if (index >= 0 && index < static_cast<int>(tracks.size()))
        return tracks[index].get();
    return nullptr;
}

void AudioEngine::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;

    for (auto& track : tracks)
    {
        track->prepareToPlay(sampleRate, samplesPerBlock);
    }

    slicer.prepareToPlay(sampleRate, samplesPerBlock);

    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = static_cast<juce::uint32>(samplesPerBlock);
    spec.numChannels = 2;

    masterFilter.prepare(spec);
    masterFilter.reset();

    globalReverb.prepare(spec);
    globalReverb.reset();

    feedbackDelay.prepare(spec);
    feedbackDelay.setMaximumDelayInSamples(static_cast<int>(sampleRate * 2.0));
    feedbackDelay.reset();

    // Set delay time to 8th note duration at default BPM 170
    double delayTimeSec = (60.0 / 170.0) * 0.5;
    feedbackDelay.setDelay(static_cast<float>(delayTimeSec * sampleRate));

    submixBuffer.setSize(2, samplesPerBlock);
    reverbSendBuffer.setSize(2, samplesPerBlock);
}

void AudioEngine::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill)
{
    bufferToFill.clearActiveBufferRegion();

    const int numSamples = bufferToFill.numSamples;
    const int startSample = bufferToFill.startSample;

    if (numSamples <= 0)
        return;

    submixBuffer.setSize(2, numSamples, false, false, true);
    submixBuffer.clear();

    reverbSendBuffer.setSize(2, numSamples, false, false, true);
    reverbSendBuffer.clear();

    // 1. Synchronize Slice Sequencer with Master Clock
    sliceSequencer.setBpm(sequencer.getBpm());
    sliceSequencer.setPlaying(sequencer.isPlaying());

    // 2. Process Slice Sequencer triggers & render Slicer engine
    std::vector<SliceTrigger> sliceTriggers;
    sliceSequencer.processBlock(numSamples, currentSampleRate, sliceTriggers);
    for (const auto& st : sliceTriggers)
    {
        slicer.triggerSlice(st.sliceIndex, st.velocity, st.pitchOffset, st.reverse);
    }
    slicer.processBlock(submixBuffer, reverbSendBuffer, 0, numSamples);

    // 3. Process BreakStep Drum Sequencer clock & triggers
    std::vector<StepTrigger> drumTriggers;
    sequencer.processBlock(numSamples, currentSampleRate, drumTriggers);

    for (const auto& trig : drumTriggers)
    {
        if (DrumTrack* track = getTrack(trig.trackIndex))
        {
            track->trigger(trig.velocityState);
        }
    }

    // 4. Check if any drum track is soloed
    bool globalSoloActive = false;
    for (const auto& track : tracks)
    {
        if (track->getSoloed())
        {
            globalSoloActive = true;
            break;
        }
    }

    // 5. Render drum tracks into submix & reverb send buffers
    for (auto& track : tracks)
    {
        track->processBlock(submixBuffer, reverbSendBuffer, 0, numSamples, globalSoloActive);
    }

    // 6. Process Global Reverb
    juce::dsp::AudioBlock<float> revBlock(reverbSendBuffer);
    juce::dsp::ProcessContextReplacing<float> revContext(revBlock);
    globalReverb.process(revContext);

    // Mix wet reverb into submix
    submixBuffer.addFrom(0, 0, reverbSendBuffer, 0, 0, numSamples);
    submixBuffer.addFrom(1, 0, reverbSendBuffer, 1, 0, numSamples);

    // 7. Process Feedback Delay
    float wet = delayWet.load();
    if (wet > 0.001f)
    {
        double currentBpm = sequencer.getBpm();
        double delayTimeSec = (60.0 / currentBpm) * 0.5;
        feedbackDelay.setDelay(static_cast<float>(delayTimeSec * currentSampleRate));

        for (int i = 0; i < numSamples; ++i)
        {
            for (int ch = 0; ch < 2; ++ch)
            {
                float inputSample = submixBuffer.getSample(ch, i);
                float delayedSample = feedbackDelay.popSample(ch);

                feedbackDelay.pushSample(ch, inputSample + delayedSample * 0.3f);
                submixBuffer.addFrom(ch, i, &delayedSample, 1, wet);
            }
        }
    }

    // 8. Process Master Filter
    masterFilter.setCutoffFrequency(masterCutoff.load());
    juce::dsp::AudioBlock<float> masterBlock(submixBuffer);
    juce::dsp::ProcessContextReplacing<float> masterContext(masterBlock);
    masterFilter.process(masterContext);

    // 9. Output to final audio buffer
    for (int ch = 0; ch < bufferToFill.buffer->getNumChannels(); ++ch)
    {
        if (ch < 2)
        {
            bufferToFill.buffer->addFrom(ch, startSample, submixBuffer, ch, 0, numSamples);
        }
    }
}

void AudioEngine::releaseResources()
{
}

bool AudioEngine::saveProjectToFile(const juce::File& file)
{
    auto* rootObj = new juce::DynamicObject();
    rootObj->setProperty("version", "3.0");
    rootObj->setProperty("bpm", sequencer.getBpm());
    rootObj->setProperty("swing", sequencer.getSwing());
    rootObj->setProperty("delayWet", delayWet.load());
    rootObj->setProperty("masterCutoff", masterCutoff.load());

    // Save Slicer & Slice Sequencer state
    auto* slicerObj = new juce::DynamicObject();
    slicerObj->setProperty("loadedFilePath", slicer.getLoadedFilePath(slicer.getActiveSlot()));
    slicerObj->setProperty("activeSlot", slicer.getActiveSlot());
    slicerObj->setProperty("volume", slicer.getVolume());
    slicerObj->setProperty("cutoff", slicer.getCutoff());
    slicerObj->setProperty("reverbSend", slicer.getReverbSend());
    slicerObj->setProperty("sliceSequence", sliceSequencer.saveStateToVar());
    rootObj->setProperty("slicer", juce::var(slicerObj));

    // Save Drum Tracks
    juce::Array<juce::var> trackArray;
    for (int t = 0; t < StepSequencer::NUM_TRACKS; ++t)
    {
        if (auto* track = getTrack(t))
        {
            auto trackVar = track->saveStateToVar();

            juce::Array<juce::var> stepArray;
            for (int s = 0; s < StepSequencer::NUM_STEPS; ++s)
            {
                stepArray.add(sequencer.getStepState(t, s));
            }
            trackVar.getDynamicObject()->setProperty("steps", stepArray);

            trackArray.add(trackVar);
        }
    }
    rootObj->setProperty("tracks", trackArray);

    juce::String jsonText = juce::JSON::toString(juce::var(rootObj), true);
    return file.replaceWithText(jsonText);
}

bool AudioEngine::loadProjectFromFile(const juce::File& file)
{
    juce::var parsedJson = juce::JSON::parse(file);
    if (!parsedJson.isObject())
        return false;

    sequencer.setBpm(parsedJson.getProperty("bpm", 170.0));
    sequencer.setSwing(parsedJson.getProperty("swing", 0.0));
    setDelayWet(static_cast<float>(parsedJson.getProperty("delayWet", 0.0)));
    setMasterCutoff(static_cast<float>(parsedJson.getProperty("masterCutoff", 20000.0)));

    // Restore Slicer
    auto slicerVar = parsedJson.getProperty("slicer", juce::var());
    if (slicerVar.isObject())
    {
        int slot = static_cast<int>(slicerVar.getProperty("activeSlot", 0));
        juce::String filePath = slicerVar.getProperty("loadedFilePath", "").toString();
        if (filePath.isNotEmpty())
        {
            juce::File audioFile(filePath);
            if (audioFile.existsAsFile())
                slicer.loadAudioFileToSlot(slot, audioFile, formatManager);
        }
        slicer.setActiveSlot(slot);
        slicer.setVolume(static_cast<float>(slicerVar.getProperty("volume", 0.85)));
        slicer.setCutoff(static_cast<float>(slicerVar.getProperty("cutoff", 20000.0)));
        slicer.setReverbSend(static_cast<float>(slicerVar.getProperty("reverbSend", 0.1)));

        auto seqVar = slicerVar.getProperty("sliceSequence", juce::var());
        if (seqVar.isArray())
            sliceSequencer.loadStateFromVar(seqVar);
    }

    // Restore Drum Tracks
    auto trackArray = parsedJson.getProperty("tracks", juce::var());
    if (trackArray.isArray())
    {
        for (int t = 0; t < trackArray.size() && t < StepSequencer::NUM_TRACKS; ++t)
        {
            auto trackVar = trackArray[t];
            if (auto* track = getTrack(t))
            {
                track->loadStateFromVar(trackVar, formatManager);
            }

            if (trackVar.isObject())
            {
                auto stepsArray = trackVar.getProperty("steps", juce::var());
                if (stepsArray.isArray())
                {
                    for (int s = 0; s < stepsArray.size() && s < StepSequencer::NUM_STEPS; ++s)
                    {
                        sequencer.setStepState(t, s, static_cast<int>(stepsArray[s]));
                    }
                }
            }
        }
    }
    return true;
}

} // namespace BreakStepAudio
