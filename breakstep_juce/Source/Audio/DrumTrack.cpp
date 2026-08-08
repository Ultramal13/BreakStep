#include "DrumTrack.h"
#include <cmath>

namespace BreakStepAudio
{

DrumTrack::DrumTrack(TrackType type)
    : trackType(type)
{
    filter.setType(juce::dsp::StateVariableTPTFilterType::lowpass);
}

juce::String DrumTrack::getDefaultTrackName() const
{
    switch (trackType)
    {
        case Kick:    return "KICK";
        case Snare:   return "SNARE";
        case Hat:     return "HAT";
        case OpenHat: return "OPEN HAT";
        case Clap:    return "CLAP";
        case Perc:    return "PERC";
        default:      return "TRACK";
    }
}

void DrumTrack::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;

    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = (juce::uint32) samplesPerBlock;
    spec.numChannels = 2;

    filter.prepare(spec);
    filter.reset();

    vintageDSP.prepare(sampleRate);
    vintageDSP.reset();

    generateDefaultSynthBuffer();
    if (userSampleBuffer.getNumSamples() == 0)
    {
        activeBuffer = &synthBuffer;
        sampleName = "synth";
        sampleFilePath = "";
    }
}

void DrumTrack::generateDefaultSynthBuffer()
{
    double durationSec = (trackType == OpenHat) ? 0.45 : 0.35;
    int numSamples = static_cast<int>(currentSampleRate * durationSec);
    synthBuffer.setSize(2, numSamples);
    synthBuffer.clear();

    float* left = synthBuffer.getWritePointer(0);
    float* right = synthBuffer.getWritePointer(1);

    juce::Random random;

    for (int i = 0; i < numSamples; ++i)
    {
        double t = static_cast<double>(i) / currentSampleRate;
        float sample = 0.0f;

        switch (trackType)
        {
            case Kick:
            {
                // Sine sweep 150Hz -> 45Hz
                double freq = 45.0 + 105.0 * std::exp(-t * 35.0);
                double phase = 2.0 * juce::MathConstants<double>::pi * (45.0 * t + (105.0 / 35.0) * (1.0 - std::exp(-t * 35.0)));
                float env = std::exp(-t * 12.0);
                sample = std::sin(phase) * env;
                break;
            }
            case Snare:
            {
                // Noise + Tone
                float noise = (random.nextFloat() * 2.0f - 1.0f) * std::exp(-t * 22.0);
                double toneFreq = 190.0 * std::exp(-t * 15.0);
                double phase = 2.0 * juce::MathConstants<double>::pi * toneFreq * t;
                float tone = std::sin(phase) * std::exp(-t * 35.0);
                sample = noise * 0.7f + tone * 0.5f;
                break;
            }
            case Hat:
            {
                // Highpass noise
                float noise = (random.nextFloat() * 2.0f - 1.0f);
                float env = std::exp(-t * 70.0);
                sample = noise * env * 0.5f;
                break;
            }
            case OpenHat:
            {
                float noise = (random.nextFloat() * 2.0f - 1.0f);
                float env = std::exp(-t * 12.0);
                sample = noise * env * 0.5f;
                break;
            }
            case Clap:
            {
                float noise = (random.nextFloat() * 2.0f - 1.0f);
                float env = 0.0f;
                if (t < 0.01) env = 0.8f;
                else if (t < 0.02) env = 0.4f;
                else if (t < 0.03) env = 0.9f;
                else env = std::exp(-(t - 0.03) * 25.0);
                sample = noise * env * 0.6f;
                break;
            }
            case Perc:
            {
                double freq = 180.0 + 240.0 * std::exp(-t * 30.0);
                double phase = 2.0 * juce::MathConstants<double>::pi * (180.0 * t + (240.0 / 30.0) * (1.0 - std::exp(-t * 30.0)));
                float square = (std::sin(phase) >= 0 ? 0.6f : -0.6f);
                float env = std::exp(-t * 28.0);
                sample = square * env;
                break;
            }
            default:
                break;
        }

        left[i] = sample;
        right[i] = sample;
    }
}

