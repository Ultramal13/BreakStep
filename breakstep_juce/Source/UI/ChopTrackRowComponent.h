#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "../Audio/AudioEngine.h"
#include "RotaryKnob.h"
#include <memory>
#include <vector>

namespace BreakStepUI
{

class ChopTrackRowComponent : public juce::Component
{
public:
    ChopTrackRowComponent(BreakStepAudio::AudioEngine& engine);
    ~ChopTrackRowComponent() override = default;

    void paint(juce::Graphics& g) override;
    void resized() override;

    void updatePlayhead(int step);
    void updateAllControls();

private:
    void updateStepButtonStyles();

    BreakStepAudio::AudioEngine& audioEngine;

    juce::Label trackNameLabel;
    juce::Label sampleNameLabel;

    juce::TextButton clearButton { "CLR" };
    juce::TextButton randomizeButton { "RND" };

    std::unique_ptr<RotaryKnob> volKnob;
    std::unique_ptr<RotaryKnob> fltKnob;
    std::unique_ptr<RotaryKnob> revKnob;

    struct StepButtonUI
    {
        std::unique_ptr<juce::TextButton> stepPad;
        std::unique_ptr<juce::TextButton> ratchetBtn;
        std::unique_ptr<juce::TextButton> probBtn;
    };

    std::vector<StepButtonUI> stepButtons;
    int activePlayheadStep = -1;
};

} // namespace BreakStepUI
