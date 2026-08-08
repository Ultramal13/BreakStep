#pragma once

#include "StepSequencer.h"
#include "DrumTrack.h"
#include "SampleChopper/AudioSlicer.h"
#include "SampleChopper/SliceSequencer.h"
#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_dsp/juce_dsp.h>
#include <memory>
#include <vector>

namespace BreakStepAudio
{

class AudioEngine
{
public:
    AudioEngine();
    ~AudioEngine() = default;

    void prepareToPlay(double sampleRate, int samplesPerBlock);
    void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill);
    void releaseResources();

    StepSequencer& getSequencer() { return sequencer; }
    DrumTrack* getTrack(int index);
    const DrumTrack* getTrack(int index) const;

    AudioSlicer& getSlicer() { return slicer; }
    SliceSequencer& getSliceSequencer() { return sliceSequencer; }

    juce::AudioFormatManager& getFormatManager() { return formatManager; }

    // Master FX controls
    void setDelayWet(float wet) { delayWet.store(wet); }
    float getDelayWet() const { return delayWet.load(); }

    void setMasterCutoff(float freq) { masterCutoff.store(freq); }
    float getMasterCutoff() const { return masterCutoff.load(); }

    // Project Save / Load (.breakstep / .seq)
    bool saveProjectToFile(const juce::File& file);
    bool loadProjectFromFile(const juce::File& file);

private:
    StepSequencer sequencer;
    std::vector<std::unique_ptr<DrumTrack>> tracks;

    // Master MPC Sample Chopper & Slice Sequencer Engine
    AudioSlicer slicer;
    SliceSequencer sliceSequencer;

    juce::AudioFormatManager formatManager;

    std::atomic<float> delayWet { 0.0f };
    std::atomic<float> masterCutoff { 20000.0f };

    // DSP Master Effects
    juce::dsp::DelayLine<float> feedbackDelay;
    juce::dsp::Reverb globalReverb;
    juce::dsp::Reverb::Parameters reverbParams;
    juce::dsp::StateVariableTPTFilter<float> masterFilter;

    // Temporary buffers for rendering block
    juce::AudioBuffer<float> submixBuffer;
    juce::AudioBuffer<float> reverbSendBuffer;

    double currentSampleRate = 44100.0;
};

} // namespace BreakStepAudio
