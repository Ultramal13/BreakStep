#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace BreakStepUI
{

class CustomLookAndFeel : public juce::LookAndFeel_V4
{
public:
    CustomLookAndFeel();

    // Color definitions
    static const juce::Colour colorBg;
    static const juce::Colour colorPanel;
    static const juce::Colour colorPanel2;
    static const juce::Colour colorLine;
    static const juce::Colour colorLed;
    static const juce::Colour colorLedDim;
    static const juce::Colour colorCyan;
    static const juce::Colour colorGreen;
    static const juce::Colour colorRed;
    static const juce::Colour colorText;
    static const juce::Colour colorTextDim;

    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                          float sliderPosProportional, float rotaryStartAngle,
                          float rotaryEndAngle, juce::Slider& slider) override;

    void drawButtonBackground(juce::Graphics& g, juce::Button& button,
                              const juce::Colour& backgroundColour,
                              bool shouldDrawButtonAsHighlighted,
                              bool shouldDrawButtonAsDown) override;

    juce::Font getTextButtonFont(juce::TextButton& button, int buttonHeight) override;
};

} // namespace BreakStepUI
