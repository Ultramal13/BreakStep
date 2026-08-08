#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "../Audio/AudioEngine.h"
#include <memory>
#include <vector>

namespace BreakStepUI
{

class SongTimelineComponent : public juce::Component
{
public:
    SongTimelineComponent(BreakStepAudio::AudioEngine& engine);
    ~SongTimelineComponent() override = default;

    void paint(juce::Graphics& g) override;
    void resized() override;

    void mouseDown(const juce::MouseEvent& e) override;
    void updatePlayhead();

private:
    BreakStepAudio::AudioEngine& audioEngine;

    juce::Label titleLabel;
    juce::TextButton clearButton { "CLEAR ARR" };
    juce::TextButton stdArrButton { "STD DNB ARR" };
    juce::TextButton repeat4Button { "REPEAT 1-4" };

    juce::Rectangle<int> rulerArea;
    juce::Rectangle<int> drumLaneArea;
    juce::Rectangle<int> slicerLaneArea;

    int lastPlayheadBar = -1;
};

} // namespace BreakStepUI
