#include "AudioSlicer.h"
#include <cmath>
#include <algorithm>

namespace BreakStepAudio
{

AudioSlicer::AudioSlicer()
{
    filter.setType(juce::dsp::StateVariableTPTFilterType::lowpass);
    for (int i = 0; i < NUM_SLOTS; ++i)
    {
        slotFileNames[i] = "Slot " + juce::String(i + 1) + ": (Empty)";
        slotFilePaths[i] = "";
    }
}

void AudioSlicer::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;

    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = static_cast<juce::uint32>(samplesPerBlock);
    spec.numChannels = 2;

    filter.prepare(spec);
    filter.reset();

    if (sampleSlots[0].getNumSamples() == 0)
    {
        generateDefaultAmenBreak();
    }
}

void AudioSlicer::generateDefaultAmenBreak()
{
    // Generate 2-bar Amen Breakbeat at 170 BPM into Slot 0
    double durationSec = (60.0 / 170.0) * 8.0;
    int numSamples = static_cast<int>(currentSampleRate * durationSec);
    sampleSlots[0].setSize(2, numSamples);
    sampleSlots[0].clear();

    float* left = sampleSlots[0].getWritePointer(0);
    float* right = sampleSlots[0].getWritePointer(1);

    juce::Random random;

    int samplesPer16th = numSamples / 16;
    for (int step = 0; step < 16; ++step)
    {
        int onset = step * samplesPer16th;
        int hitLength = std::min(samplesPer16th * 2, numSamples - onset);

        bool isKick = (step == 0 || step == 10 || step == 11);
        bool isSnare = (step == 4 || step == 12 || step == 14);

        for (int i = 0; i < hitLength; ++i)
        {
            double t = static_cast<double>(i) / currentSampleRate;
            float sample = 0.0f;

            if (isKick)
            {
                double freq = 45.0 + 110.0 * std::exp(-t * 40.0);
                double phase = 2.0 * juce::MathConstants<double>::pi * (45.0 * t + (110.0 / 40.0) * (1.0 - std::exp(-t * 40.0)));
                sample = std::sin(phase) * std::exp(-t * 14.0) * 0.9f;
            }
            else if (isSnare)
            {
                float noise = (random.nextFloat() * 2.0f - 1.0f) * std::exp(-t * 24.0);
                double toneFreq = 200.0 * std::exp(-t * 20.0);
                double phase = 2.0 * juce::MathConstants<double>::pi * toneFreq * t;
                sample = noise * 0.7f + std::sin(phase) * std::exp(-t * 30.0) * 0.5f;
            }
            else
            {
                float noise = (random.nextFloat() * 2.0f - 1.0f) * std::exp(-t * 60.0);
                sample = noise * 0.35f;
            }

            left[onset + i] += sample;
            right[onset + i] += sample;
        }
    }

    slotFileNames[0] = "Amen Break (170 BPM)";
    slotFilePaths[0] = "";
    activeSlotIndex = 0;
    detectTransients(0.5f);
}

bool AudioSlicer::loadAudioFileToSlot(int slotIndex, const juce::File& file, juce::AudioFormatManager& formatManager)
{
    if (slotIndex < 0 || slotIndex >= NUM_SLOTS)
        return false;

    std::unique_ptr<juce::AudioFormatReader> reader(formatManager.createReaderFor(file));
    if (reader != nullptr)
    {
        sampleSlots[slotIndex].setSize(2, static_cast<int>(reader->lengthInSamples));
        sampleSlots[slotIndex].clear();

        if (reader->numChannels == 1)
        {
            juce::AudioBuffer<float> monoBuffer(1, static_cast<int>(reader->lengthInSamples));
            reader->read(&monoBuffer, 0, static_cast<int>(reader->lengthInSamples), 0, true, true);
            sampleSlots[slotIndex].copyFrom(0, 0, monoBuffer, 0, 0, monoBuffer.getNumSamples());
            sampleSlots[slotIndex].copyFrom(1, 0, monoBuffer, 0, 0, monoBuffer.getNumSamples());
        }
        else
        {
            reader->read(&sampleSlots[slotIndex], 0, static_cast<int>(reader->lengthInSamples), 0, true, true);
        }

        slotFileNames[slotIndex] = file.getFileName();
        slotFilePaths[slotIndex] = file.getFullPathName();
        activeSlotIndex = slotIndex;
        detectTransients(currentSensitivity);
        return true;
    }
    return false;
}

