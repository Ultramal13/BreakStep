#pragma once

#include <juce_gui_extra/juce_gui_extra.h>
#include "Audio/AudioEngine.h"
#include "UI/CustomLookAndFeel.h"
#include "UI/HeaderComponent.h"
#include "UI/TrackRowComponent.h"
#include "UI/ChopTrackRowComponent.h"
#include "UI/ChopperWaveformComponent.h"
#include "UI/SongTimelineComponent.h"
#include <vector>
#include <memory>

class MainComponent : public juce::AudioAppComponent,
                      private juce::Timer
{
public:
    MainComponent();
    ~MainComponent() override;

    void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override;
    void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) override;
    void releaseResources() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

    void setView(int viewIndex);

private:
    void timerCallback() override;

    BreakStepAudio::AudioEngine audioEngine;
    BreakStepUI::CustomLookAndFeel customLookAndFeel;

    std::unique_ptr<BreakStepUI::HeaderComponent> headerComponent;
    std::unique_ptr<BreakStepUI::ChopperWaveformComponent> chopperComponent;
    std::unique_ptr<BreakStepUI::ChopTrackRowComponent> chopTrackRow;
    std::vector<std::unique_ptr<BreakStepUI::TrackRowComponent>> trackRows;
    std::unique_ptr<BreakStepUI::SongTimelineComponent> timelineComponent;

    juce::Viewport viewport;
    std::unique_ptr<juce::Component> contentContainer;

    int lastPlayheadStep = -1;
    int lastSliceStep = -1;
    int lastSongBar = -1;
    int currentView = 3; // All (Chopper + Drums + Timeline)

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};