void DrumTrack::trigger(int velocityState)
{
    if (velocityState <= 0)
        return;

    readPosition = 0.0;
    currentVelocityGain = (velocityState == 2) ? 1.0f : 0.6f;
    currentEnvSamples = 0.0;
    isPlaying.store(true);
}

bool DrumTrack::loadSampleFile(const juce::File& file, juce::AudioFormatManager& formatManager)
{
    std::unique_ptr<juce::AudioFormatReader> reader(formatManager.createReaderFor(file));
    if (reader != nullptr)
    {
        userSampleBuffer.setSize((int) reader->numChannels, (int) reader->lengthInSamples);
        reader->read(&userSampleBuffer, 0, (int) reader->lengthInSamples, 0, true, true);

        // Ensure stereo
        if (userSampleBuffer.getNumChannels() == 1)
        {
            juce::AudioBuffer<float> stereoBuffer(2, userSampleBuffer.getNumSamples());
            stereoBuffer.copyFrom(0, 0, userSampleBuffer, 0, 0, userSampleBuffer.getNumSamples());
            stereoBuffer.copyFrom(1, 0, userSampleBuffer, 0, 0, userSampleBuffer.getNumSamples());
            userSampleBuffer = std::move(stereoBuffer);
        }

        activeBuffer = &userSampleBuffer;
        sampleName = file.getFileName();
        sampleFilePath = file.getFullPathName();
        return true;
    }
    return false;
}

void DrumTrack::resetToDefaultSynth()
{
    activeBuffer = &synthBuffer;
    sampleName = "synth";
    sampleFilePath = "";
}

void DrumTrack::processBlock(juce::AudioBuffer<float>& trackOutputBuffer,
                            juce::AudioBuffer<float>& reverbSendBuffer,
                            int startSample,
                            int numSamples,
                            bool globalSoloActive)
{
    if (!isPlaying.load() || activeBuffer == nullptr || activeBuffer->getNumSamples() == 0)
        return;

    // Solo/Mute logic
    bool shouldPlay = globalSoloActive ? isSoloed.load() : !isMuted.load();
    if (!shouldPlay)
        return;

    float vol = volume.load() * currentVelocityGain;
    float p = pitch.load();
    float playbackRate = std::pow(2.0f, p / 12.0f);
    float atkTimeSec = attack.load();
    int atkSamples = std::max(1, static_cast<int>(currentSampleRate * atkTimeSec));

    float lenFraction = length.load();
    int totalBufferSamples = activeBuffer->getNumSamples();
    int maxAllowedSamples = std::max(64, static_cast<int>(totalBufferSamples * lenFraction));

    filter.setCutoffFrequency(cutoff.load());

    const int totalSourceSamples = activeBuffer->getNumSamples();
    const float* const* srcData = activeBuffer->getArrayOfReadPointers();

    // Temporary scratch block for processing
    juce::AudioBuffer<float> tempBuffer(2, numSamples);
    tempBuffer.clear();
    float* const* dstData = tempBuffer.getArrayOfWritePointers();

    float currentCrunch = crunch.load();

    for (int i = 0; i < numSamples; ++i)
    {
        if (readPosition >= maxAllowedSamples || readPosition >= totalSourceSamples)
        {
            isPlaying.store(false);
            break;
        }

        // Linear interpolation playback
        int posInt = static_cast<int>(readPosition);
        float frac = static_cast<float>(readPosition - posInt);
        int nextPosInt = std::min(posInt + 1, totalSourceSamples - 1);

        // Attack envelope
        float envGain = 1.0f;
        if (currentEnvSamples < atkSamples)
        {
            envGain = static_cast<float>(currentEnvSamples / atkSamples);
        }

        // Fade out at length end
        if (readPosition > maxAllowedSamples - 128)
        {
            float fadeProgress = (maxAllowedSamples - static_cast<float>(readPosition)) / 128.0f;
            envGain *= juce::jlimit(0.0f, 1.0f, fadeProgress);
        }

        float totalGain = vol * envGain;

        float sLeft = (srcData[0][posInt] + frac * (srcData[0][nextPosInt] - srcData[0][posInt])) * totalGain;
        float sRight = (srcData[1][posInt] + frac * (srcData[1][nextPosInt] - srcData[1][posInt])) * totalGain;

        // Apply Vintage Sampler DSP (MPC-60, MPC-3000, EPS-16+ Crunch/Saturation)
        vintageDSP.process(sLeft, sRight, vintageMode, currentCrunch);

        dstData[0][i] = sLeft;
        dstData[1][i] = sRight;

        readPosition += playbackRate;
        currentEnvSamples += 1.0;
    }

    // Apply per-track lowpass filter
    juce::dsp::AudioBlock<float> block(tempBuffer);
    juce::dsp::ProcessContextReplacing<float> context(block);
    filter.process(context);

    // Mix into track main buffer and reverb send buffer
    float revSend = reverbSend.load();
    for (int ch = 0; ch < trackOutputBuffer.getNumChannels(); ++ch)
    {
        trackOutputBuffer.addFrom(ch, startSample, tempBuffer, ch, 0, numSamples);

        if (revSend > 0.001f && ch < reverbSendBuffer.getNumChannels())
        {
            reverbSendBuffer.addFrom(ch, startSample, tempBuffer, ch, 0, numSamples, revSend);
        }
    }
}

