#include "MainComponent.h"

MainComponent::MainComponent()
{
    juce::LookAndFeel::setDefaultLookAndFeel(&customLookAndFeel);

    headerComponent = std::make_unique<BreakStepUI::HeaderComponent>(audioEngine);
    headerComponent->onBankChanged = [this](int bank) {
        for (auto& row : trackRows)
        {
            row->setViewBank(bank);
        }
    };
    headerComponent->onPatternChanged = [this](int /*patternIndex*/) {
        for (auto& row : trackRows)
        {
            row->updateAllControlsFromState();
        }
        if (chopTrackRow) chopTrackRow->updateAllControls();
    };
    headerComponent->onViewChanged = [this](int viewIndex) {
        setView(viewIndex);
    };
    headerComponent->onProjectLoaded = [this]() {
        for (auto& row : trackRows)
        {
            row->updateAllControlsFromState();
        }
        if (chopperComponent) chopperComponent->updateAllControls();
        if (chopTrackRow) chopTrackRow->updateAllControls();
        if (timelineComponent) timelineComponent->repaint();
    };
    headerComponent->onStateChanged = [this]() {
        for (auto& row : trackRows)
        {
            row->updateAllControlsFromState();
        }
        if (chopTrackRow) chopTrackRow->updateAllControls();
        if (chopperComponent) chopperComponent->updateAllControls();
        if (timelineComponent) timelineComponent->repaint();
    };
    addAndMakeVisible(*headerComponent);

    // Create scrollable content container
    contentContainer = std::make_unique<juce::Component>();

    // 1. Top Section: MPC Sample Chopper & ReCycle Waveform Lab
    chopperComponent = std::make_unique<BreakStepUI::ChopperWaveformComponent>(audioEngine);
    contentContainer->addAndMakeVisible(*chopperComponent);

    // 2. Track 0: Breakbeat Chop Track
    chopTrackRow = std::make_unique<BreakStepUI::ChopTrackRowComponent>(audioEngine);
    contentContainer->addAndMakeVisible(*chopTrackRow);

    // 3. Tracks 1..6: BreakStep Drum Machine Tracks
    for (int t = 0; t < BreakStepAudio::StepSequencer::NUM_TRACKS; ++t)
    {
        auto row = std::make_unique<BreakStepUI::TrackRowComponent>(t, audioEngine);
        contentContainer->addAndMakeVisible(*row);
        trackRows.push_back(std::move(row));
    }

    // 4. Reason-Style Linear Song Timeline Arranger
    timelineComponent = std::make_unique<BreakStepUI::SongTimelineComponent>(audioEngine);
    contentContainer->addAndMakeVisible(*timelineComponent);

    viewport.setViewedComponent(contentContainer.get(), false);
    viewport.setScrollBarsShown(true, false);
    viewport.setScrollBarThickness(10);
    addAndMakeVisible(viewport);

    setSize(1040, 720);
    setAudioChannels(0, 2);

    // Set default view to ALL (Chopper + Drums + Timeline)
    setView(3);

    // 60Hz UI refresh timer for playheads
    startTimerHz(60);
}

MainComponent::~MainComponent()
{
    stopTimer();
    shutdownAudio();
    juce::LookAndFeel::setDefaultLookAndFeel(nullptr);
}

void MainComponent::setView(int viewIndex)
{
    currentView = viewIndex;

    if (currentView == 0) // Chopper + Chop Track
    {
        chopperComponent->setVisible(true);
        chopTrackRow->setVisible(true);
        for (auto& row : trackRows) row->setVisible(false);
        timelineComponent->setVisible(false);
    }
    else if (currentView == 1) // Drum Machine Only
    {
        chopperComponent->setVisible(false);
        chopTrackRow->setVisible(false);
        for (auto& row : trackRows) row->setVisible(true);
        timelineComponent->setVisible(false);
    }
    else if (currentView == 2) // Timeline Only
    {
        chopperComponent->setVisible(false);
        chopTrackRow->setVisible(false);
        for (auto& row : trackRows) row->setVisible(false);
        timelineComponent->setVisible(true);
    }
    else // All Tracks View (Full Workstation with scroll)
    {
        chopperComponent->setVisible(true);
        chopTrackRow->setVisible(true);
        for (auto& row : trackRows) row->setVisible(true);
        timelineComponent->setVisible(true);
    }

    resized();
    repaint();
}

