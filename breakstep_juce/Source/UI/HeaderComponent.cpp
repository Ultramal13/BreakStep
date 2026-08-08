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

    // 3 View Switcher Tabs
    viewChopperButton.onClick = [this]() {
        currentView = 0;
        updateViewButtons();
        if (onViewChanged) onViewChanged(0);
    };
    addAndMakeVisible(viewChopperButton);

    viewDrumsButton.onClick = [this]() {
        currentView = 1;
        updateViewButtons();
        if (onViewChanged) onViewChanged(1);
    };
    addAndMakeVisible(viewDrumsButton);

    viewAllButton.onClick = [this]() {
        currentView = 2;
        updateViewButtons();
        if (onViewChanged) onViewChanged(2);
    };
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

    // BPM Knob
    bpmKnob = std::make_unique<RotaryKnob>("TEMPO", 60.0, 200.0, 1.0, 170.0, [](double v) {
        return juce::String(juce::roundToInt(v));
    });
    bpmKnob->onValueChange = [this](double val) {
        audioEngine.getSequencer().setBpm(val);
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

void HeaderComponent::updateViewButtons()
{
    viewChopperButton.setColour(juce::TextButton::buttonColourId, (currentView == 0) ? CustomLookAndFeel::colorLed : CustomLookAndFeel::colorPanel2);
    viewChopperButton.setColour(juce::TextButton::textColourOffId, (currentView == 0) ? CustomLookAndFeel::colorBg : CustomLookAndFeel::colorText);

    viewDrumsButton.setColour(juce::TextButton::buttonColourId, (currentView == 1) ? CustomLookAndFeel::colorLed : CustomLookAndFeel::colorPanel2);
    viewDrumsButton.setColour(juce::TextButton::textColourOffId, (currentView == 1) ? CustomLookAndFeel::colorBg : CustomLookAndFeel::colorText);

    viewAllButton.setColour(juce::TextButton::buttonColourId, (currentView == 2) ? CustomLookAndFeel::colorLed : CustomLookAndFeel::colorPanel2);
    viewAllButton.setColour(juce::TextButton::textColourOffId, (currentView == 2) ? CustomLookAndFeel::colorBg : CustomLookAndFeel::colorText);
}

void HeaderComponent::paint(juce::Graphics& g)
{
    g.setColour(CustomLookAndFeel::colorPanel);
    g.fillRoundedRectangle(getLocalBounds().toFloat(), 6.0f);

    g.setColour(CustomLookAndFeel::colorLine);
    g.drawRoundedRectangle(getLocalBounds().toFloat(), 6.0f, 1.0f);

    // Title
    g.setFont(juce::FontOptions(19.0f).withStyle("Bold"));
    g.setColour(CustomLookAndFeel::colorText);
    g.drawText("BREAK", 12, 10, 64, 20, juce::Justification::left);

    g.setColour(CustomLookAndFeel::colorLed);
    g.drawText("STEP", 74, 10, 52, 20, juce::Justification::left);

    g.setFont(juce::FontOptions(8.0f));
    g.setColour(CustomLookAndFeel::colorTextDim);
    g.drawText("MODULAR WORKSTATION", 12, 32, 120, 14, juce::Justification::left);
}

void HeaderComponent::resized()
{
    int y = 10;
    playButton.setBounds(136, y, 58, 34);

    viewChopperButton.setBounds(200, y, 96, 34);
    viewDrumsButton.setBounds(300, y, 96, 34);
    viewAllButton.setBounds(400, y, 86, 34);

    clearAllButton.setBounds(494, y, 56, 34);
    randomizeAllButton.setBounds(554, y, 58, 34);

    bankAButton.setBounds(620, y, 44, 34);
    bankBButton.setBounds(668, y, 44, 34);

    saveProjectButton.setBounds(720, y, 62, 34);
    loadProjectButton.setBounds(786, y, 62, 34);

    int knobX = 858;
    int knobWidth = 42;
    int knobGap = 4;

    bpmKnob->setBounds(knobX, 4, knobWidth, 48);
    knobX += knobWidth + knobGap;

    swingKnob->setBounds(knobX, 4, knobWidth, 48);
    knobX += knobWidth + knobGap;

    delayKnob->setBounds(knobX, 4, knobWidth, 48);
    knobX += knobWidth + knobGap;

    masterCutoffKnob->setBounds(knobX, 4, knobWidth, 48);
}

void HeaderComponent::updateAllControlsFromEngine()
{
    bpmKnob->setValue(audioEngine.getSequencer().getBpm(), juce::dontSendNotification);
    swingKnob->setValue(audioEngine.getSequencer().getSwing(), juce::dontSendNotification);
    delayKnob->setValue(audioEngine.getDelayWet(), juce::dontSendNotification);
    masterCutoffKnob->setValue(audioEngine.getMasterCutoff(), juce::dontSendNotification);
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