void AudioSlicer::setActiveSlot(int slotIndex)
{
    if (slotIndex >= 0 && slotIndex < NUM_SLOTS)
    {
        activeSlotIndex = slotIndex;
        if (sampleSlots[activeSlotIndex].getNumSamples() > 0)
        {
            detectTransients(currentSensitivity);
        }
    }
}

void AudioSlicer::resetToDefaultLoop()
{
    generateDefaultAmenBreak();
}

int AudioSlicer::snapToZeroCrossing(int sampleIndex) const
{
    const auto& currentBuffer = sampleSlots[activeSlotIndex];
    const int totalSamples = currentBuffer.getNumSamples();
    if (totalSamples <= 0 || sampleIndex <= 0)
        return 0;
    if (sampleIndex >= totalSamples - 1)
        return totalSamples - 1;

    const float* left = currentBuffer.getReadPointer(0);
    const float* right = (currentBuffer.getNumChannels() > 1) ? currentBuffer.getReadPointer(1) : left;

    int searchStart = std::max(1, sampleIndex - 96);
    int searchEnd = std::min(totalSamples - 1, sampleIndex + 32);

    int bestSample = sampleIndex;
    float minAmp = 100.0f;

    for (int i = searchStart; i <= searchEnd; ++i)
    {
        float cur = 0.5f * (left[i] + right[i]);
        float prev = 0.5f * (left[i - 1] + right[i - 1]);

        if (cur * prev <= 0.0f)
        {
            float absAmp = std::abs(cur);
            if (absAmp < minAmp)
            {
                minAmp = absAmp;
                bestSample = i;
            }
        }
    }

    return bestSample;
}

void AudioSlicer::rebuildSlicesFromOnsets(std::vector<int>& onsets)
{
    std::sort(onsets.begin(), onsets.end());
    onsets.erase(std::unique(onsets.begin(), onsets.end()), onsets.end());

    if (onsets.empty() || onsets.front() != 0)
        onsets.insert(onsets.begin(), 0);

    const int totalSamples = sampleSlots[activeSlotIndex].getNumSamples();

    std::lock_guard<std::mutex> lock(sliceMutex);
    slices.clear();

    for (size_t i = 0; i < onsets.size(); ++i)
    {
        SliceMarker marker;
        marker.startSample = onsets[i];
        marker.endSample = (i + 1 < onsets.size()) ? onsets[i + 1] : totalSamples;

        if (marker.endSample - marker.startSample >= 64)
        {
            slices.push_back(marker);
        }
    }

    if (!slices.empty())
    {
        slices.back().endSample = totalSamples;
    }
}

