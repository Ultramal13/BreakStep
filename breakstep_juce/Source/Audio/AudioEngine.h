#pragma once

#include "StepSequencer.h"
#include "DrumTrack.h"
#include "SampleChopper/AudioSlicer.h"
#include "SampleChopper/SliceSequencer.h"
#include "SongTimeline.h"
#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_dsp/juce_dsp.h>
#include <memory>
#include <vector>

namespace BreakStepAudio
{

enum class PlaybackMode
{
    PatternLoop = 0,
    SongTimeline = 1
};

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
    SongTimeline& getSongTimeline() { return songTimeline; }

    juce::AudioFormatManager& getFormatManager() { return formatManager; }

    // Playback Mode (Pattern vs Song Timeline)
    void setPlaybackMode(PlaybackMode mode) { playbackMode.store(static_cast<int>(mode)); }
    PlaybackMode getPlaybackMode() const { return static_cast<PlaybackMode>(playbackMode.load()); }

    // Independent Submix Bus Volumes (0.0 .. 1.0)
    void setDrumBusVolume(float vol) { drumBusVolume.store(juce::jlimit(0.0f, 1.0f, vol)); }
    float getDrumBusVolume() const { return drumBusVolume.load(); }

    void setSamplerBusVolume(float vol) { samplerBusVolume.store(juce::jlimit(0.0f, 1.0f, vol)); }
    float getSamplerBusVolume() const { return samplerBusVolume.load(); }

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

    // Propellerhead Reason-style Linear Song Timeline Arranger
    SongTimeline songTimeline;

    juce::AudioFormatManager formatManager;

    std::atomic<int> playbackMode { static_cast<int>(PlaybackMode::PatternLoop) };
    std::atomic<float> drumBusVolume { 0.85f };
    std::atomic<float> samplerBusVolume { 0.85f };
    std::atomic<float> delayWet { 0.0f };
    std::atomic<float> masterCutoff { 20000.0f };

    // DSP Master Effects
    juce::dsp::DelayLine<float> feedbackDelay;
    juce::dsp::Reverb globalReverb;
    juce::dsp::Reverb::Parameters reverbParams;
    juce::dsp::StateVariableTPTFilter<float> masterFilter;

    // Submix and Bus Buffers for rendering block
    juce::AudioBuffer<float> drumBusBuffer;
    juce::AudioBuffer<float> samplerBusBuffer;
    juce::AudioBuffer<float> submixBuffer;
    juce::AudioBuffer<float> reverbSendBuffer;

    double currentSampleRate = 44100.0;
};

} // namespace BreakStepAudio
