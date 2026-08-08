#include "ChopTrackRowComponent.h"
#include "CustomLookAndFeel.h"

namespace BreakStepUI
{

ChopTrackRowComponent::ChopTrackRowComponent(BreakStepAudio::AudioEngine& engine)
    : audioEngine(engine)
{
    trackNameLabel.setText("CHOP SEQ", juce::dontSendNotification);
    trackNameLabel.setFont(juce::FontOptions(11.0f).withStyle("Bold"));
    trackNameLabel.setColour(juce::Label::textColourId, CustomLookAndFeel::colorCyan);
    addAndMakeVisible(trackNameLabel);

    sampleNameLabel.setText("amen_break.wav", juce::dontSendNotification);
    sampleNameLabel.setFont(juce::FontOptions(8.0f));
    sampleNameLabel.setColour(juce::Label::textColourId, CustomLookAndFeel::colorLed);
    addAndMakeVisible(sampleNameLabel);

    // Clear and Randomize Buttons
    clearButton.setColour(juce::TextButton::buttonColourId, CustomLookAndFeel::colorPanel2);
    clearButton.setColour(juce::TextButton::textColourOffId, CustomLookAndFeel::colorRed);
    clearButton.onClick = [this]() {
        audioEngine.getSliceSequencer().clear();
        updateStepButtonStyles();
    };
    addAndMakeVisible(clearButton);

    randomizeButton.setColour(juce::TextButton::buttonColourId, CustomLookAndFeel::colorPanel2);
    randomizeButton.setColour(juce::TextButton::textColourOffId, CustomLookAndFeel::colorGreen);
    randomizeButton.onClick = [this]() {
        audioEngine.getSliceSequencer().randomize();
        updateStepButtonStyles();
    };
    addAndMakeVisible(randomizeButton);

    // Knobs for Chop Track
    volKnob = std::make_unique<RotaryKnob>("VOL", 0.0, 1.0, 0.01, 0.85, [](double v) {
        return juce::String(juce::roundToInt(v * 100.0));
    });
    volKnob->onValueChange = [this](double v) {
        audioEngine.getSlicer().setVolume(static_cast<float>(v));
    };
    addAndMakeVisible(*volKnob);

    fltKnob = std::make_unique<RotaryKnob>("FLT", 200.0, 20000.0, 100.0, 20000.0, [](double v) {
        return (v >= 1000.0) ? juce::String(juce::roundToInt(v / 1000.0)) + "k" : juce::String(juce::roundToInt(v));
    });
    fltKnob->onValueChange = [this](double v) {
        audioEngine.getSlicer().setCutoff(static_cast<float>(v));
    };
    addAndMakeVisible(*fltKnob);

    revKnob = std::make_unique<RotaryKnob>("REV", 0.0, 1.0, 0.01, 0.1, [](double v) {
        return juce::String(juce::roundToInt(v * 100.0));
    });
    revKnob->onValueChange = [this](double v) {
        audioEngine.getSlicer().setReverbSend(static_cast<float>(v));
    };
    addAndMakeVisible(*revKnob);

    // 16 Step Buttons for Chop Track
    for (int i = 0; i < 16; ++i)
    {
        StepButtonUI ui;
        int stepIdx = i;

        ui.stepPad = std::make_unique<juce::TextButton>("S" + juce::String(i + 1));
        ui.stepPad->onClick = [this, stepIdx]() {
            if (juce::ModifierKeys::getCurrentModifiers().isPopupMenu() ||
                juce::ModifierKeys::getCurrentModifiers().isCtrlDown())
            {
                auto step = audioEngine.getSliceSequencer().getStep(stepIdx);
                int nextSlice = (step.sliceIndex + 1) % 16;
                audioEngine.getSliceSequencer().setStepSlice(stepIdx, nextSlice);
            }
            else
            {
                audioEngine.getSliceSequencer().toggleStepActive(stepIdx);
            }
            updateStepButtonStyles();
        };
        addAndMakeVisible(*ui.stepPad);

        ui.ratchetBtn = std::make_unique<juce::TextButton>("1x");
        ui.ratchetBtn->onClick = [this, stepIdx]() {
            audioEngine.getSliceSequencer().cycleRatchets(stepIdx);
            updateStepButtonStyles();
        };
        addAndMakeVisible(*ui.ratchetBtn);

        ui.probBtn = std::make_unique<juce::TextButton>("100%");
        ui.probBtn->onClick = [this, stepIdx]() {
            audioEngine.getSliceSequencer().cycleProbability(stepIdx);
            updateStepButtonStyles();
        };
        addAndMakeVisible(*ui.probBtn);

        stepButtons.push_back(std::move(ui));
    }

    updateStepButtonStyles();
}

void ChopTrackRowComponent::paint(juce::Graphics& g)
{
    g.setColour(CustomLookAndFeel::colorPanel);
    g.fillRoundedRectangle(getLocalBounds().toFloat(), 4.0f);

    g.setColour(CustomLookAndFeel::colorCyan.withAlpha(0.7f));
    g.drawRoundedRectangle(getLocalBounds().toFloat(), 4.0f, 1.0f);
}

void ChopTrackRowComponent::resized()
{
    int leftMargin = 6;
    trackNameLabel.setBounds(leftMargin, 4, 76, 16);
    sampleNameLabel.setBounds(leftMargin, 20, 76, 12);

    int btnY = 34;
    clearButton.setBounds(leftMargin, btnY, 34, 18);
    randomizeButton.setBounds(leftMargin + 38, btnY, 34, 18);

    int knobX = 84;
    int knobW = 40;
    int knobGap = 4;

    volKnob->setBounds(knobX, 2, knobW, 52); knobX += knobW + knobGap;
    fltKnob->setBounds(knobX, 2, knobW, 52); knobX += knobW + knobGap;
    revKnob->setBounds(knobX, 2, knobW, 52);

    // 16 Steps
    int stepX = 222;
    int stepW = (getWidth() - stepX - (15 * 4) - 8) / 16;
    int stepH = 26;

    for (int i = 0; i < 16; ++i)
    {
        int x = stepX + i * (stepW + 4);
        stepButtons[i].stepPad->setBounds(x, 4, stepW, stepH);
        stepButtons[i].ratchetBtn->setBounds(x, 32, stepW / 2 - 1, 14);
        stepButtons[i].probBtn->setBounds(x + stepW / 2, 32, stepW / 2 + 1, 14);
    }
}

void ChopTrackRowComponent::updatePlayhead(int step)
{
    activePlayheadStep = step;
    updateStepButtonStyles();
}

void ChopTrackRowComponent::updateAllControls()
{
    volKnob->setValue(audioEngine.getSlicer().getVolume(), juce::dontSendNotification);
    fltKnob->setValue(audioEngine.getSlicer().getCutoff(), juce::dontSendNotification);
    revKnob->setValue(audioEngine.getSlicer().getReverbSend(), juce::dontSendNotification);
    sampleNameLabel.setText(audioEngine.getSlicer().getLoadedFileName(audioEngine.getSlicer().getActiveSlot()), juce::dontSendNotification);
    updateStepButtonStyles();
}

void ChopTrackRowComponent::updateStepButtonStyles()
{
    const auto& seq = audioEngine.getSliceSequencer();

    for (int i = 0; i < 16; ++i)
    {
        auto step = seq.getStep(i);
        bool isPlayhead = (activePlayheadStep == i);

        auto* pad = stepButtons[i].stepPad.get();
        pad->setButtonText("S" + juce::String(step.sliceIndex + 1));

        if (step.active)
        {
            pad->setColour(juce::TextButton::buttonColourId, isPlayhead ? CustomLookAndFeel::colorCyan.brighter(0.3f) : CustomLookAndFeel::colorCyan);
            pad->setColour(juce::TextButton::textColourOffId, CustomLookAndFeel::colorBg);
        }
        else
        {
            pad->setColour(juce::TextButton::buttonColourId, isPlayhead ? CustomLookAndFeel::colorPanel2.brighter(0.3f) : CustomLookAndFeel::colorPanel2);
            pad->setColour(juce::TextButton::textColourOffId, CustomLookAndFeel::colorTextDim);
        }

        stepButtons[i].ratchetBtn->setButtonText(juce::String(step.ratchets) + "x");
        stepButtons[i].ratchetBtn->setColour(juce::TextButton::textColourOffId, (step.ratchets > 1) ? CustomLookAndFeel::colorGreen : CustomLookAndFeel::colorTextDim);

        stepButtons[i].probBtn->setButtonText(juce::String(juce::roundToInt(step.probability * 100)) + "%");
        stepButtons[i].probBtn->setColour(juce::TextButton::textColourOffId, (step.probability < 0.99f) ? CustomLookAndFeel::colorRed : CustomLookAndFeel::colorTextDim);

        pad->repaint();
    }
}

} // namespace BreakStepUI
