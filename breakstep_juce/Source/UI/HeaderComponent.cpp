#include "HeaderComponent.h"
#include "CustomLookAndFeel.h"

namespace BreakStepUI
{

HeaderComponent::HeaderComponent(BreakStepAudio::AudioEngine& engine)
    : audioEngine(engine)
{
    playButton.setColour(juce::TextButton::buttonColourId, CustomLookAndFeel::colorPanel2);
    playButton.setColour(juce::TextButton::textColourOffId, CustomLookAndFeel::colorCyan);
    playButton.onClick = [this]()
    {
        auto& seq = audioEngine.getSequencer();
        bool newPlayState = !seq.isPlaying();
        seq.setPlaying(newPlayState);

        if (newPlayState)
        {
            if (audioEngine.getPlaybackMode() == BreakStepAudio::PlaybackMode::SongTimeline)
            {
                audioEngine.getSongTimeline().resetPosition();
            }
            playButton.setButtonText("STOP");
            playButton.setColour(juce::TextButton::buttonColourId, CustomLookAndFeel::colorCyan);
            playButton.setColour(juce::TextButton::textColourOffId, CustomLookAndFeel::colorBg);
        }
        else
        {
            playButton.setButtonText("PLAY");
            playButton.setColour(juce::TextButton::buttonColourId, CustomLookAndFeel::colorPanel2);
            playButton.setColour(juce::TextButton::textColourOffId, CustomLookAndFeel::colorCyan);
        }
    };
    addAndMakeVisible(playButton);

    // Mode Buttons (PATTERN vs SONG TIMELINE)
    patternModeButton.onClick = [this]() {
        audioEngine.setPlaybackMode(BreakStepAudio::PlaybackMode::PatternLoop);
        updateModeButtons();
    };
    addAndMakeVisible(patternModeButton);

    songModeButton.onClick = [this]() {
        audioEngine.setPlaybackMode(BreakStepAudio::PlaybackMode::SongTimeline);
        updateModeButtons();
    };
    addAndMakeVisible(songModeButton);
    updateModeButtons();

    // Style Presets ComboBox
    styleComboBox.setTextWhenNothingSelected("STYLE PRESETS");
    styleComboBox.addItem("DnB Roller (174 BPM)", 1);
    styleComboBox.addItem("Jungle Amen (168 BPM)", 2);
    styleComboBox.addItem("Dubstep 140 (140 BPM)", 3);
    styleComboBox.addItem("UK Garage 2-Step (132 BPM)", 4);
    styleComboBox.addItem("UK Bass / Bassline (138 BPM)", 5);
    styleComboBox.addItem("Nu-Skool Breaks (135 BPM)", 6);
    styleComboBox.addItem("Liquid DnB (172 BPM)", 7);

    styleComboBox.onChange = [this]() {
        int id = styleComboBox.getSelectedId();
        if (id >= 1 && id <= 7)
        {
            auto preset = static_cast<BreakStepAudio::StylePreset>(id - 1);
            BreakStepAudio::StyleTemplates::applyStyle(preset, audioEngine);
            updateAllControlsFromEngine();
            if (onStateChanged) onStateChanged();
        }
    };
    addAndMakeVisible(styleComboBox);

    // 8 Pattern Bank Buttons (P1 .. P8)
    for (int p = 0; p < 8; ++p)
    {
        patternButtons[p].setButtonText("P" + juce::String(p + 1));
        int ptnIdx = p;
        patternButtons[p].onClick = [this, ptnIdx]() {
            currentPattern = ptnIdx;
            audioEngine.getSequencer().setActivePattern(ptnIdx);
            audioEngine.getSliceSequencer().setActivePattern(ptnIdx);
            updatePatternButtons();
            if (onPatternChanged) onPatternChanged(ptnIdx);
            if (onStateChanged) onStateChanged();
        };
        addAndMakeVisible(patternButtons[p]);
    }
    updatePatternButtons();

    // View Switcher Tabs
    viewChopperButton.onClick = [this]() { currentView = 0; updateViewButtons(); if (onViewChanged) onViewChanged(0); };
    addAndMakeVisible(viewChopperButton);

    viewDrumsButton.onClick = [this]() { currentView = 1; updateViewButtons(); if (onViewChanged) onViewChanged(1); };
    addAndMakeVisible(viewDrumsButton);

    viewTimelineButton.onClick = [this]() { currentView = 2; updateViewButtons(); if (onViewChanged) onViewChanged(2); };
    addAndMakeVisible(viewTimelineButton);

    viewAllButton.onClick = [this]() { currentView = 3; updateViewButtons(); if (onViewChanged) onViewChanged(3); };
    addAndMakeVisible(viewAllButton);
    updateViewButtons();

    // Global Clear and Randomize Buttons
    clearAllButton.setColour(juce::TextButton::buttonColourId, CustomLookAndFeel::colorPanel2);
    clearAllButton.setColour(juce::TextButton::textColourOffId, CustomLookAndFeel::colorRed);
    clearAllButton.onClick = [this]() {
        audioEngine.getSequencer().clearAllTracks();
        audioEngine.getSliceSequencer().clear();
        if (onStateChanged) onStateChanged();
    };
    addAndMakeVisible(clearAllButton);

    randomizeAllButton.setColour(juce::TextButton::buttonColourId, CustomLookAndFeel::colorPanel2);
    randomizeAllButton.setColour(juce::TextButton::textColourOffId, CustomLookAndFeel::colorGreen);
    randomizeAllButton.onClick = [this]() {
        audioEngine.getSequencer().randomizeAllTracks();
        audioEngine.getSliceSequencer().randomize();
        if (onStateChanged) onStateChanged();
    };
    addAndMakeVisible(randomizeAllButton);

    // Bank A/B
    bankAButton.setToggleState(true, juce::dontSendNotification);
    bankAButton.onClick = [this]()
    {
        currentBank = 0;
        bankAButton.setToggleState(true, juce::dontSendNotification);
        bankBButton.setToggleState(false, juce::dontSendNotification);
        if (onBankChanged) onBankChanged(0);
    };
    addAndMakeVisible(bankAButton);

    bankBButton.onClick = [this]()
    {
        currentBank = 1;
        bankBButton.setToggleState(true, juce::dontSendNotification);
        bankAButton.setToggleState(false, juce::dontSendNotification);
        if (onBankChanged) onBankChanged(1);
    };
    addAndMakeVisible(bankBButton);

    // Save & Load Project Buttons
    saveProjectButton.setColour(juce::TextButton::buttonColourId, CustomLookAndFeel::colorPanel2);
    saveProjectButton.setColour(juce::TextButton::textColourOffId, CustomLookAndFeel::colorLed);
    saveProjectButton.onClick = [this]() { openSaveFileChooser(); };
    addAndMakeVisible(saveProjectButton);

    loadProjectButton.setColour(juce::TextButton::buttonColourId, CustomLookAndFeel::colorPanel2);
    loadProjectButton.setColour(juce::TextButton::textColourOffId, CustomLookAndFeel::colorLed);
    loadProjectButton.onClick = [this]() { openLoadFileChooser(); };
    addAndMakeVisible(loadProjectButton);

    // Submix Bus Volume Knobs
    drumBusKnob = std::make_unique<RotaryKnob>("DRUMS", 0.0, 1.0, 0.01, 0.85, [](double v) {
        return juce::String(juce::roundToInt(v * 100.0));
    });
    drumBusKnob->onValueChange = [this](double val) {
        audioEngine.setDrumBusVolume(static_cast<float>(val));
    };
    addAndMakeVisible(*drumBusKnob);

    chopBusKnob = std::make_unique<RotaryKnob>("CHOP", 0.0, 1.0, 0.01, 0.85, [](double v) {
        return juce::String(juce::roundToInt(v * 100.0));
    });
    chopBusKnob->onValueChange = [this](double val) {
        audioEngine.setSamplerBusVolume(static_cast<float>(val));
    };
    addAndMakeVisible(*chopBusKnob);

    // BPM Knob
    bpmKnob = std::make_unique<RotaryKnob>("TEMPO", 60.0, 200.0, 1.0, 170.0, [](double v) {
        return juce::String(juce::roundToInt(v));
    });
    bpmKnob->onValueChange = [this](double val) {
        audioEngine.getSequencer().setBpm(val);
        audioEngine.getSliceSequencer().setBpm(val);
    };
    addAndMakeVisible(*bpmKnob);

    // Swing Knob
    swingKnob = std::make_unique<RotaryKnob>("SWING", 0.0, 0.7, 0.01, 0.0, [](double v) {
        return juce::String(juce::roundToInt(v * 100.0)) + "%";
    });
    swingKnob->onValueChange = [this](double val) {
        audioEngine.getSequencer().setSwing(val);
    };
    addAndMakeVisible(*swingKnob);

    // Delay Knob
    delayKnob = std::make_unique<RotaryKnob>("DELAY", 0.0, 0.6, 0.01, 0.0, [](double v) {
        return juce::String(juce::roundToInt(v * 100.0));
    });
    delayKnob->onValueChange = [this](double val) {
        audioEngine.setDelayWet(static_cast<float>(val));
    };
    addAndMakeVisible(*delayKnob);

    // Master Cutoff Knob
    masterCutoffKnob = std::make_unique<RotaryKnob>("MASTER", 200.0, 20000.0, 100.0, 20000.0, [](double v) {
        return (v >= 1000.0) ? juce::String(juce::roundToInt(v / 1000.0)) + "k" : juce::String(juce::roundToInt(v));
    });
    masterCutoffKnob->onValueChange = [this](double val) {
        audioEngine.setMasterCutoff(static_cast<float>(val));
    };
    addAndMakeVisible(*masterCutoffKnob);
}

void HeaderComponent::updateModeButtons()
{
    bool isSong = (audioEngine.getPlaybackMode() == BreakStepAudio::PlaybackMode::SongTimeline);

    patternModeButton.setColour(juce::TextButton::buttonColourId, !isSong ? CustomLookAndFeel::colorLed : CustomLookAndFeel::colorPanel2);
    patternModeButton.setColour(juce::TextButton::textColourOffId, !isSong ? CustomLookAndFeel::colorBg : CustomLookAndFeel::colorTextDim);

    songModeButton.setColour(juce::TextButton::buttonColourId, isSong ? CustomLookAndFeel::colorGreen : CustomLookAndFeel::colorPanel2);
    songModeButton.setColour(juce::TextButton::textColourOffId, isSong ? CustomLookAndFeel::colorBg : CustomLookAndFeel::colorTextDim);
}

void HeaderComponent::updatePatternButtons()
{
    for (int p = 0; p < 8; ++p)
    {
        if (p == currentPattern)
        {
            patternButtons[p].setColour(juce::TextButton::buttonColourId, CustomLookAndFeel::colorCyan);
            patternButtons[p].setColour(juce::TextButton::textColourOffId, CustomLookAndFeel::colorBg);
        }
        else
        {
            patternButtons[p].setColour(juce::TextButton::buttonColourId, CustomLookAndFeel::colorPanel2);
            patternButtons[p].setColour(juce::TextButton::textColourOffId, CustomLookAndFeel::colorTextDim);
        }
    }
}

void HeaderComponent::updateViewButtons()
{
    viewChopperButton.setColour(juce::TextButton::buttonColourId, (currentView == 0) ? CustomLookAndFeel::colorLed : CustomLookAndFeel::colorPanel2);
    viewChopperButton.setColour(juce::TextButton::textColourOffId, (currentView == 0) ? CustomLookAndFeel::colorBg : CustomLookAndFeel::colorText);

    viewDrumsButton.setColour(juce::TextButton::buttonColourId, (currentView == 1) ? CustomLookAndFeel::colorLed : CustomLookAndFeel::colorPanel2);
    viewDrumsButton.setColour(juce::TextButton::textColourOffId, (currentView == 1) ? CustomLookAndFeel::colorBg : CustomLookAndFeel::colorText);

    viewTimelineButton.setColour(juce::TextButton::buttonColourId, (currentView == 2) ? CustomLookAndFeel::colorLed : CustomLookAndFeel::colorPanel2);
    viewTimelineButton.setColour(juce::TextButton::textColourOffId, (currentView == 2) ? CustomLookAndFeel::colorBg : CustomLookAndFeel::colorText);

    viewAllButton.setColour(juce::TextButton::buttonColourId, (currentView == 3) ? CustomLookAndFeel::colorLed : CustomLookAndFeel::colorPanel2);
    viewAllButton.setColour(juce::TextButton::textColourOffId, (currentView == 3) ? CustomLookAndFeel::colorBg : CustomLookAndFeel::colorText);
}

void HeaderComponent::paint(juce::Graphics& g)
{
    g.setColour(CustomLookAndFeel::colorPanel);
    g.fillRoundedRectangle(getLocalBounds().toFloat(), 6.0f);

    g.setColour(CustomLookAndFeel::colorLine);
    g.drawRoundedRectangle(getLocalBounds().toFloat(), 6.0f, 1.0f);

    // Title
    g.setFont(juce::FontOptions(17.0f).withStyle("Bold"));
    g.setColour(CustomLookAndFeel::colorText);
    g.drawText("BREAK", 10, 8, 54, 18, juce::Justification::left);

    g.setColour(CustomLookAndFeel::colorLed);
    g.drawText("STEP", 64, 8, 44, 18, juce::Justification::left);

    g.setFont(juce::FontOptions(7.5f));
    g.setColour(CustomLookAndFeel::colorTextDim);
    g.drawText("MODULAR WORKSTATION", 10, 26, 100, 12, juce::Justification::left);
}

void HeaderComponent::resized()
{
    int yTop = 6;
    int yBot = 28;

    // Transport & Mode
    playButton.setBounds(112, yTop, 52, 40);
    patternModeButton.setBounds(168, yTop, 54, 19);
    songModeButton.setBounds(168, yBot, 54, 18);

    // Presets Dropdown
    styleComboBox.setBounds(226, yTop, 136, 19);

    // 8 Pattern Bank Buttons (P1..P8)
    int ptnX = 226;
    int ptnW = 16;
    for (int p = 0; p < 8; ++p)
    {
        patternButtons[p].setBounds(ptnX, yBot, ptnW, 18);
        ptnX += ptnW + 1;
    }

    // Views
    int vX = 368;
    viewChopperButton.setBounds(vX, yTop, 58, 19);
    viewDrumsButton.setBounds(vX + 60, yTop, 52, 19);
    viewTimelineButton.setBounds(vX, yBot, 58, 18);
    viewAllButton.setBounds(vX + 60, yBot, 52, 18);

    // Clear / Randomize
    int clrX = 484;
    clearAllButton.setBounds(clrX, yTop, 44, 19);
    randomizeAllButton.setBounds(clrX + 46, yTop, 44, 19);

    // Banks A/B
    bankAButton.setBounds(clrX, yBot, 44, 18);
    bankBButton.setBounds(clrX + 46, yBot, 44, 18);

    // Save / Load
    int saveX = 578;
    saveProjectButton.setBounds(saveX, yTop, 42, 19);
    loadProjectButton.setBounds(saveX, yBot, 42, 18);

    // Knobs (Bus volumes & Master FX)
    int knobX = 628;
    int knobWidth = 38;
    int knobGap = 2;

    drumBusKnob->setBounds(knobX, 2, knobWidth, 48); knobX += knobWidth + knobGap;
    chopBusKnob->setBounds(knobX, 2, knobWidth, 48); knobX += knobWidth + knobGap;
    bpmKnob->setBounds(knobX, 2, knobWidth, 48); knobX += knobWidth + knobGap;
    swingKnob->setBounds(knobX, 2, knobWidth, 48); knobX += knobWidth + knobGap;
    delayKnob->setBounds(knobX, 2, knobWidth, 48); knobX += knobWidth + knobGap;
    masterCutoffKnob->setBounds(knobX, 2, knobWidth, 48);
}

void HeaderComponent::updateAllControlsFromEngine()
{
    drumBusKnob->setValue(audioEngine.getDrumBusVolume(), juce::dontSendNotification);
    chopBusKnob->setValue(audioEngine.getSamplerBusVolume(), juce::dontSendNotification);
    bpmKnob->setValue(audioEngine.getSequencer().getBpm(), juce::dontSendNotification);
    swingKnob->setValue(audioEngine.getSequencer().getSwing(), juce::dontSendNotification);
    delayKnob->setValue(audioEngine.getDelayWet(), juce::dontSendNotification);
    masterCutoffKnob->setValue(audioEngine.getMasterCutoff(), juce::dontSendNotification);
    updateModeButtons();
    updatePatternButtons();
}

void HeaderComponent::openSaveFileChooser()
{
    fileChooser = std::make_unique<juce::FileChooser>(
        "Save BreakStep Workstation Project",
        juce::File::getSpecialLocation(juce::File::userHomeDirectory).getChildFile("my_workstation.breakstep"),
        "*.breakstep"
    );

    auto flags = juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::canSelectFiles;

    fileChooser->launchAsync(flags, [this](const juce::FileChooser& fc) {
        auto file = fc.getResult();
        if (file != juce::File())
        {
            if (file.getFileExtension().isEmpty())
                file = file.withFileExtension(".breakstep");

            audioEngine.saveProjectToFile(file);
        }
    });
}

void HeaderComponent::openLoadFileChooser()
{
    fileChooser = std::make_unique<juce::FileChooser>(
        "Load BreakStep Workstation Project",
        juce::File::getSpecialLocation(juce::File::userHomeDirectory),
        "*.breakstep"
    );

    auto flags = juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles;

    fileChooser->launchAsync(flags, [this](const juce::FileChooser& fc) {
        auto file = fc.getResult();
        if (file.existsAsFile())
        {
            if (audioEngine.loadProjectFromFile(file))
            {
                updateAllControlsFromEngine();
                if (onProjectLoaded)
                    onProjectLoaded();
            }
        }
    });
}

} // namespace BreakStepUI