void AudioSlicer::detectTransients(float sensitivity)
{
    currentSensitivity = juce::jlimit(0.01f, 0.99f, sensitivity);

    const auto& currentBuffer = sampleSlots[activeSlotIndex];
    const int totalSamples = currentBuffer.getNumSamples();
    if (totalSamples <= 256)
        return;

    const float* left = currentBuffer.getReadPointer(0);
    const float* right = (currentBuffer.getNumChannels() > 1) ? currentBuffer.getReadPointer(1) : left;

    // 1. ReCycle Adaptive High-Frequency Pre-Emphasis
    std::vector<float> preEmphasized(totalSamples, 0.0f);
    preEmphasized[0] = 0.5f * (left[0] + right[0]);
    for (int i = 1; i < totalSamples; ++i)
    {
        float current = 0.5f * (left[i] + right[i]);
        float prev = 0.5f * (left[i - 1] + right[i - 1]);
        preEmphasized[i] = current - 0.92f * prev;
    }

    // 2. Dual Envelope Followers
    float alphaFast = std::exp(-1.0f / (0.0012f * static_cast<float>(currentSampleRate)));
    float alphaSlow = std::exp(-1.0f / (0.0300f * static_cast<float>(currentSampleRate)));

    std::vector<float> onsetFunction(totalSamples, 0.0f);
    float fastEnv = 0.0f;
    float slowEnv = 0.0f;

    float baselineFactor = 1.0f + (1.0f - currentSensitivity) * 4.5f;
    float noiseFloor = 0.004f * (1.0f - currentSensitivity * 0.7f);

    for (int i = 0; i < totalSamples; ++i)
    {
        float absVal = std::abs(preEmphasized[i]);

        fastEnv = (absVal > fastEnv) ? (absVal + (fastEnv - absVal) * 0.2f) : (fastEnv * alphaFast);
        slowEnv = (absVal > slowEnv) ? (absVal + (slowEnv - absVal) * 0.8f) : (slowEnv * alphaSlow);

        float onset = fastEnv - (slowEnv * baselineFactor);
        onsetFunction[i] = (onset > noiseFloor) ? onset : 0.0f;
    }

    // 3. Peak Picking
    std::vector<int> rawOnsets;
    rawOnsets.push_back(0);

    int minSpacingSamples = static_cast<int>(currentSampleRate * (0.020 + (1.0 - currentSensitivity) * 0.050));
    int lastOnset = 0;
    int searchRadius = 64;

    for (int i = searchRadius; i < totalSamples - searchRadius; ++i)
    {
        if (onsetFunction[i] > 0.0f && (i - lastOnset) >= minSpacingSamples)
        {
            bool isPeak = true;
            for (int r = 1; r <= searchRadius; ++r)
            {
                if (onsetFunction[i] < onsetFunction[i - r] || onsetFunction[i] < onsetFunction[i + r])
                {
                    isPeak = false;
                    break;
                }
            }

            if (isPeak)
            {
                int zeroCross = snapToZeroCrossing(i);
                rawOnsets.push_back(zeroCross);
                lastOnset = i;
            }
        }
    }

    if (rawOnsets.size() < 2)
    {
        rawOnsets.clear();
        int step = totalSamples / 16;
        for (int k = 0; k < 16; ++k)
        {
            rawOnsets.push_back(snapToZeroCrossing(k * step));
        }
    }

    rebuildSlicesFromOnsets(rawOnsets);
}

void AudioSlicer::nudgeSliceMarker(int sliceIndex, int sampleDelta)
{
    if (sliceIndex <= 0 || sliceIndex >= getNumSlices())
        return;

    int currentStart = getSlice(sliceIndex).startSample;
    int newSample = currentStart + sampleDelta;
    moveSliceMarker(sliceIndex, newSample);
}

void AudioSlicer::addSliceMarker(int samplePosition)
{
    std::vector<int> onsets;
    {
        std::lock_guard<std::mutex> lock(sliceMutex);
        for (const auto& s : slices)
        {
            onsets.push_back(s.startSample);
        }
    }
    onsets.push_back(snapToZeroCrossing(samplePosition));
    rebuildSlicesFromOnsets(onsets);
}

void AudioSlicer::removeSliceMarker(int sliceIndex)
{
    std::vector<int> onsets;
    {
        std::lock_guard<std::mutex> lock(sliceMutex);
        for (size_t i = 0; i < slices.size(); ++i)
        {
            if (static_cast<int>(i) != sliceIndex || i == 0)
            {
                onsets.push_back(slices[i].startSample);
            }
        }
    }
    rebuildSlicesFromOnsets(onsets);
}

void AudioSlicer::moveSliceMarker(int sliceIndex, int newSamplePosition)
{
    std::vector<int> onsets;
    {
        std::lock_guard<std::mutex> lock(sliceMutex);
        for (size_t i = 0; i < slices.size(); ++i)
        {
            if (static_cast<int>(i) == sliceIndex && i != 0)
            {
                onsets.push_back(snapToZeroCrossing(newSamplePosition));
            }
            else
            {
                onsets.push_back(slices[i].startSample);
            }
        }
    }
    rebuildSlicesFromOnsets(onsets);
}

int AudioSlicer::getNumSlices() const
{
    std::lock_guard<std::mutex> lock(sliceMutex);
    return static_cast<int>(slices.size());
}

