#include "SliceSequencer.h"

namespace BreakStepAudio
{

SliceSequencer::SliceSequencer()
{
    randomize();
}

SliceStep SliceSequencer::getStep(int index) const
{
    if (index >= 0 && index < NUM_STEPS)
        return steps[index];
    return {};
}

void SliceSequencer::setStep(int index, const SliceStep& step)
{
    if (index >= 0 && index < NUM_STEPS)
        steps[index] = step;
}

void SliceSequencer::toggleStepActive(int index)
{
    if (index >= 0 && index < NUM_STEPS)
        steps[index].active = !steps[index].active;
}

void SliceSequencer::setStepSlice(int index, int sliceIndex)
{
    if (index >= 0 && index < NUM_STEPS)
        steps[index].sliceIndex = juce::jlimit(0, 15, sliceIndex);
}

void SliceSequencer::cycleRatchets(int index)
{
    if (index >= 0 && index < NUM_STEPS)
    {
        int r = steps[index].ratchets;
        if (r == 1) steps[index].ratchets = 2;
        else if (r == 2) steps[index].ratchets = 3;
        else if (r == 3) steps[index].ratchets = 4;
        else steps[index].ratchets = 1;
    }
}

void SliceSequencer::cycleProbability(int index)
{
    if (index >= 0 && index < NUM_STEPS)
    {
        float p = steps[index].probability;
        if (p >= 0.99f) steps[index].probability = 0.75f;
        else if (p >= 0.74f) steps[index].probability = 0.50f;
        else if (p >= 0.49f) steps[index].probability = 0.25f;
        else steps[index].probability = 1.0f;
    }
}

void SliceSequencer::toggleReverse(int index)
{
    if (index >= 0 && index < NUM_STEPS)
        steps[index].reverse = !steps[index].reverse;
}

void SliceSequencer::clear()
{
    for (int i = 0; i < NUM_STEPS; ++i)
    {
        steps[i].active = false;
        steps[i].sliceIndex = i % 16;
        steps[i].ratchets = 1;
        steps[i].probability = 1.0f;
        steps[i].reverse = false;
        steps[i].pitchOffset = 0;
    }
}

void SliceSequencer::randomize()
{
    for (int i = 0; i < NUM_STEPS; ++i)
    {
        float roll = random.nextFloat();
        steps[i].active = (i % 2 == 0) || (i == 7) || (i == 11) || (i == 15) || (roll > 0.45f);
        steps[i].sliceIndex = (i + (roll > 0.7f ? 2 : 0)) % 16;
        steps[i].ratchets = (i == 14) ? 2 : ((i == 15) ? 4 : ((roll > 0.85f) ? 2 : 1));
        steps[i].probability = (roll > 0.75f) ? 0.75f : 1.0f;
        steps[i].reverse = (i == 6 && roll > 0.5f);
        steps[i].pitchOffset = 0;
    }
}

void SliceSequencer::processBlock(int numSamples, double sampleRate, std::vector<SliceTrigger>& outTriggers)
{
    outTriggers.clear();

    if (!playing.load() || sampleRate <= 0.0)
        return;

    const double currentBpm = bpm.load();
    const double samplesPer16th = (sampleRate * 60.0) / (currentBpm * 4.0);

    for (int i = 0; i < numSamples; ++i)
    {
        if (sampleCounter == 0.0)
        {
            currentStep.store(internalStep);
            const auto& step = steps[internalStep];

            if (step.active)
            {
                // Check probability
                float roll = random.nextFloat();
                if (roll <= step.probability)
                {
                    int ratchets = std::max(1, step.ratchets);
                    double subSpacing = samplesPer16th / ratchets;

                    for (int r = 0; r < ratchets; ++r)
                    {
                        SliceTrigger trig;
                        trig.sliceIndex = step.sliceIndex;
                        trig.velocity = 1.0f;
                        trig.pitchOffset = static_cast<float>(step.pitchOffset);
                        trig.reverse = step.reverse;
                        trig.sampleOffset = i + static_cast<int>(r * subSpacing);

                        if (trig.sampleOffset < numSamples)
                        {
                            outTriggers.push_back(trig);
                        }
                    }
                }
            }
        }

        sampleCounter += 1.0;
        if (sampleCounter >= samplesPer16th)
        {
            sampleCounter = 0.0;
            internalStep = (internalStep + 1) % NUM_STEPS;
        }
    }
}

juce::var SliceSequencer::saveStateToVar() const
{
    juce::Array<juce::var> stepArray;
    for (int i = 0; i < NUM_STEPS; ++i)
    {
        auto* obj = new juce::DynamicObject();
        obj->setProperty("active", steps[i].active);
        obj->setProperty("sliceIndex", steps[i].sliceIndex);
        obj->setProperty("ratchets", steps[i].ratchets);
        obj->setProperty("probability", steps[i].probability);
        obj->setProperty("reverse", steps[i].reverse);
        obj->setProperty("pitchOffset", steps[i].pitchOffset);
        stepArray.add(juce::var(obj));
    }
    return juce::var(stepArray);
}

void SliceSequencer::loadStateFromVar(const juce::var& stateVar)
{
    if (stateVar.isArray())
    {
        for (int i = 0; i < stateVar.size() && i < NUM_STEPS; ++i)
        {
            auto item = stateVar[i];
            if (item.isObject())
            {
                steps[i].active = static_cast<bool>(item.getProperty("active", false));
                steps[i].sliceIndex = static_cast<int>(item.getProperty("sliceIndex", i));
                steps[i].ratchets = static_cast<int>(item.getProperty("ratchets", 1));
                steps[i].probability = static_cast<float>(item.getProperty("probability", 1.0));
                steps[i].reverse = static_cast<bool>(item.getProperty("reverse", false));
                steps[i].pitchOffset = static_cast<int>(item.getProperty("pitchOffset", 0));
            }
        }
    }
}

} // namespace BreakStepAudio
