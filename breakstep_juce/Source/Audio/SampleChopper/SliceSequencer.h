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

struct SlicePattern
{
    SliceStep steps[16];
};

class SliceSequencer
{
public:
    static constexpr int NUM_STEPS = 16;
    static constexpr int NUM_PATTERNS = 8;

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

    // Pattern Bank Management (0..7)
    void setActivePattern(int patternIndex);
    int getActivePattern() const { return activePatternIndex.load(); }

    SliceStep getStep(int index) const;
    void setStep(int index, const SliceStep& step);
    void toggleStepActive(int index);
    void setStepSlice(int index, int sliceIndex);
    void cycleRatchets(int index);
    void cycleProbability(int index);
    void toggleReverse(int index);

    SliceStep getPatternStep(int ptnIndex, int stepIndex) const;
    void setPatternStep(int ptnIndex, int stepIndex, const SliceStep& step);

    // Clear and Randomize
    void clear();
    void clearPattern(int ptnIndex);
    void randomize();
    void randomizePattern(int ptnIndex);
    void copyPattern(int srcPattern, int dstPattern);

    int getCurrentStep() const { return currentStep.load(); }

    void processBlock(int numSamples, double sampleRate, std::vector<SliceTrigger>& outTriggers);

    // Serialization
    juce::var saveStateToVar() const;
    void loadStateFromVar(const juce::var& stateVar);

private:
    std::atomic<double> bpm { 170.0 };
    std::atomic<bool> playing { false };
    std::atomic<int> currentStep { -1 };
    std::atomic<int> activePatternIndex { 0 };

    SlicePattern patterns[NUM_PATTERNS];

    double sampleCounter = 0.0;
    int internalStep = 0;
    juce::Random random;
};

} // namespace BreakStepAudio
