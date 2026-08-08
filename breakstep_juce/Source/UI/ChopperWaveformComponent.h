#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "../Audio/AudioEngine.h"
#include "RotaryKnob.h"
#include <memory>
#include <vector>

namespace BreakStepUI
{

class ChopperWaveformComponent : public juce::Component,
                                 public juce::FileDragAndDropTarget
{
public:
    ChopperWaveformComponent(BreakStepAudio::AudioEngine& engine);
    ~ChopperWaveformComponent() override = default;

    void paint(juce::Graphics& g) override;
    void resized() override;

    // Mouse interactions for ReCycle-style slicing
    void mouseDown(const juce::MouseEvent& e) override;
    void mouseDrag(const juce::MouseEvent& e) override;
    void mouseUp(const juce::MouseEvent& e) override;
    void mouseDoubleClick(const juce::MouseEvent& e) override;

    bool isInterestedInFileDrag(const juce::StringArray& files) override;
    void filesDropped(const juce::StringArray& files, int x, int y) override;

    void updateAllControls();

private:
    void openLoopChooser();
    void updatePadLabels();
    void updateSlotButtonStyles();
    int getSliceIndexAtX(float x) const;
    int getSliceMarkerNearX(float x, float tolerancePx = 6.0f) const;

    BreakStepAudio::AudioEngine& audioEngine;

    juce::TextButton slotButtons[4];
    juce::TextButton loadLoopButton { "LOAD AUDIO" };
    juce::TextButton resetTransientsButton { "AUTO SLICE" };
    juce::Label fileNameLabel;
    juce::Label selectedSliceLabel;

    std::unique_ptr<RotaryKnob> nudgeKnob;
    std::unique_ptr<RotaryKnob> sensitivityKnob;
    std::unique_ptr<RotaryKnob> volumeKnob;
    std::unique_ptr<RotaryKnob> cutoffKnob;
    std::unique_ptr<RotaryKnob> reverbKnob;

    std::vector<std::unique_ptr<juce::TextButton>> slicePads;
    std::unique_ptr<juce::FileChooser> fileChooser;

    // Interaction state
    int draggedSliceMarker = -1;
    int activeAuditionSlice = 0;
    juce::Rectangle<int> waveArea;
};

} // namespace BreakStepUI
