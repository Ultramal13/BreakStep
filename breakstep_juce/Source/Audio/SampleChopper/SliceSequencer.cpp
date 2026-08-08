#include "SliceSequencer.h"

namespace BreakStepAudio
{

SliceSequencer::SliceSequencer()
{
    for (int p = 0; p < NUM_PATTERNS; ++p)
    {
        randomizePattern(p);
    }
}

void SliceSequencer::setActivePattern(int patternIndex)
{
    activePatternIndex.store(juce::jlimit(0, NUM_PATTERNS - 1, patternIndex));
}

SliceStep SliceSequencer::getStep(int index) const
{
    int ptn = activePatternIndex.load();
    return getPatternStep(ptn, index);
}

void SliceSequencer::setStep(int index, const SliceStep& step)
{
    int ptn = activePatternIndex.load();
    setPatternStep(ptn, index, step);
}

SliceStep SliceSequencer::getPatternStep(int ptnIndex, int stepIndex) const
{
    if (ptnIndex >= 0 && ptnIndex < NUM_PATTERNS &&
        stepIndex >= 0 && stepIndex < NUM_STEPS)
    {
        return patterns[ptnIndex].steps[stepIndex];
    }
    return {};
}

void SliceSequencer::setPatternStep(int ptnIndex, int stepIndex, const SliceStep& step)
{
    if (ptnIndex >= 0 && ptnIndex < NUM_PATTERNS &&
        stepIndex >= 0 && stepIndex < NUM_STEPS)
    {
        patterns[ptnIndex].steps[stepIndex] = step;
    }
}

void SliceSequencer::toggleStepActive(int index)
{
    int ptn = activePatternIndex.load();
    if (index >= 0 && index < NUM_STEPS)
        patterns[ptn].steps[index].active = !patterns[ptn].steps[index].active;
}

void SliceSequencer::setStepSlice(int index, int sliceIndex)
{
    int ptn = activePatternIndex.load();
    if (index >= 0 && index < NUM_STEPS)
        patterns[ptn].steps[index].sliceIndex = juce::jlimit(0, 15, sliceIndex);
}

void SliceSequencer::cycleRatchets(int index)
{
    int ptn = activePatternIndex.load();
    if (index >= 0 && index < NUM_STEPS)
    {
        int r = patterns[ptn].steps[index].ratchets;
        if (r == 1) patterns[ptn].steps[index].ratchets = 2;
        else if (r == 2) patterns[ptn].steps[index].ratchets = 3;
        else if (r == 3) patterns[ptn].steps[index].ratchets = 4;
        else patterns[ptn].steps[index].ratchets = 1;
    }
}

void SliceSequencer::cycleProbability(int index)
{
    int ptn = activePatternIndex.load();
    if (index >= 0 && index < NUM_STEPS)
    {
        float p = patterns[ptn].steps[index].probability;
        if (p >= 0.99f) patterns[ptn].steps[index].probability = 0.75f;
        else if (p >= 0.74f) patterns[ptn].steps[index].probability = 0.50f;
        else if (p >= 0.49f) patterns[ptn].steps[index].probability = 0.25f;
        else patterns[ptn].steps[index].probability = 1.0f;
    }
}

void SliceSequencer::toggleReverse(int index)
{
    int ptn = activePatternIndex.load();
    if (index >= 0 && index < NUM_STEPS)
        patterns[ptn].steps[index].reverse = !patterns[ptn].steps[index].reverse;
}

void SliceSequencer::clear()
{
    int ptn = activePatternIndex.load();
    clearPattern(ptn);
}

void SliceSequencer::clearPattern(int ptnIndex)
{
    if (ptnIndex >= 0 && ptnIndex < NUM_PATTERNS)
    {
        for (int i = 0; i < NUM_STEPS; ++i)
        {
            patterns[ptnIndex].steps[i].active = false;
            patterns[ptnIndex].steps[i].sliceIndex = i % 16;
            patterns[ptnIndex].steps[i].ratchets = 1;
            patterns[ptnIndex].steps[i].probability = 1.0f;
            patterns[ptnIndex].steps[i].reverse = false;
            patterns[ptnIndex].steps[i].pitchOffset = 0;
        }
    }
}

void SliceSequencer::randomize()
{
    int ptn = activePatternIndex.load();
    randomizePattern(ptn);
}

void SliceSequencer::randomizePattern(int ptnIndex)
{
    if (ptnIndex >= 0 && ptnIndex < NUM_PATTERNS)
    {
        for (int i = 0; i < NUM_STEPS; ++i)
        {
            float roll = random.nextFloat();
            patterns[ptnIndex].steps[i].active = (i % 2 == 0) || (i == 7) || (i == 11) || (i == 15) || (roll > 0.45f);
            patterns[ptnIndex].steps[i].sliceIndex = (i + (roll > 0.7f ? (ptnIndex % 4) : 0)) % 16;
            patterns[ptnIndex].steps[i].ratchets = (i == 14) ? 2 : ((i == 15) ? 4 : ((roll > 0.85f) ? 2 : 1));
            patterns[ptnIndex].steps[i].probability = (roll > 0.75f) ? 0.75f : 1.0f;
            patterns[ptnIndex].steps[i].reverse = (i == 6 && roll > 0.5f);
            patterns[ptnIndex].steps[i].pitchOffset = 0;
        }
    }
}

void SliceSequencer::copyPattern(int srcPattern, int dstPattern)
{
    if (srcPattern >= 0 && srcPattern < NUM_PATTERNS &&
        dstPattern >= 0 && dstPattern < NUM_PATTERNS)
    {
        patterns[dstPattern] = patterns[srcPattern];
    }
}

void SliceSequencer::processBlock(int numSamples, double sampleRate, std::vector<SliceTrigger>& outTriggers)
{
    outTriggers.clear();

    if (!playing.load() || sampleRate <= 0.0)
        return;

    const double currentBpm = bpm.load();
    const double samplesPer16th = (sampleRate * 60.0) / (currentBpm * 4.0);
    const int ptn = activePatternIndex.load();

    for (int i = 0; i < numSamples; ++i)
    {
        if (sampleCounter == 0.0)
        {
            currentStep.store(internalStep);
            const auto& step = patterns[ptn].steps[internalStep];

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
    auto* rootObj = new juce::DynamicObject();
    rootObj->setProperty("activePattern", activePatternIndex.load());

    juce::Array<juce::var> patternArray;
    for (int p = 0; p < NUM_PATTERNS; ++p)
    {
        juce::Array<juce::var> stepArray;
        for (int i = 0; i < NUM_STEPS; ++i)
        {
            auto* obj = new juce::DynamicObject();
            obj->setProperty("active", patterns[p].steps[i].active);
            obj->setProperty("sliceIndex", patterns[p].steps[i].sliceIndex);
            obj->setProperty("ratchets", patterns[p].steps[i].ratchets);
            obj->setProperty("probability", patterns[p].steps[i].probability);
            obj->setProperty("reverse", patterns[p].steps[i].reverse);
            obj->setProperty("pitchOffset", patterns[p].steps[i].pitchOffset);
            stepArray.add(juce::var(obj));
        }
        patternArray.add(juce::var(stepArray));
    }
    rootObj->setProperty("patterns", patternArray);
    return juce::var(rootObj);
}

void SliceSequencer::loadStateFromVar(const juce::var& stateVar)
{
    if (stateVar.isObject())
    {
        activePatternIndex.store(static_cast<int>(stateVar.getProperty("activePattern", 0)));
        auto ptnArr = stateVar.getProperty("patterns", juce::var());
        if (ptnArr.isArray())
        {
            for (int p = 0; p < ptnArr.size() && p < NUM_PATTERNS; ++p)
            {
                auto stepArr = ptnArr[p];
                if (stepArr.isArray())
                {
                    for (int i = 0; i < stepArr.size() && i < NUM_STEPS; ++i)
                    {
                        auto item = stepArr[i];
                        if (item.isObject())
                        {
                            patterns[p].steps[i].active = static_cast<bool>(item.getProperty("active", false));
                            patterns[p].steps[i].sliceIndex = static_cast<int>(item.getProperty("sliceIndex", i));
                            patterns[p].steps[i].ratchets = static_cast<int>(item.getProperty("ratchets", 1));
                            patterns[p].steps[i].probability = static_cast<float>(item.getProperty("probability", 1.0));
                            patterns[p].steps[i].reverse = static_cast<bool>(item.getProperty("reverse", false));
                            patterns[p].steps[i].pitchOffset = static_cast<int>(item.getProperty("pitchOffset", 0));
                        }
                    }
                }
            }
        }
    }
}

} // namespace BreakStepAudio
