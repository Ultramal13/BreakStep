#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_audio_formats/juce_audio_formats.h>
#include <juce_dsp/juce_dsp.h>
#include <vector>
#include <atomic>
#include <mutex>

namespace BreakStepAudio
{

struct SliceMarker
{
    int startSample = 0;
    int endSample = 0;
};

class AudioSlicer
{
public:
    static constexpr int NUM_SLOTS = 4;

    AudioSlicer();
    ~AudioSlicer() = default;

    void prepareToPlay(double sampleRate, int samplesPerBlock);
    bool loadAudioFileToSlot(int slotIndex, const juce::File& file, juce::AudioFormatManager& formatManager);
    void setActiveSlot(int slotIndex);
    int getActiveSlot() const { return activeSlotIndex; }

    void resetToDefaultLoop();

    // ReCycle-grade adaptive transient detection with zero-crossing alignment
    void detectTransients(float sensitivity = 0.5f);

    // Manual slice marker manipulation & Nudge
    void addSliceMarker(int samplePosition);
    void removeSliceMarker(int sliceIndex);
    void moveSliceMarker(int sliceIndex, int newSamplePosition);
    void nudgeSliceMarker(int sliceIndex, int sampleDelta);

    void setSelectedSlice(int sliceIdx) { selectedSliceIndex = sliceIdx; }
    int getSelectedSlice() const { return selectedSliceIndex; }

    int getNumSlices() const;
    SliceMarker getSlice(int index) const;
    const std::vector<SliceMarker>& getAllSlices() const { return slices; }

    // Trigger slice playback
    void triggerSlice(int sliceIndex, float velocity = 1.0f, float pitchSemitones = 0.0f, bool reverse = false);

    // Audio block rendering
    void processBlock(juce::AudioBuffer<float>& outputBuffer, juce::AudioBuffer<float>& reverbSendBuffer, int startSample, int numSamples);

    const juce::AudioBuffer<float>& getAudioBuffer() const { return sampleSlots[activeSlotIndex]; }
    juce::String getLoadedFileName(int slot) const { return slotFileNames[slot]; }
    juce::String getLoadedFilePath(int slot) const { return slotFilePaths[slot]; }

    void setVolume(float v) { volume.store(v); }
    float getVolume() const { return volume.load(); }

    void setCutoff(float c) { cutoff.store(c); }
    float getCutoff() const { return cutoff.load(); }

    void setReverbSend(float r) { reverbSend.store(r); }
    float getReverbSend() const { return reverbSend.load(); }

private:
    void generateDefaultAmenBreak();
    int snapToZeroCrossing(int sampleIndex) const;
    void rebuildSlicesFromOnsets(std::vector<int>& onsets);

    double currentSampleRate = 44100.0;
    int activeSlotIndex = 0;
    juce::AudioBuffer<float> sampleSlots[NUM_SLOTS];
    juce::String slotFileNames[NUM_SLOTS];
    juce::String slotFilePaths[NUM_SLOTS];

    float currentSensitivity = 0.5f;
    int selectedSliceIndex = 0;

    std::vector<SliceMarker> slices;
    mutable std::mutex sliceMutex;

    // Voice playback state
    struct SliceVoice
    {
        bool active = false;
        double currentPosition = 0.0;
        int endSample = 0;
        int startSample = 0;
        float velocityGain = 1.0f;
        float playbackRate = 1.0f;
        bool isReverse = false;
    };

    static constexpr int MAX_POLYPHONY = 8;
    SliceVoice voices[MAX_POLYPHONY];

    std::atomic<float> volume { 0.85f };
    std::atomic<float> cutoff { 20000.0f };
    std::atomic<float> reverbSend { 0.1f };

    juce::dsp::StateVariableTPTFilter<float> filter;
};

} // namespace BreakStepAudio
