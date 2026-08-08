#include "CustomLookAndFeel.h"

namespace BreakStepUI
{

const juce::Colour CustomLookAndFeel::colorBg      = juce::Colour(0x0d, 0x0e, 0x0f);
const juce::Colour CustomLookAndFeel::colorPanel   = juce::Colour(0x1a, 0x1b, 0x1d);
const juce::Colour CustomLookAndFeel::colorPanel2  = juce::Colour(0x13, 0x14, 0x16);
const juce::Colour CustomLookAndFeel::colorLine    = juce::Colour(0x2a, 0x2c, 0x2f);
const juce::Colour CustomLookAndFeel::colorLed     = juce::Colour(0xff, 0x9d, 0x2e);
const juce::Colour CustomLookAndFeel::colorLedDim  = juce::Colour(0x5a, 0x3d, 0x15);
const juce::Colour CustomLookAndFeel::colorCyan    = juce::Colour(0x3e, 0xe0, 0xc8);
const juce::Colour CustomLookAndFeel::colorGreen   = juce::Colour(0x4a, 0xde, 0x80);
const juce::Colour CustomLookAndFeel::colorRed     = juce::Colour(0xff, 0x4d, 0x4d);
const juce::Colour CustomLookAndFeel::colorText    = juce::Colour(0xc8, 0xc9, 0xcb);
const juce::Colour CustomLookAndFeel::colorTextDim = juce::Colour(0x6b, 0x6d, 0x70);

CustomLookAndFeel::CustomLookAndFeel()
{
    setColour(juce::ResizableWindow::backgroundColourId, colorBg);
    setColour(juce::TextButton::buttonColourId, colorPanel2);
    setColour(juce::TextButton::buttonOnColourId, colorLed);
    setColour(juce::TextButton::textColourOffId, colorText);
    setColour(juce::TextButton::textColourOnId, colorBg);
}

void CustomLookAndFeel::drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                                         float sliderPosProportional, float rotaryStartAngle,
                                         float rotaryEndAngle, juce::Slider& /*slider*/)
{
    auto radius = (float) juce::jmin(width, height) / 2.0f - 4.0f;
    auto centreX = (float) x + (float) width * 0.5f;
    auto centreY = (float) y + (float) height * 0.5f;
    auto rx = centreX - radius;
    auto ry = centreY - radius;
    auto rw = radius * 2.0f;

    // Body
    g.setColour(colorPanel2);
    g.fillEllipse(rx, ry, rw, rw);

    // Border
    g.setColour(colorLine);
    g.drawEllipse(rx, ry, rw, rw, 1.5f);

    // Value Arc
    float angle = rotaryStartAngle + sliderPosProportional * (rotaryEndAngle - rotaryStartAngle);
    juce::Path arcPath;
    arcPath.addCentredArc(centreX, centreY, radius - 1.0f, radius - 1.0f, 0.0f, rotaryStartAngle, angle, true);
    g.setColour(colorCyan.withAlpha(0.6f));
    g.strokePath(arcPath, juce::PathStrokeType(2.0f));

    // Pointer Needle
    juce::Path p;
    auto pointerLength = radius * 0.7f;
    auto pointerThickness = 2.0f;
    p.addRectangle(-pointerThickness * 0.5f, -radius + 2.0f, pointerThickness, pointerLength);
    p.applyTransform(juce::AffineTransform::rotation(angle).translated(centreX, centreY));

    g.setColour(colorLed);
    g.fillPath(p);
}

void CustomLookAndFeel::drawButtonBackground(juce::Graphics& g, juce::Button& button,
                                             const juce::Colour& backgroundColour,
                                             bool shouldDrawButtonAsHighlighted,
                                             bool shouldDrawButtonAsDown)
{
    auto bounds = button.getLocalBounds().toFloat().reduced(0.5f);
    auto baseColour = backgroundColour;

    if (shouldDrawButtonAsDown)
        baseColour = baseColour.darker(0.2f);
    else if (shouldDrawButtonAsHighlighted)
        baseColour = baseColour.brighter(0.1f);

    g.setColour(baseColour);
    g.fillRoundedRectangle(bounds, 4.0f);

    g.setColour(button.getToggleState() ? colorLed : colorLine);
    g.drawRoundedRectangle(bounds, 4.0f, 1.0f);
}

juce::Font CustomLookAndFeel::getTextButtonFont(juce::TextButton& /*button*/, int buttonHeight)
{
    return juce::FontOptions(juce::jmin(11.0f, (float) buttonHeight * 0.5f)).withStyle("Bold");
}

} // namespace BreakStepUI
