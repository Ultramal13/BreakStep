#pragma once

#include <juce_gui_extra/juce_gui_extra.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include "Audio/AudioEngine.h"
#include "UI/CustomLookAndFeel.h"
#include "UI/HeaderComponent.h"
#include "UI/TrackRowComponent.h"
#include "UI/ChopTrackRowComponent.h"
#include "UI/ChopperWaveformComponent.h"
#include <memory>
#include <vector>

class MainComponent : public juce::AudioAppComponent,
                      public juce::Timer
{
public:
    MainComponent();
    ~MainComponent() override;

    void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override;
    void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) override;
    void releaseResources() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

    void timerCallback() override;

private:
    void setView(int viewIndex);

    BreakStepUI::CustomLookAndFeel customLookAndFeel;
    BreakStepAudio::AudioEngine audioEngine;

    std::unique_ptr<BreakStepUI::HeaderComponent> headerComponent;

    // Scrollable Viewport and content container
    juce::Viewport viewport;
    std::unique_ptr<juce::Component> contentContainer;

    // Workstation Modules
    std::unique_ptr<BreakStepUI::ChopperWaveformComponent> chopperComponent;
    std::unique_ptr<BreakStepUI::ChopTrackRowComponent> chopTrackRow;
    std::vector<std::unique_ptr<BreakStepUI::TrackRowComponent>> trackRows;

    int currentView = 2; // 0 = Chopper, 1 = Drums, 2 = All
    int lastPlayheadStep = -1;
    int lastSliceStep = -1;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};