juce::var DrumTrack::saveStateToVar() const
{
    auto* obj = new juce::DynamicObject();
    obj->setProperty("id", static_cast<int>(trackType));
    obj->setProperty("name", getDefaultTrackName());
    obj->setProperty("sampleName", sampleName);
    obj->setProperty("samplePath", sampleFilePath);
    obj->setProperty("volume", volume.load());
    obj->setProperty("pitch", pitch.load());
    obj->setProperty("attack", attack.load());
    obj->setProperty("cutoff", cutoff.load());
    obj->setProperty("length", length.load());
    obj->setProperty("reverbSend", reverbSend.load());
    obj->setProperty("vintageMode", static_cast<int>(vintageMode));
    obj->setProperty("crunch", crunch.load());
    obj->setProperty("mute", isMuted.load());
    obj->setProperty("solo", isSoloed.load());
    return juce::var(obj);
}

void DrumTrack::loadStateFromVar(const juce::var& stateVar, juce::AudioFormatManager& formatManager)
{
    if (!stateVar.isObject())
        return;

    volume.store(static_cast<float>(stateVar.getProperty("volume", 0.8)));
    pitch.store(static_cast<float>(stateVar.getProperty("pitch", 0.0)));
    attack.store(static_cast<float>(stateVar.getProperty("attack", 0.001)));
    cutoff.store(static_cast<float>(stateVar.getProperty("cutoff", 20000.0)));
    length.store(static_cast<float>(stateVar.getProperty("length", 1.0)));
    reverbSend.store(static_cast<float>(stateVar.getProperty("reverbSend", 0.0)));
    crunch.store(static_cast<float>(stateVar.getProperty("crunch", 0.0)));
    vintageMode = static_cast<VintageMode>(static_cast<int>(stateVar.getProperty("vintageMode", 0)));
    isMuted.store(static_cast<bool>(stateVar.getProperty("mute", false)));
    isSoloed.store(static_cast<bool>(stateVar.getProperty("solo", false)));

    juce::String savedPath = stateVar.getProperty("samplePath", "").toString();
    if (savedPath.isNotEmpty())
    {
        juce::File sampleFile(savedPath);
        if (sampleFile.existsAsFile())
        {
            loadSampleFile(sampleFile, formatManager);
        }
        else
        {
            resetToDefaultSynth();
        }
    }
    else
    {
        resetToDefaultSynth();
    }
}

} // namespace BreakStepAudio
