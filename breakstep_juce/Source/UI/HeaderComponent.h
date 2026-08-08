#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "RotaryKnob.h"
#include "../Audio/AudioEngine.h"
#include "../Audio/StyleTemplates.h"
#include <memory>
#include <vector>

namespace BreakStepUI
{

class HeaderComponent : public juce::Component
{
public:
    HeaderComponent(BreakStepAudio::AudioEngine& engine);
    ~HeaderComponent() override = default;

    void paint(juce::Graphics& g) override;
    void resized() override;

    int getCurrentBank() const { return currentBank; }
    int getCurrentPattern() const { return currentPattern; }

    std::function<void(int bankIndex)> onBankChanged;
    std::function<void(int patternIndex)> onPatternChanged;
    std::function<void(int viewIndex)> onViewChanged;
    std::function<void()> onProjectLoaded;
    std::function<void()> onStateChanged;

    void updateAllControlsFromEngine();

private:
    void openSaveFileChooser();
    void openLoadFileChooser();
    void updateViewButtons();
    void updateModeButtons();
    void updatePatternButtons();

    BreakStepAudio::AudioEngine& audioEngine;

    juce::TextButton playButton { "PLAY" };

    // Playback Mode: Pattern vs Song Timeline
    juce::TextButton patternModeButton { "PATTERN" };
    juce::TextButton songModeButton { "SONG" };

    // Style Presets ComboBox
    juce::ComboBox styleComboBox;

    // Pattern Bank Selectors (P1 .. P8)
    juce::TextButton patternButtons[8];

    // View Switcher Tabs
    juce::TextButton viewChopperButton { "CHOPPER" };
    juce::TextButton viewDrumsButton { "DRUMS" };
    juce::TextButton viewTimelineButton { "TIMELINE" };
    juce::TextButton viewAllButton { "ALL" };

    // Global Clear and Randomize
    juce::TextButton clearAllButton { "CLR ALL" };
    juce::TextButton randomizeAllButton { "RND ALL" };

    // Step Banks A (1-16) and B (17-32)
    juce::TextButton bankAButton { "A 1-16" };
    juce::TextButton bankBButton { "B 17-32" };

    // Save & Load
    juce::TextButton saveProjectButton { "SAVE" };
    juce::TextButton loadProjectButton { "LOAD" };

    // Knobs
    std::unique_ptr<RotaryKnob> drumBusKnob;
    std::unique_ptr<RotaryKnob> chopBusKnob;
    std::unique_ptr<RotaryKnob> bpmKnob;
    std::unique_ptr<RotaryKnob> swingKnob;
    std::unique_ptr<RotaryKnob> delayKnob;
    std::unique_ptr<RotaryKnob> masterCutoffKnob;

    int currentBank = 0;
    int currentPattern = 0;
    int currentView = 3; // All tracks + Timeline

    std::unique_ptr<juce::FileChooser> fileChooser;
};

} // namespace BreakStepUI
