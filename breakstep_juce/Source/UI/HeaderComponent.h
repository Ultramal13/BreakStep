#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "RotaryKnob.h"
#include "../Audio/AudioEngine.h"
#include <memory>

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
    std::function<void(int bankIndex)> onBankChanged;
    std::function<void(int viewIndex)> onViewChanged;
    std::function<void()> onProjectLoaded;
    std::function<void()> onStateChanged;

    void updateAllControlsFromEngine();

private:
    void openSaveFileChooser();
    void openLoadFileChooser();
    void updateViewButtons();

    BreakStepAudio::AudioEngine& audioEngine;

    juce::TextButton playButton { "PLAY" };
    juce::TextButton viewChopperButton { "MPC CHOPPER" };
    juce::TextButton viewDrumsButton { "DRUM TRACKS" };
    juce::TextButton viewAllButton { "ALL TRACKS" };

    juce::TextButton clearAllButton { "CLR ALL" };
    juce::TextButton randomizeAllButton { "RND ALL" };

    juce::TextButton bankAButton { "A 1-16" };
    juce::TextButton bankBButton { "B 17-32" };
    juce::TextButton saveProjectButton { "SAVE PROJ" };
    juce::TextButton loadProjectButton { "LOAD PROJ" };

    std::unique_ptr<RotaryKnob> bpmKnob;
    std::unique_ptr<RotaryKnob> swingKnob;
    std::unique_ptr<RotaryKnob> delayKnob;
    std::unique_ptr<RotaryKnob> masterCutoffKnob;

    int currentBank = 0;
    int currentView = 2; // All tracks
    std::unique_ptr<juce::FileChooser> fileChooser;
};

} // namespace BreakStepUI
