#include "StepSequencer.h"
#include <cmath>
#include <cstring>

namespace BreakStepAudio
{

StepSequencer::StepSequencer()
{
    std::memset(stepGrid, 0, sizeof(stepGrid));
    randomizeAllTracks();
}

int StepSequencer::getStepState(int trackIndex, int stepIndex) const
{
    if (trackIndex >= 0 && trackIndex < NUM_TRACKS && stepIndex >= 0 && stepIndex < NUM_STEPS)
    {
        return stepGrid[trackIndex][stepIndex];
    }
    return 0;
}

void StepSequencer::setStepState(int trackIndex, int stepIndex, int state)
{
    if (trackIndex >= 0 && trackIndex < NUM_TRACKS && stepIndex >= 0 && stepIndex < NUM_STEPS)
    {
        stepGrid[trackIndex][stepIndex] = state;
    }
}

void StepSequencer::toggleStepState(int trackIndex, int stepIndex)
{
    if (trackIndex >= 0 && trackIndex < NUM_TRACKS && stepIndex >= 0 && stepIndex < NUM_STEPS)
    {
        // Cycles: 0 (Off) -> 1 (Normal) -> 2 (Accent) -> 0
        int currentState = stepGrid[trackIndex][stepIndex];
        stepGrid[trackIndex][stepIndex] = (currentState + 1) % 3;
    }
}

void StepSequencer::clearTrack(int trackIndex)
{
    if (trackIndex >= 0 && trackIndex < NUM_TRACKS)
    {
        for (int s = 0; s < NUM_STEPS; ++s)
        {
            stepGrid[trackIndex][s] = 0;
        }
    }
}

void StepSequencer::clearAllTracks()
{
    for (int t = 0; t < NUM_TRACKS; ++t)
    {
        clearTrack(t);
    }
}

void StepSequencer::randomizeTrack(int trackIndex)
{
    if (trackIndex < 0 || trackIndex >= NUM_TRACKS)
        return;

    clearTrack(trackIndex);

    for (int s = 0; s < NUM_STEPS; ++s)
    {
        int beat16 = s % 16;
        float roll = random.nextFloat();

        switch (trackIndex)
        {
            case 0: // Kick: Strong downbeats and DnB syncopation
                if (beat16 == 0 || beat16 == 10)
                    stepGrid[trackIndex][s] = 2; // Accent
                else if ((beat16 == 6 || beat16 == 11 || beat16 == 15) && roll > 0.45f)
                    stepGrid[trackIndex][s] = 1;
                break;

            case 1: // Snare: Strong backbeat on 4 and 12, ghost notes on 9/15
                if (beat16 == 4 || beat16 == 12)
                    stepGrid[trackIndex][s] = 2;
                else if ((beat16 == 9 || beat16 == 15 || beat16 == 7) && roll > 0.6f)
                    stepGrid[trackIndex][s] = 1;
                break;

            case 2: // Closed Hat: Running 16ths with velocity accents
                if (beat16 % 2 == 0)
                    stepGrid[trackIndex][s] = (roll > 0.7f) ? 2 : 1;
                else if (roll > 0.35f)
                    stepGrid[trackIndex][s] = 1;
                break;

            case 3: // Open Hat: Syncopated offbeats
                if ((beat16 == 2 || beat16 == 6 || beat16 == 10 || beat16 == 14) && roll > 0.5f)
                    stepGrid[trackIndex][s] = (roll > 0.75f) ? 2 : 1;
                break;

            case 4: // Clap: Layered on beat 12 or syncopated
                if (beat16 == 12 && roll > 0.3f)
                    stepGrid[trackIndex][s] = 2;
                else if (beat16 == 8 && roll > 0.7f)
                    stepGrid[trackIndex][s] = 1;
                break;

            case 5: // Perc: Rolling breakbeat accents
                if ((beat16 == 3 || beat16 == 7 || beat16 == 11 || beat16 == 13) && roll > 0.4f)
                    stepGrid[trackIndex][s] = (roll > 0.7f) ? 2 : 1;
                break;
        }
    }
}

void StepSequencer::randomizeAllTracks()
{
    for (int t = 0; t < NUM_TRACKS; ++t)
    {
        randomizeTrack(t);
    }
}

void StepSequencer::processBlock(int numSamples, double sampleRate, std::vector<StepTrigger>& outTriggers)
{
    outTriggers.clear();

    if (!playing.load() || sampleRate <= 0.0)
        return;

    const double currentBpm = bpm.load();
    const double currentSwing = swing.load();

    // 16th note base duration in samples
    const double baseSamplesPer16th = (sampleRate * 60.0) / (currentBpm * 4.0);

    for (int i = 0; i < numSamples; ++i)
    {
        // Calculate swing offset for odd 16th steps
        double currentStepDuration = baseSamplesPer16th;
        if (internalStep % 2 == 1)
        {
            currentStepDuration += baseSamplesPer16th * (currentSwing * 0.4);
        }
        else
        {
            currentStepDuration -= baseSamplesPer16th * (currentSwing * 0.4);
        }

        if (sampleCounter == 0.0)
        {
            currentStep.store(internalStep);

            for (int t = 0; t < NUM_TRACKS; ++t)
            {
                int state = stepGrid[t][internalStep];
                if (state > 0)
                {
                    StepTrigger trig;
                    trig.trackIndex = t;
                    trig.velocityState = state;
                    trig.sampleOffset = i;
                    outTriggers.push_back(trig);
                }
            }
        }

        sampleCounter += 1.0;
        if (sampleCounter >= currentStepDuration)
        {
            sampleCounter = 0.0;
            internalStep = (internalStep + 1) % NUM_STEPS;
        }
    }
}

} // namespace BreakStepAudio
