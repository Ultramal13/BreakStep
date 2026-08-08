#pragma once

#include <juce_core/juce_core.h>
#include <atomic>
#include <vector>

namespace BreakStepAudio
{

class StepSequencer;
class SliceSequencer;

class SongTimeline
{
public:
    static constexpr int NUM_BARS = 32;

    SongTimeline();

    void setDrumBlock(int barIndex, int patternIndex);
    int getDrumBlock(int barIndex) const;

    void setSlicerBlock(int barIndex, int patternIndex);
    int getSlicerBlock(int barIndex) const;

    void clearTimeline();
    void fillStandardArrangement();

    int getCurrentBar() const { return currentBar.load(); }
    float getBarProgress() const { return barProgress.load(); }

    void resetPosition()
    {
        sampleCounter = 0.0;
        internalBar = 0;
        currentBar.store(0);
        barProgress.store(0.0f);
    }

    void processBlock(int numSamples, double sampleRate, double bpm, StepSequencer& drums, SliceSequencer& slicer);

    juce::var saveStateToVar() const;
    void loadStateFromVar(const juce::var& stateVar);

private:
    int drumBlocks[NUM_BARS];    // 0 = Mute, 1..8 = Pattern 1..8
    int slicerBlocks[NUM_BARS];  // 0 = Mute, 1..8 = Pattern 1..8

    std::atomic<int> currentBar { 0 };
    std::atomic<float> barProgress { 0.0f };

    double sampleCounter = 0.0;
    int internalBar = 0;
};

} // namespace BreakStepAudio
