#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "../Audio/AudioEngine.h"
#include <memory>
#include <vector>

namespace BreakStepUI
{

class SliceSequencerComponent : public juce::Component
{
public:
    SliceSequencerComponent(BreakStepAudio::AudioEngine& engine);
    ~SliceSequencerComponent() override = default;

    void paint(juce::Graphics& g) override;
    void resized() override;

    void updatePlayhead(int step);
    void updateAllControls();

private:
    void updateStepButtons();

    BreakStepAudio::AudioEngine& audioEngine;

    struct StepUI
    {
        std::unique_ptr<juce::TextButton> mainPad;
        std::unique_ptr<juce::TextButton> ratchetBtn;
        std::unique_ptr<juce::TextButton> probBtn;
        std::unique_ptr<juce::TextButton> revBtn;
    };

    std::vector<StepUI> stepControls;
    int activePlayheadStep = -1;
};

} // namespace BreakStepUI
