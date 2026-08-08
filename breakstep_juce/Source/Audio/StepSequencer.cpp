#include "StepSequencer.h"

namespace BreakStepAudio
{

StepSequencer::StepSequencer()
{
    // Initialize default Pattern 0 with a classic DnB 2-step
    randomizePattern(0);
}

void StepSequencer::setActivePattern(int patternIndex)
{
    activePatternIndex.store(juce::jlimit(0, NUM_PATTERNS - 1, patternIndex));
}

int StepSequencer::getStepState(int trackIndex, int stepIndex) const
{
    int ptn = activePatternIndex.load();
    return getPatternStepState(ptn, trackIndex, stepIndex);
}

void StepSequencer::setStepState(int trackIndex, int stepIndex, int state)
{
    int ptn = activePatternIndex.load();
    setPatternStepState(ptn, trackIndex, stepIndex, state);
}

void StepSequencer::toggleStepState(int trackIndex, int stepIndex)
{
    int cur = getStepState(trackIndex, stepIndex);
    setStepState(trackIndex, stepIndex, (cur + 1) % 3);
}

int StepSequencer::getPatternStepState(int ptnIndex, int trackIndex, int stepIndex) const
{
    if (ptnIndex >= 0 && ptnIndex < NUM_PATTERNS &&
        trackIndex >= 0 && trackIndex < NUM_TRACKS &&
        stepIndex >= 0 && stepIndex < NUM_STEPS)
    {
        return patterns[ptnIndex].steps[trackIndex][stepIndex];
    }
    return 0;
}

void StepSequencer::setPatternStepState(int ptnIndex, int trackIndex, int stepIndex, int state)
{
    if (ptnIndex >= 0 && ptnIndex < NUM_PATTERNS &&
        trackIndex >= 0 && trackIndex < NUM_TRACKS &&
        stepIndex >= 0 && stepIndex < NUM_STEPS)
    {
        patterns[ptnIndex].steps[trackIndex][stepIndex] = juce::jlimit(0, 2, state);
    }
}

void StepSequencer::clearTrack(int trackIndex)
{
    int ptn = activePatternIndex.load();
    if (trackIndex >= 0 && trackIndex < NUM_TRACKS)
    {
        for (int s = 0; s < NUM_STEPS; ++s)
            patterns[ptn].steps[trackIndex][s] = 0;
    }
}

void StepSequencer::clearPattern(int ptnIndex)
{
    if (ptnIndex >= 0 && ptnIndex < NUM_PATTERNS)
    {
        for (int t = 0; t < NUM_TRACKS; ++t)
            for (int s = 0; s < NUM_STEPS; ++s)
                patterns[ptnIndex].steps[t][s] = 0;
    }
}

void StepSequencer::clearAllTracks()
{
    int ptn = activePatternIndex.load();
    clearPattern(ptn);
}

void StepSequencer::randomizeTrack(int trackIndex)
{
    int ptn = activePatternIndex.load();
    if (trackIndex >= 0 && trackIndex < NUM_TRACKS)
    {
        for (int s = 0; s < NUM_STEPS; ++s)
        {
            float roll = random.nextFloat();
            if (trackIndex == 0) // Kick
            {
                patterns[ptn].steps[trackIndex][s] = (s == 0 || s == 10 || s == 16 || s == 26 || (roll > 0.82f && s % 2 == 0)) ? ((s == 0 || s == 16) ? 2 : 1) : 0;
            }
            else if (trackIndex == 1) // Snare
            {
                patterns[ptn].steps[trackIndex][s] = (s == 4 || s == 12 || s == 20 || s == 28 || (roll > 0.9f)) ? ((s == 4 || s == 12) ? 2 : 1) : 0;
            }
            else if (trackIndex == 2) // Closed Hat
            {
                patterns[ptn].steps[trackIndex][s] = (s % 2 == 0) ? ((s % 4 == 0) ? 2 : 1) : (roll > 0.65f ? 1 : 0);
            }
            else if (trackIndex == 3) // Open Hat
            {
                patterns[ptn].steps[trackIndex][s] = (s == 2 || s == 6 || s == 14 || s == 18 || s == 22 || s == 30) ? 2 : 0;
            }
            else if (trackIndex == 4) // Clap
            {
                patterns[ptn].steps[trackIndex][s] = (s == 12 || s == 28) ? 2 : 0;
            }
            else // Perc
            {
                patterns[ptn].steps[trackIndex][s] = (roll > 0.75f && s % 2 != 0) ? 1 : 0;
            }
        }
    }
}

void StepSequencer::randomizePattern(int ptnIndex)
{
    if (ptnIndex >= 0 && ptnIndex < NUM_PATTERNS)
    {
        for (int t = 0; t < NUM_TRACKS; ++t)
        {
            for (int s = 0; s < NUM_STEPS; ++s)
            {
                float roll = random.nextFloat();
                if (t == 0) // Kick
                {
                    patterns[ptnIndex].steps[t][s] = (s == 0 || s == 10 || s == 16 || s == 26 || (roll > 0.85f && s % 2 == 0)) ? ((s == 0 || s == 16) ? 2 : 1) : 0;
                }
                else if (t == 1) // Snare
                {
                    patterns[ptnIndex].steps[t][s] = (s == 4 || s == 12 || s == 20 || s == 28) ? 2 : (roll > 0.92f ? 1 : 0);
                }
                else if (t == 2) // Closed Hat
                {
                    patterns[ptnIndex].steps[t][s] = (s % 2 == 0) ? ((s % 4 == 0) ? 2 : 1) : (roll > 0.65f ? 1 : 0);
                }
                else if (t == 3) // Open Hat
                {
                    patterns[ptnIndex].steps[t][s] = (s == 6 || s == 22) ? 2 : 0;
                }
                else if (t == 4) // Clap
                {
                    patterns[ptnIndex].steps[t][s] = (s == 12 || s == 28) ? 2 : 0;
                }
                else // Perc
                {
                    patterns[ptnIndex].steps[t][s] = (roll > 0.8f && s % 2 != 0) ? 1 : 0;
                }
            }
        }
    }
}

void StepSequencer::randomizeAllTracks()
{
    int ptn = activePatternIndex.load();
    randomizePattern(ptn);
}

void StepSequencer::copyPattern(int srcPattern, int dstPattern)
{
    if (srcPattern >= 0 && srcPattern < NUM_PATTERNS &&
        dstPattern >= 0 && dstPattern < NUM_PATTERNS)
    {
        patterns[dstPattern] = patterns[srcPattern];
    }
}

void StepSequencer::processBlock(int numSamples, double sampleRate, std::vector<StepTrigger>& outTriggers)
{
    outTriggers.clear();

    if (!playing.load() || sampleRate <= 0.0)
        return;

    const double currentBpm = bpm.load();
    const double currentSwing = swing.load();
    const double samplesPer16th = (sampleRate * 60.0) / (currentBpm * 4.0);

    const int ptn = activePatternIndex.load();

    for (int i = 0; i < numSamples; ++i)
    {
        if (sampleCounter == 0.0)
        {
            currentStep.store(internalStep);

            for (int t = 0; t < NUM_TRACKS; ++t)
            {
                int state = patterns[ptn].steps[t][internalStep];
                if (state > 0)
                {
                    outTriggers.push_back({ t, state, i });
                }
            }
        }

        sampleCounter += 1.0;

        double stepDuration = samplesPer16th;
        if (internalStep % 2 == 0)
        {
            stepDuration += samplesPer16th * currentSwing * 0.5;
        }
        else
        {
            stepDuration -= samplesPer16th * currentSwing * 0.5;
        }

        if (sampleCounter >= stepDuration)
        {
            sampleCounter = 0.0;
            internalStep = (internalStep + 1) % NUM_STEPS;
        }
    }
}

} // namespace BreakStepAudio