SliceMarker AudioSlicer::getSlice(int index) const
{
    std::lock_guard<std::mutex> lock(sliceMutex);
    if (index >= 0 && index < static_cast<int>(slices.size()))
        return slices[index];

    return { 0, sampleSlots[activeSlotIndex].getNumSamples() };
}

void AudioSlicer::triggerSlice(int sliceIndex, float velocity, float pitchSemitones, bool reverse)
{
    std::lock_guard<std::mutex> lock(sliceMutex);
    if (slices.empty())
        return;

    int idx = juce::jlimit(0, static_cast<int>(slices.size()) - 1, sliceIndex);
    const auto& slice = slices[idx];

    int voiceIdx = 0;
    for (int v = 0; v < MAX_POLYPHONY; ++v)
    {
        if (!voices[v].active)
        {
            voiceIdx = v;
            break;
        }
    }

    auto& voice = voices[voiceIdx];
    voice.active = true;
    voice.startSample = slice.startSample;
    voice.endSample = slice.endSample;
    voice.isReverse = reverse;
    voice.playbackRate = std::pow(2.0f, pitchSemitones / 12.0f);
    voice.velocityGain = velocity;

    if (reverse)
    {
        voice.currentPosition = static_cast<double>(slice.endSample - 1);
    }
    else
    {
        voice.currentPosition = static_cast<double>(slice.startSample);
    }
}

void AudioSlicer::processBlock(juce::AudioBuffer<float>& outputBuffer, juce::AudioBuffer<float>& reverbSendBuffer, int startSample, int numSamples)
{
    const auto& currentBuffer = sampleSlots[activeSlotIndex];
    if (currentBuffer.getNumSamples() == 0)
        return;

    const int totalSourceSamples = currentBuffer.getNumSamples();
    const float* const* srcData = currentBuffer.getArrayOfReadPointers();

    juce::AudioBuffer<float> tempBuffer(2, numSamples);
    tempBuffer.clear();
    float* const* dstData = tempBuffer.getArrayOfWritePointers();

    float masterVol = volume.load();

    for (int v = 0; v < MAX_POLYPHONY; ++v)
    {
        auto& voice = voices[v];
        if (!voice.active)
            continue;

        for (int i = 0; i < numSamples; ++i)
        {
            if (voice.isReverse)
            {
                if (voice.currentPosition <= voice.startSample || voice.currentPosition < 0)
                {
                    voice.active = false;
                    break;
                }
            }
            else
            {
                if (voice.currentPosition >= voice.endSample || voice.currentPosition >= totalSourceSamples)
                {
                    voice.active = false;
                    break;
                }
            }

            int posInt = static_cast<int>(voice.currentPosition);
            float frac = static_cast<float>(voice.currentPosition - posInt);
            int nextPosInt = voice.isReverse ? std::max(0, posInt - 1) : std::min(posInt + 1, totalSourceSamples - 1);

            for (int ch = 0; ch < 2; ++ch)
            {
                float s1 = srcData[ch][posInt];
                float s2 = srcData[ch][nextPosInt];
                float interpolated = s1 + frac * (s2 - s1);
                dstData[ch][i] += interpolated * voice.velocityGain * masterVol;
            }

            if (voice.isReverse)
                voice.currentPosition -= voice.playbackRate;
            else
                voice.currentPosition += voice.playbackRate;
        }
    }

    // Apply Filter
    filter.setCutoffFrequency(cutoff.load());
    juce::dsp::AudioBlock<float> block(tempBuffer);
    juce::dsp::ProcessContextReplacing<float> context(block);
    filter.process(context);

    // Mix into main output and reverb send
    float revSend = reverbSend.load();
    for (int ch = 0; ch < outputBuffer.getNumChannels(); ++ch)
    {
        outputBuffer.addFrom(ch, startSample, tempBuffer, ch, 0, numSamples);
        if (revSend > 0.001f && ch < reverbSendBuffer.getNumChannels())
        {
            reverbSendBuffer.addFrom(ch, startSample, tempBuffer, ch, 0, numSamples, revSend);
        }
    }
}

} // namespace BreakStepAudio
