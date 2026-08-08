#pragma once

#include <juce_core/juce_core.h>
#include <vector>
#include <atomic>

namespace BreakStepAudio
{

struct SliceTrigger
{
    int sliceIndex;
    float velocity;
    float pitchOffset;
    bool reverse;
    int sampleOffset;
};

struct SliceStep
{
    bool active = false;
    int sliceIndex = 0;        // Mapped slice chop (0..15)
    int ratchets = 1;          // 1x, 2x, 3x, 4x sub-step rolls
    float probability = 1.0f;  // 0.25, 0.5, 0.75, 1.0
    bool reverse = false;      // Reverse slice playback
    int pitchOffset = 0;       // -12 .. +12 semitones
};

class SliceSequencer
{
public:
    static constexpr int NUM_STEPS = 16;

    SliceSequencer();

    void setBpm(double newBpm) { bpm.store(newBpm); }
    double getBpm() const { return bpm.load(); }

    void setPlaying(bool play)
    {
        playing.store(play);
        if (!play) currentStep.store(-1);
    }
    bool isPlaying() const { return playing.load(); }

    void resetPosition()
    {
        sampleCounter = 0.0;
        internalStep = 0;
        currentStep.store(-1);
    }

    SliceStep getStep(int index) const;
    void setStep(int index, const SliceStep& step);
    void toggleStepActive(int index);
    void setStepSlice(int index, int sliceIndex);
    void cycleRatchets(int index);
    void cycleProbability(int index);
    void toggleReverse(int index);

    // Clear and Randomize for Chop Track
    void clear();
    void randomize();

    int getCurrentStep() const { return currentStep.load(); }

    void processBlock(int numSamples, double sampleRate, std::vector<SliceTrigger>& outTriggers);

    // Serialization
    juce::var saveStateToVar() const;
    void loadStateFromVar(const juce::var& stateVar);

private:
    std::atomic<double> bpm { 170.0 };
    std::atomic<bool> playing { false };
    std::atomic<int> currentStep { -1 };

    SliceStep steps[NUM_STEPS];

    double sampleCounter = 0.0;
    int internalStep = 0;
    juce::Random random;
};

} // namespace BreakStepAudio
