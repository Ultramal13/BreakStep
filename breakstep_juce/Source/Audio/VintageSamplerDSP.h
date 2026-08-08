#pragma once

#include <juce_core/juce_core.h>
#include <cmath>

namespace BreakStepAudio
{

enum class VintageMode
{
    Clean = 0,    // 32-bit float Hi-Fi
    MPC60,        // 12-Bit / 40kHz Punchy DAC + Analog Output Stage
    MPC3000,      // 16-Bit / 44.1kHz Warm Discrete Op-Amp Stage
    EPS16Plus     // Ensoniq OTIS Variable-Rate Crunch & Aliased Decimation
};

class VintageSamplerDSP
{
public:
    VintageSamplerDSP() = default;

    void prepare(double sampleRate)
    {
        currentSampleRate = sampleRate;
        holdCounter[0] = 0.0;
        holdCounter[1] = 0.0;
        lastSample[0] = 0.0f;
        lastSample[1] = 0.0f;
    }

    void reset()
    {
        holdCounter[0] = 0.0;
        holdCounter[1] = 0.0;
        lastSample[0] = 0.0f;
        lastSample[1] = 0.0f;
    }

    // Process a single stereo sample according to Vintage Mode and Crunch amount (0.0 .. 1.0)
    void process(float& left, float& right, VintageMode mode, float crunch)
    {
        if (mode == VintageMode::Clean && crunch < 0.001f)
            return;

        left = processChannel(left, 0, mode, crunch);
        right = processChannel(right, 1, mode, crunch);
    }

    static juce::String getModeName(VintageMode mode)
    {
        switch (mode)
        {
            case VintageMode::Clean:     return "CLEAN";
            case VintageMode::MPC60:     return "MPC-60";
            case VintageMode::MPC3000:   return "MPC-3K";
            case VintageMode::EPS16Plus: return "EPS-16";
            default:                     return "CLEAN";
        }
    }

    static VintageMode getNextMode(VintageMode mode)
    {
        int next = (static_cast<int>(mode) + 1) % 4;
        return static_cast<VintageMode>(next);
    }

private:
    float processChannel(float sample, int ch, VintageMode mode, float crunch)
    {
        float out = sample;

        switch (mode)
        {
            case VintageMode::Clean:
            {
                if (crunch > 0.001f)
                {
                    // Generic transparent bit reduction with crunch knob
                    float bits = juce::jlimit(4.0f, 16.0f, 16.0f - crunch * 12.0f);
                    out = quantizeBits(out, bits);
                }
                break;
            }

            case VintageMode::MPC60:
            {
                // 12-Bit Linear/Non-Linear Quantization (Roger Linn 12-bit DAC)
                float bits = 12.0f - crunch * 4.0f; // 12-bit -> 8-bit with crunch
                out = quantizeBits(out, bits);

                // MPC-60 Rate Decimation (Nominal ~40kHz)
                float targetRate = 40000.0f / (1.0f + crunch * 2.0f);
                out = decimateRate(out, ch, targetRate);

                // Analog Output Stage Saturation (Punchy Low-Mids)
                out = saturateWarm(out * 1.25f) * 0.85f;
                break;
            }

            case VintageMode::MPC3000:
            {
                // 16-Bit Discrete Converter with Warm Analog Preamp
                float bits = 16.0f - crunch * 4.0f;
                out = quantizeBits(out, bits);

                // Smooth analog saturation curve
                out = saturateTape(out * (1.0f + crunch * 0.8f));
                break;
            }

            case VintageMode::EPS16Plus:
            {
                // Ensoniq OTIS Chip Emulation (13-bit -> 8-bit variable-rate lo-fi grit)
                float bits = 13.0f - crunch * 6.0f;
                out = quantizeBits(out, bits);

                // Ensoniq Sample-and-Hold rate decimation (11.2kHz to 31.25kHz)
                float targetRate = 31250.0f / (1.0f + crunch * 2.5f);
                out = decimateRate(out, ch, targetRate);

                // Raw digital grit without soft smoothing
                out = juce::jlimit(-1.0f, 1.0f, out * (1.1f + crunch * 0.5f));
                break;
            }
        }

        return out;
    }

    float quantizeBits(float sample, float bits)
    {
        float levels = std::pow(2.0f, bits - 1.0f);
        return std::round(sample * levels) / levels;
    }

    float decimateRate(float sample, int ch, float targetSampleRate)
    {
        if (targetSampleRate >= currentSampleRate)
            return sample;

        float ratio = static_cast<float>(currentSampleRate / targetSampleRate);
        holdCounter[ch] += 1.0;

        if (holdCounter[ch] >= ratio)
        {
            holdCounter[ch] -= ratio;
            lastSample[ch] = sample;
        }

        return lastSample[ch];
    }

    float saturateWarm(float x)
    {
        // Smooth soft-clipping tanh approximation: x / (1 + |x|) or tanh
        return std::tanh(x);
    }

    float saturateTape(float x)
    {
        // Asymmetric warm harmonic distortion
        if (x > 1.0f) return 1.0f;
        if (x < -1.0f) return -1.0f;
        return 1.5f * x - 0.5f * x * x * x;
    }

    double currentSampleRate = 44100.0;
    double holdCounter[2] = { 0.0, 0.0 };
    float lastSample[2] = { 0.0f, 0.0f };
};

} // namespace BreakStepAudio
