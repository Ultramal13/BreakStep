#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <functional>

namespace BreakStepUI
{

class RotaryKnob : public juce::Component
{
public:
    RotaryKnob(const juce::String& labelText, double minVal, double maxVal, double stepVal,
               double defaultVal, std::function<juce::String(double)> valueFmt);

    ~RotaryKnob() override = default;

    void resized() override;

    void setValue(double newValue, juce::NotificationType notification = juce::sendNotification);
    double getValue() const;

    juce::Slider& getSlider() { return slider; }

    std::function<void(double)> onValueChange;

private:
    juce::Label label;
    juce::Slider slider;
    juce::Label valueDisplay;

    double defaultValue = 0.0;
    std::function<juce::String(double)> formatter;
};

} // namespace BreakStepUI
