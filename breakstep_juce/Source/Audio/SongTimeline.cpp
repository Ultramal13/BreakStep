#include "SongTimeline.h"
#include "StepSequencer.h"
#include "SampleChopper/SliceSequencer.h"

namespace BreakStepAudio
{

SongTimeline::SongTimeline()
{
    fillStandardArrangement();
}

void SongTimeline::setDrumBlock(int barIndex, int patternIndex)
{
    if (barIndex >= 0 && barIndex < NUM_BARS)
    {
        drumBlocks[barIndex] = juce::jlimit(0, 8, patternIndex);
    }
}

int SongTimeline::getDrumBlock(int barIndex) const
{
    if (barIndex >= 0 && barIndex < NUM_BARS)
    {
        return drumBlocks[barIndex];
    }
    return 0;
}

void SongTimeline::setSlicerBlock(int barIndex, int patternIndex)
{
    if (barIndex >= 0 && barIndex < NUM_BARS)
    {
        slicerBlocks[barIndex] = juce::jlimit(0, 8, patternIndex);
    }
}

int SongTimeline::getSlicerBlock(int barIndex) const
{
    if (barIndex >= 0 && barIndex < NUM_BARS)
    {
        return slicerBlocks[barIndex];
    }
    return 0;
}

void SongTimeline::clearTimeline()
{
    for (int b = 0; b < NUM_BARS; ++b)
    {
        drumBlocks[b] = 0;
        slicerBlocks[b] = 0;
    }
}

void SongTimeline::fillStandardArrangement()
{
    for (int b = 0; b < NUM_BARS; ++b)
    {
        if (b < 4) // Intro: Chops only
        {
            drumBlocks[b] = 0;
            slicerBlocks[b] = 1;
        }
        else if (b < 8) // Build: Chops + Light Drums
        {
            drumBlocks[b] = 1;
            slicerBlocks[b] = 1;
        }
        else if (b < 16) // Main Drop: Full Drums + Chop variations
        {
            drumBlocks[b] = (b % 4 == 3) ? 2 : 1;
            slicerBlocks[b] = (b % 2 == 0) ? 1 : 2;
        }
        else if (b < 20) // Breakdown: Atmospheric Chops
        {
            drumBlocks[b] = 0;
            slicerBlocks[b] = 3;
        }
        else if (b < 28) // Second Drop: Heavy Drums + Ratchet Chops
        {
            drumBlocks[b] = (b % 4 == 3) ? 3 : 2;
            slicerBlocks[b] = (b % 2 == 0) ? 2 : 4;
        }
        else // Outro
        {
            drumBlocks[b] = 1;
            slicerBlocks[b] = 1;
        }
    }
}

void SongTimeline::processBlock(int numSamples, double sampleRate, double bpm, StepSequencer& drums, SliceSequencer& slicer)
{
    if (sampleRate <= 0.0 || bpm <= 0.0)
        return;

    const double samplesPerBeat = (sampleRate * 60.0) / bpm;
    const double samplesPerBar = samplesPerBeat * 4.0; // 4/4 meter

    for (int i = 0; i < numSamples; ++i)
    {
        if (sampleCounter == 0.0)
        {
            currentBar.store(internalBar);

            int dBlock = drumBlocks[internalBar];
            if (dBlock > 0)
            {
                drums.setActivePattern(dBlock - 1);
            }

            int sBlock = slicerBlocks[internalBar];
            if (sBlock > 0)
            {
                slicer.setActivePattern(sBlock - 1);
            }
        }

        sampleCounter += 1.0;
        if (sampleCounter >= samplesPerBar)
        {
            sampleCounter = 0.0;
            internalBar = (internalBar + 1) % NUM_BARS;
        }
    }

    barProgress.store(static_cast<float>(sampleCounter / samplesPerBar));
}

juce::var SongTimeline::saveStateToVar() const
{
    auto* rootObj = new juce::DynamicObject();

    juce::Array<juce::var> drumArray;
    juce::Array<juce::var> slicerArray;

    for (int b = 0; b < NUM_BARS; ++b)
    {
        drumArray.add(drumBlocks[b]);
        slicerArray.add(slicerBlocks[b]);
    }

    rootObj->setProperty("drumBlocks", drumArray);
    rootObj->setProperty("slicerBlocks", slicerArray);

    return juce::var(rootObj);
}

void SongTimeline::loadStateFromVar(const juce::var& stateVar)
{
    if (stateVar.isObject())
    {
        auto dArr = stateVar.getProperty("drumBlocks", juce::var());
        if (dArr.isArray())
        {
            for (int b = 0; b < dArr.size() && b < NUM_BARS; ++b)
                drumBlocks[b] = static_cast<int>(dArr[b]);
        }

        auto sArr = stateVar.getProperty("slicerBlocks", juce::var());
        if (sArr.isArray())
        {
            for (int b = 0; b < sArr.size() && b < NUM_BARS; ++b)
                slicerBlocks[b] = static_cast<int>(sArr[b]);
        }
    }
}

} // namespace BreakStepAudio
