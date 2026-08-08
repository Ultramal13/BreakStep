#include "RotaryKnob.h"
#include "CustomLookAndFeel.h"

namespace BreakStepUI
{

RotaryKnob::RotaryKnob(const juce::String& labelText, double minVal, double maxVal, double stepVal,
                       double defaultVal, std::function<juce::String(double)> valueFmt)
    : defaultValue(defaultVal), formatter(valueFmt)
{
    label.setText(labelText, juce::dontSendNotification);
    label.setFont(juce::FontOptions(8.0f).withStyle("Bold"));
    label.setColour(juce::Label::textColourId, CustomLookAndFeel::colorTextDim);
    label.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(label);

    slider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    slider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    slider.setRange(minVal, maxVal, stepVal);
    slider.setValue(defaultVal, juce::dontSendNotification);
    slider.setDoubleClickReturnValue(true, defaultVal);
    addAndMakeVisible(slider);

    valueDisplay.setText(formatter ? formatter(defaultVal) : juce::String(defaultVal), juce::dontSendNotification);
    valueDisplay.setFont(juce::FontOptions(9.0f));
    valueDisplay.setColour(juce::Label::textColourId, CustomLookAndFeel::colorLed);
    valueDisplay.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(valueDisplay);

    slider.onValueChange = [this]()
    {
        double val = slider.getValue();
        if (formatter)
            valueDisplay.setText(formatter(val), juce::dontSendNotification);
        else
            valueDisplay.setText(juce::String(val, 1), juce::dontSendNotification);

        if (onValueChange)
            onValueChange(val);
    };
}

void RotaryKnob::resized()
{
    auto bounds = getLocalBounds();
    label.setBounds(bounds.removeFromTop(12));
    valueDisplay.setBounds(bounds.removeFromBottom(12));
    slider.setBounds(bounds);
}

void RotaryKnob::setValue(double newValue, juce::NotificationType notification)
{
    slider.setValue(newValue, notification);
}

double RotaryKnob::getValue() const
{
    return slider.getValue();
}

} // namespace BreakStepUI
