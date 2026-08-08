#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "RotaryKnob.h"
#include "../Audio/DrumTrack.h"
#include "../Audio/StepSequencer.h"
#include "../Audio/AudioEngine.h"
#include <memory>
#include <vector>

namespace BreakStepUI
{

class TrackRowComponent : public juce::Component,
                          public juce::FileDragAndDropTarget
{
public:
    TrackRowComponent(int trackIndex, BreakStepAudio::AudioEngine& engine);
    ~TrackRowComponent() override = default;

    void paint(juce::Graphics& g) override;
    void resized() override;

    void setViewBank(int bank);
    void updatePlayhead(int step);
    void updateAllControlsFromState();

    // Drag and Drop target
    bool isInterestedInFileDrag(const juce::StringArray& files) override;
    void filesDropped(const juce::StringArray& files, int x, int y) override;

private:
    void openFileChooser();
    void updateStepButtonStyles();
    void updateVintageButtonText();

    int trackIndex;
    BreakStepAudio::AudioEngine& audioEngine;

    juce::Label trackNameLabel;
    juce::Label sampleNameLabel;

    juce::TextButton muteButton { "M" };
    juce::TextButton soloButton { "S" };
    juce::TextButton loadButton { "LOAD" };
    juce::TextButton clearButton { "CLR" };
    juce::TextButton randomizeButton { "RND" };
    juce::TextButton vintageModeButton { "CLEAN" };

    std::unique_ptr<RotaryKnob> volKnob;
    std::unique_ptr<RotaryKnob> pitchKnob;
    std::unique_ptr<RotaryKnob> atkKnob;
    std::unique_ptr<RotaryKnob> fltKnob;
    std::unique_ptr<RotaryKnob> lenKnob;
    std::unique_ptr<RotaryKnob> revKnob;
    std::unique_ptr<RotaryKnob> crunchKnob;

    std::vector<std::unique_ptr<juce::TextButton>> stepButtons;

    int currentBank = 0; // 0 = steps 0..15, 1 = steps 16..31
    int activePlayheadStep = -1;

    std::unique_ptr<juce::FileChooser> fileChooser;
};

} // namespace BreakStepUI