void MainComponent::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
    audioEngine.prepareToPlay(sampleRate, samplesPerBlockExpected);
}

void MainComponent::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill)
{
    audioEngine.getNextAudioBlock(bufferToFill);
}

void MainComponent::releaseResources()
{
    audioEngine.releaseResources();
}

void MainComponent::paint(juce::Graphics& g)
{
    g.fillAll(BreakStepUI::CustomLookAndFeel::colorBg);
}

void MainComponent::resized()
{
    int margin = 10;
    int headerH = 58;

    headerComponent->setBounds(margin, margin, getWidth() - margin * 2, headerH);

    int viewY = headerH + margin + 6;
    int viewW = getWidth() - margin * 2;
    int viewH = getHeight() - viewY - margin;

    viewport.setBounds(margin, viewY, viewW, viewH);

    int contentW = viewW - (viewport.isVerticalScrollBarShown() ? 14 : 0);
    int y = 4;

    if (currentView == 0) // Chopper + Chop Track
    {
        int chopperH = 212;
        chopperComponent->setBounds(0, y, contentW, chopperH);
        y += chopperH + 8;

        int chopRowH = 56;
        chopTrackRow->setBounds(0, y, contentW, chopRowH);
        y += chopRowH + 8;

        contentContainer->setBounds(0, 0, contentW, y);
    }
    else if (currentView == 1) // Drum Machine Only
    {
        int rowHeight = 54;
        int rowGap = 4;

        for (auto& row : trackRows)
        {
            row->setBounds(0, y, contentW, rowHeight);
            y += rowHeight + rowGap;
        }

        contentContainer->setBounds(0, 0, contentW, y);
    }
    else if (currentView == 2) // Timeline Only
    {
        int tlH = 114;
        timelineComponent->setBounds(0, y, contentW, tlH);
        y += tlH + 8;

        contentContainer->setBounds(0, 0, contentW, y);
    }
    else // All Tracks View (Full Workstation with scroll)
    {
        int chopperH = 212;
        chopperComponent->setBounds(0, y, contentW, chopperH);
        y += chopperH + 8;

        int chopRowH = 56;
        chopTrackRow->setBounds(0, y, contentW, chopRowH);
        y += chopRowH + 6;

        int rowHeight = 54;
        int rowGap = 4;

        for (auto& row : trackRows)
        {
            row->setBounds(0, y, contentW, rowHeight);
            y += rowHeight + rowGap;
        }
        y += 4;

        int tlH = 114;
        timelineComponent->setBounds(0, y, contentW, tlH);
        y += tlH + 10;

        contentContainer->setBounds(0, 0, contentW, y + 10);
    }
}

void MainComponent::timerCallback()
{
    // Update BreakStep Drum Playhead
    int currentDrumStep = audioEngine.getSequencer().getCurrentStep();
    if (currentDrumStep != lastPlayheadStep)
    {
        lastPlayheadStep = currentDrumStep;
        for (auto& row : trackRows)
        {
            row->updatePlayhead(currentDrumStep);
        }
    }

    // Update Slice Sequencer Playhead
    int currentSlice = audioEngine.getSliceSequencer().getCurrentStep();
    if (currentSlice != lastSliceStep)
    {
        lastSliceStep = currentSlice;
        if (chopTrackRow)
        {
            chopTrackRow->updatePlayhead(currentSlice);
        }
    }

    // Update Song Timeline Playhead in Song Mode
    if (timelineComponent && timelineComponent->isVisible())
    {
        if (audioEngine.getPlaybackMode() == BreakStepAudio::PlaybackMode::SongTimeline &&
            audioEngine.getSequencer().isPlaying())
        {
            timelineComponent->updatePlayhead();

            // Refresh track rows if the active pattern changed on a new bar
            int currentBar = audioEngine.getSongTimeline().getCurrentBar();
            if (currentBar != lastSongBar)
            {
                lastSongBar = currentBar;
                for (auto& row : trackRows)
                {
                    row->updateAllControlsFromState();
                }
                if (chopTrackRow) chopTrackRow->updateAllControls();
            }
        }
    }
}
