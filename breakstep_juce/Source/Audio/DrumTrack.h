#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_audio_formats/juce_audio_formats.h>
#include <juce_dsp/juce_dsp.h>
#include "VintageSamplerDSP.h"
#include <atomic>

namespace BreakStepAudio
{

class DrumTrack
{
public:
    enum TrackType
    {
        Kick = 0,
        Snare,
        Hat,
        OpenHat,
        Clap,
        Perc,
        Count
    };

    DrumTrack(TrackType type);

    void prepareToPlay(double sampleRate, int samplesPerBlock);

    void trigger(int velocityState);

    // Render audio block for this track into output buffers
    void processBlock(juce::AudioBuffer<float>& trackOutputBuffer,
                      juce::AudioBuffer<float>& reverbSendBuffer,
                      int startSample,
                      int numSamples,
                      bool globalSoloActive);

    // Audio sample loading
    bool loadSampleFile(const juce::File& file, juce::AudioFormatManager& formatManager);
    void resetToDefaultSynth();

    // Parameter getters and setters
    void setVolume(float v) { volume.store(v); }
    float getVolume() const { return volume.load(); }

    void setPitch(float p) { pitch.store(p); }
    float getPitch() const { return pitch.load(); }

    void setAttack(float a) { attack.store(a); }
    float getAttack() const { return attack.load(); }

    void setCutoff(float c) { cutoff.store(c); }
    float getCutoff() const { return cutoff.load(); }

    void setLength(float l) { length.store(l); }
    float getLength() const { return length.load(); }

    void setReverbSend(float r) { reverbSend.store(r); }
    float getReverbSend() const { return reverbSend.load(); }

    void setMuted(bool m) { isMuted.store(m); }
    bool getMuted() const { return isMuted.load(); }

    void setSoloed(bool s) { isSoloed.store(s); }
    bool getSoloed() const { return isSoloed.load(); }

    // Vintage Sampler Character (EPS-16+, MPC-60, MPC-3000)
    VintageMode getVintageMode() const { return vintageMode; }
    void setVintageMode(VintageMode m) { vintageMode = m; }
    void cycleVintageMode() { vintageMode = VintageSamplerDSP::getNextMode(vintageMode); }

    void setCrunch(float c) { crunch.store(c); }
    float getCrunch() const { return crunch.load(); }

    juce::String getSampleName() const { return sampleName; }
    juce::String getSampleFilePath() const { return sampleFilePath; }
    TrackType getType() const { return trackType; }
    juce::String getDefaultTrackName() const;

    // JSON Serialization
    juce::var saveStateToVar() const;
    void loadStateFromVar(const juce::var& stateVar, juce::AudioFormatManager& formatManager);

private:
    void generateDefaultSynthBuffer();

    TrackType trackType;
    double currentSampleRate = 44100.0;

    juce::AudioBuffer<float> synthBuffer;
    juce::AudioBuffer<float> userSampleBuffer;
    juce::AudioBuffer<float>* activeBuffer = nullptr;

    juce::String sampleName = "synth";
    juce::String sampleFilePath = "";

    std::atomic<float> volume { 0.8f };
    std::atomic<float> pitch { 0.0f };
    std::atomic<float> attack { 0.001f };
    std::atomic<float> cutoff { 20000.0f };
    std::atomic<float> length { 1.0f };
    std::atomic<float> reverbSend { 0.0f };
    std::atomic<float> crunch { 0.0f };
    std::atomic<bool> isMuted { false };
    std::atomic<bool> isSoloed { false };

    VintageMode vintageMode = VintageMode::Clean;
    VintageSamplerDSP vintageDSP;

    // Playback state
    std::atomic<bool> isPlaying { false };
    double readPosition = 0.0;
    float currentVelocityGain = 1.0f;
    double currentEnvSamples = 0.0;

    // Filter DSP
    juce::dsp::StateVariableTPTFilter<float> filter;
};

} // namespace BreakStepAudio
