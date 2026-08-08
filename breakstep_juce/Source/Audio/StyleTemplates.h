#pragma once

#include <juce_core/juce_core.h>

namespace BreakStepAudio
{

class AudioEngine;

enum class StylePreset
{
    DnB_Roller = 0,
    Jungle_Amen,
    Dubstep_140,
    UK_Garage_2Step,
    UK_Bass_138,
    Breakbeat_135,
    Liquid_DnB,
    Count
};

class StyleTemplates
{
public:
    static juce::String getStyleName(StylePreset style);
    static double getStyleBpm(StylePreset style);
    static double getStyleSwing(StylePreset style);

    static void applyStyle(StylePreset style, AudioEngine& engine);

private:
    static void generateDnBRoller(AudioEngine& engine);
    static void generateJungleAmen(AudioEngine& engine);
    static void generateDubstep140(AudioEngine& engine);
    static void generateUKGarage(AudioEngine& engine);
    static void generateUKBass(AudioEngine& engine);
    static void generateBreakbeat(AudioEngine& engine);
    static void generateLiquidDnB(AudioEngine& engine);
};

} // namespace BreakStepAudio
