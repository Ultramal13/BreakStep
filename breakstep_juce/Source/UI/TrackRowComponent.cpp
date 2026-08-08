#include "TrackRowComponent.h"
#include "CustomLookAndFeel.h"

namespace BreakStepUI
{

TrackRowComponent::TrackRowComponent(int idx, BreakStepAudio::AudioEngine& engine)
    : trackIndex(idx), audioEngine(engine)
{
    auto* track = audioEngine.getTrack(trackIndex);

    trackNameLabel.setText(track ? track->getDefaultTrackName() : "TRACK", juce::dontSendNotification);
    trackNameLabel.setFont(juce::FontOptions(11.0f).withStyle("Bold"));
    trackNameLabel.setColour(juce::Label::textColourId, CustomLookAndFeel::colorText);
    addAndMakeVisible(trackNameLabel);

    sampleNameLabel.setText(track ? track->getSampleName() : "synth", juce::dontSendNotification);
    sampleNameLabel.setFont(juce::FontOptions(8.0f));
    sampleNameLabel.setColour(juce::Label::textColourId, CustomLookAndFeel::colorCyan);
    addAndMakeVisible(sampleNameLabel);

    // Mute Button
    muteButton.setClickingTogglesState(true);
    muteButton.onClick = [this, track]() {
        if (track) track->setMuted(muteButton.getToggleState());
        updateStepButtonStyles();
    };
    muteButton.setColour(juce::TextButton::buttonColourId, CustomLookAndFeel::colorPanel2);
    muteButton.setColour(juce::TextButton::buttonOnColourId, CustomLookAndFeel::colorRed);
    muteButton.setColour(juce::TextButton::textColourOffId, CustomLookAndFeel::colorTextDim);
    muteButton.setColour(juce::TextButton::textColourOnId, CustomLookAndFeel::colorBg);
    addAndMakeVisible(muteButton);

    // Solo Button
    soloButton.setClickingTogglesState(true);
    soloButton.onClick = [this, track]() {
        if (track) track->setSoloed(soloButton.getToggleState());
        updateStepButtonStyles();
    };
    soloButton.setColour(juce::TextButton::buttonColourId, CustomLookAndFeel::colorPanel2);
    soloButton.setColour(juce::TextButton::buttonOnColourId, CustomLookAndFeel::colorGreen);
    soloButton.setColour(juce::TextButton::textColourOffId, CustomLookAndFeel::colorTextDim);
    soloButton.setColour(juce::TextButton::textColourOnId, CustomLookAndFeel::colorBg);
    addAndMakeVisible(soloButton);

    // Load Button
    loadButton.onClick = [this]() { openFileChooser(); };
    loadButton.setColour(juce::TextButton::buttonColourId, CustomLookAndFeel::colorPanel2);
    loadButton.setColour(juce::TextButton::textColourOffId, CustomLookAndFeel::colorTextDim);
    addAndMakeVisible(loadButton);

    // Clear Track Button
    clearButton.setColour(juce::TextButton::buttonColourId, CustomLookAndFeel::colorPanel2);
    clearButton.setColour(juce::TextButton::textColourOffId, CustomLookAndFeel::colorRed);
    clearButton.onClick = [this]() {
        audioEngine.getSequencer().clearTrack(trackIndex);
        updateStepButtonStyles();
    };
    addAndMakeVisible(clearButton);

    // Randomize Track Button
    randomizeButton.setColour(juce::TextButton::buttonColourId, CustomLookAndFeel::colorPanel2);
    randomizeButton.setColour(juce::TextButton::textColourOffId, CustomLookAndFeel::colorGreen);
    randomizeButton.onClick = [this]() {
        audioEngine.getSequencer().randomizeTrack(trackIndex);
        updateStepButtonStyles();
    };
    addAndMakeVisible(randomizeButton);

    // Vintage Sampler Mode Button
    vintageModeButton.setColour(juce::TextButton::buttonColourId, CustomLookAndFeel::colorPanel2);
    vintageModeButton.setColour(juce::TextButton::textColourOffId, CustomLookAndFeel::colorLed);
    vintageModeButton.onClick = [this, track]() {
        if (track)
        {
            track->cycleVintageMode();
            updateVintageButtonText();
        }
    };
    addAndMakeVisible(vintageModeButton);
    updateVintageButtonText();

    // Knobs
    volKnob = std::make_unique<RotaryKnob>("VOL", 0.0, 1.0, 0.01, 0.8, [](double v) {
        return juce::String(juce::roundToInt(v * 100.0));
    });
    volKnob->onValueChange = [track](double v) { if (track) track->setVolume(static_cast<float>(v)); };
    addAndMakeVisible(*volKnob);

    pitchKnob = std::make_unique<RotaryKnob>("PIT", -12.0, 12.0, 1.0, 0.0, [](double v) {
        int val = juce::roundToInt(v);
        return (val > 0 ? "+" : "") + juce::String(val);
    });
    pitchKnob->onValueChange = [track](double v) { if (track) track->setPitch(static_cast<float>(v)); };
    addAndMakeVisible(*pitchKnob);

    atkKnob = std::make_unique<RotaryKnob>("ATK", 0.001, 0.1, 0.001, 0.001, [](double v) {
        return juce::String(juce::roundToInt(v * 1000.0)) + "m";
    });
    atkKnob->onValueChange = [track](double v) { if (track) track->setAttack(static_cast<float>(v)); };
    addAndMakeVisible(*atkKnob);

    fltKnob = std::make_unique<RotaryKnob>("FLT", 200.0, 20000.0, 100.0, 20000.0, [](double v) {
        return (v >= 1000.0) ? juce::String(juce::roundToInt(v / 1000.0)) + "k" : juce::String(juce::roundToInt(v));
    });
    fltKnob->onValueChange = [track](double v) { if (track) track->setCutoff(static_cast<float>(v)); };
    addAndMakeVisible(*fltKnob);

    crunchKnob = std::make_unique<RotaryKnob>("CRUNCH", 0.0, 1.0, 0.01, 0.0, [](double v) {
        return juce::String(juce::roundToInt(v * 100.0));
    });
    crunchKnob->onValueChange = [track](double v) { if (track) track->setCrunch(static_cast<float>(v)); };
    addAndMakeVisible(*crunchKnob);

    lenKnob = std::make_unique<RotaryKnob>("LEN", 0.05, 1.0, 0.01, 1.0, [](double v) {
        return juce::String(juce::roundToInt(v * 100.0));
    });
    lenKnob->onValueChange = [track](double v) { if (track) track->setLength(static_cast<float>(v)); };
    addAndMakeVisible(*lenKnob);

    revKnob = std::make_unique<RotaryKnob>("REV", 0.0, 1.0, 0.01, 0.0, [](double v) {
        return juce::String(juce::roundToInt(v * 100.0));
    });
    revKnob->onValueChange = [track](double v) { if (track) track->setReverbSend(static_cast<float>(v)); };
    addAndMakeVisible(*revKnob);

    // 16 Step Buttons for current view page
    for (int i = 0; i < 16; ++i)
    {
        auto btn = std::make_unique<juce::TextButton>("");
        int localIndex = i;
        btn->onClick = [this, localIndex]() {
            int stepIndex = currentBank * 16 + localIndex;
            audioEngine.getSequencer().toggleStepState(trackIndex, stepIndex);
            updateStepButtonStyles();
        };
        addAndMakeVisible(*btn);
        stepButtons.push_back(std::move(btn));
    }

    updateStepButtonStyles();
}

void TrackRowComponent::paint(juce::Graphics& g)
{
    g.setColour(CustomLookAndFeel::colorPanel);
    g.fillRoundedRectangle(getLocalBounds().toFloat(), 4.0f);

    g.setColour(CustomLookAndFeel::colorLine);
    g.drawRoundedRectangle(getLocalBounds().toFloat(), 4.0f, 1.0f);
}

void TrackRowComponent::resized()
{
    int leftMargin = 6;
    trackNameLabel.setBounds(leftMargin, 2, 68, 14);
    sampleNameLabel.setBounds(leftMargin, 16, 68, 12);

    int btnY = 32;
    muteButton.setBounds(leftMargin, btnY, 15, 16);
    soloButton.setBounds(leftMargin + 17, btnY, 15, 16);
    loadButton.setBounds(leftMargin + 34, btnY, 32, 16);

    clearButton.setBounds(70, 2, 28, 16);
    randomizeButton.setBounds(70, 20, 28, 16);

    vintageModeButton.setBounds(102, 2, 42, 48);

    int knobX = 148;
    int knobW = 38;
    int knobGap = 2;

    volKnob->setBounds(knobX, 2, knobW, 48); knobX += knobW + knobGap;
    pitchKnob->setBounds(knobX, 2, knobW, 48); knobX += knobW + knobGap;
    atkKnob->setBounds(knobX, 2, knobW, 48); knobX += knobW + knobGap;
    fltKnob->setBounds(knobX, 2, knobW, 48); knobX += knobW + knobGap;
    crunchKnob->setBounds(knobX, 2, knobW, 48); knobX += knobW + knobGap;
    lenKnob->setBounds(knobX, 2, knobW, 48); knobX += knobW + knobGap;
    revKnob->setBounds(knobX, 2, knobW, 48);

    // 16 Steps
    int stepX = 432;
    int stepW = (getWidth() - stepX - (15 * 4) - 8) / 16;
    int stepH = 40;

    for (int i = 0; i < 16; ++i)
    {
        stepButtons[i]->setBounds(stepX + i * (stepW + 4), 6, stepW, stepH);
    }
}

void TrackRowComponent::updateVintageButtonText()
{
    if (auto* track = audioEngine.getTrack(trackIndex))
    {
        vintageModeButton.setButtonText(BreakStepAudio::VintageSamplerDSP::getModeName(track->getVintageMode()));
    }
}

void TrackRowComponent::setViewBank(int bank)
{
    currentBank = bank;
    updateStepButtonStyles();
}

void TrackRowComponent::updatePlayhead(int step)
{
    activePlayheadStep = step;
    updateStepButtonStyles();
}

void TrackRowComponent::updateAllControlsFromState()
{
    if (auto* track = audioEngine.getTrack(trackIndex))
    {
        volKnob->setValue(track->getVolume(), juce::dontSendNotification);
        pitchKnob->setValue(track->getPitch(), juce::dontSendNotification);
        atkKnob->setValue(track->getAttack(), juce::dontSendNotification);
        fltKnob->setValue(track->getCutoff(), juce::dontSendNotification);
        crunchKnob->setValue(track->getCrunch(), juce::dontSendNotification);
        lenKnob->setValue(track->getLength(), juce::dontSendNotification);
        revKnob->setValue(track->getReverbSend(), juce::dontSendNotification);

        muteButton.setToggleState(track->getMuted(), juce::dontSendNotification);
        soloButton.setToggleState(track->getSoloed(), juce::dontSendNotification);

        sampleNameLabel.setText(track->getSampleName(), juce::dontSendNotification);
        updateVintageButtonText();
    }
    updateStepButtonStyles();
}

void TrackRowComponent::updateStepButtonStyles()
{
    const auto& seq = audioEngine.getSequencer();

    for (int i = 0; i < 16; ++i)
    {
        int globalStep = currentBank * 16 + i;
        int state = seq.getStepState(trackIndex, globalStep);
        bool isPlayheadHere = (activePlayheadStep == globalStep);

        auto* btn = stepButtons[i].get();

        juce::Colour bg = CustomLookAndFeel::colorPanel2;
        juce::Colour textCol = CustomLookAndFeel::colorTextDim;

        if (state == 1) // Normal
        {
            bg = CustomLookAndFeel::colorLedDim;
            textCol = CustomLookAndFeel::colorLed;
        }
        else if (state == 2) // Accent
        {
            bg = CustomLookAndFeel::colorLed;
            textCol = CustomLookAndFeel::colorBg;
        }

        if (isPlayheadHere)
        {
            btn->setColour(juce::TextButton::buttonColourId, bg.brighter(0.3f));
            btn->setColour(juce::TextButton::textColourOffId, CustomLookAndFeel::colorCyan);
        }
        else
        {
            btn->setColour(juce::TextButton::buttonColourId, bg);
            btn->setColour(juce::TextButton::textColourOffId, textCol);
        }

        btn->repaint();
    }

    if (auto* track = audioEngine.getTrack(trackIndex))
    {
        sampleNameLabel.setText(track->getSampleName(), juce::dontSendNotification);
    }
}

void TrackRowComponent::openFileChooser()
{
    fileChooser = std::make_unique<juce::FileChooser>(
        "Select Audio Sample for " + trackNameLabel.getText(),
        juce::File::getSpecialLocation(juce::File::userHomeDirectory),
        "*.wav;*.aif;*.aiff;*.mp3;*.flac"
    );

    auto flags = juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles;

    fileChooser->launchAsync(flags, [this](const juce::FileChooser& fc) {
        auto file = fc.getResult();
        if (file.existsAsFile())
        {
            if (auto* track = audioEngine.getTrack(trackIndex))
            {
                track->loadSampleFile(file, audioEngine.getFormatManager());
                sampleNameLabel.setText(track->getSampleName(), juce::dontSendNotification);
            }
        }
    });
}

bool TrackRowComponent::isInterestedInFileDrag(const juce::StringArray& files)
{
    for (const auto& f : files)
    {
        juce::File file(f);
        auto ext = file.getFileExtension().toLowerCase();
        if (ext == ".wav" || ext == ".aif" || ext == ".aiff" || ext == ".mp3" || ext == ".flac")
            return true;
    }
    return false;
}

void TrackRowComponent::filesDropped(const juce::StringArray& files, int /*x*/, int /*y*/)
{
    if (files.size() > 0)
    {
        juce::File file(files[0]);
        if (auto* track = audioEngine.getTrack(trackIndex))
        {
            track->loadSampleFile(file, audioEngine.getFormatManager());
            sampleNameLabel.setText(track->getSampleName(), juce::dontSendNotification);
        }
    }
}

} // namespace BreakStepUI
