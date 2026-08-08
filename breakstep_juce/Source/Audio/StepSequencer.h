#pragma once

#include <juce_core/juce_core.h>
#include <vector>
#include <atomic>

namespace BreakStepAudio
{

struct StepTrigger
{
    int trackIndex;
    int velocityState; // 1 = Normal, 2 = Accent
    int sampleOffset;  // Sample offset inside the audio block
};

class StepSequencer
{
public:
    static constexpr int NUM_TRACKS = 6;
    static constexpr int NUM_STEPS = 32;

    StepSequencer();

    void setBpm(double newBpm) { bpm.store(newBpm); }
    double getBpm() const { return bpm.load(); }

    void setSwing(double newSwing) { swing.store(newSwing); }
    double getSwing() const { return swing.load(); }

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

    // Step state: 0 = Off, 1 = Normal, 2 = Accent
    int getStepState(int trackIndex, int stepIndex) const;
    void setStepState(int trackIndex, int stepIndex, int state);
    void toggleStepState(int trackIndex, int stepIndex);

    // Clear and Randomize functions
    void clearTrack(int trackIndex);
    void clearAllTracks();
    void randomizeTrack(int trackIndex);
    void randomizeAllTracks();

    int getCurrentStep() const { return currentStep.load(); }

    void processBlock(int numSamples, double sampleRate, std::vector<StepTrigger>& outTriggers);

private:
    std::atomic<double> bpm { 170.0 };
    std::atomic<double> swing { 0.0 };
    std::atomic<bool> playing { false };
    std::atomic<int> currentStep { -1 };

    int stepGrid[NUM_TRACKS][NUM_STEPS];

    double sampleCounter = 0.0;
    int internalStep = 0;
    juce::Random random;
};

} // namespace BreakStepAudio
