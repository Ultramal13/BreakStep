#include "SliceSequencerComponent.h"
#include "CustomLookAndFeel.h"

namespace BreakStepUI
{

SliceSequencerComponent::SliceSequencerComponent(BreakStepAudio::AudioEngine& engine)
    : audioEngine(engine)
{
    for (int i = 0; i < 16; ++i)
    {
        StepUI ui;
        int stepIdx = i;

        // Main Step Pad
        ui.mainPad = std::make_unique<juce::TextButton>("S" + juce::String(i + 1));
        ui.mainPad->onClick = [this, stepIdx]() {
            audioEngine.getSliceSequencer().toggleStepActive(stepIdx);
            updateStepButtons();
        };
        addAndMakeVisible(*ui.mainPad);

        // Ratchet Button (1x, 2x, 3x, 4x rolls)
        ui.ratchetBtn = std::make_unique<juce::TextButton>("1x");
        ui.ratchetBtn->onClick = [this, stepIdx]() {
            audioEngine.getSliceSequencer().cycleRatchets(stepIdx);
            updateStepButtons();
        };
        addAndMakeVisible(*ui.ratchetBtn);

        // Probability Button (100%, 75%, 50%, 25%)
        ui.probBtn = std::make_unique<juce::TextButton>("100%");
        ui.probBtn->onClick = [this, stepIdx]() {
            audioEngine.getSliceSequencer().cycleProbability(stepIdx);
            updateStepButtons();
        };
        addAndMakeVisible(*ui.probBtn);

        // Reverse Toggle Button
        ui.revBtn = std::make_unique<juce::TextButton>("REV");
        ui.revBtn->onClick = [this, stepIdx]() {
            audioEngine.getSliceSequencer().toggleReverse(stepIdx);
            updateStepButtons();
        };
        addAndMakeVisible(*ui.revBtn);

        stepControls.push_back(std::move(ui));
    }

    updateStepButtons();
}

void SliceSequencerComponent::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    g.setColour(CustomLookAndFeel::colorPanel);
    g.fillRoundedRectangle(bounds, 6.0f);
    g.setColour(CustomLookAndFeel::colorLine);
    g.drawRoundedRectangle(bounds, 6.0f, 1.0f);

    // Section title
    g.setFont(juce::FontOptions(11.0f).withStyle("Bold"));
    g.setColour(CustomLookAndFeel::colorLed);
    g.drawText("16-STEP CHOP SEQUENCER (RATCHETS & PROBABILITY)", 14, 8, 400, 16, juce::Justification::left);
}

void SliceSequencerComponent::resized()
{
    int stepW = (getWidth() - 28 - (15 * 4)) / 16;
    int startY = 30;

    for (int i = 0; i < 16; ++i)
    {
        int x = 14 + i * (stepW + 4);

        stepControls[i].mainPad->setBounds(x, startY, stepW, 46);
        stepControls[i].ratchetBtn->setBounds(x, startY + 50, stepW, 16);
        stepControls[i].probBtn->setBounds(x, startY + 68, stepW, 16);
        stepControls[i].revBtn->setBounds(x, startY + 86, stepW, 16);
    }
}

void SliceSequencerComponent::updatePlayhead(int step)
{
    activePlayheadStep = step;
    updateStepButtons();
}

void SliceSequencerComponent::updateAllControls()
{
    updateStepButtons();
}

void SliceSequencerComponent::updateStepButtons()
{
    const auto& seq = audioEngine.getSliceSequencer();

    for (int i = 0; i < 16; ++i)
    {
        auto step = seq.getStep(i);
        bool isPlayhead = (activePlayheadStep == i);

        auto* pad = stepControls[i].mainPad.get();
        pad->setButtonText("S" + juce::String(step.sliceIndex + 1));

        if (step.active)
        {
            pad->setColour(juce::TextButton::buttonColourId, isPlayhead ? CustomLookAndFeel::colorCyan : CustomLookAndFeel::colorLed);
            pad->setColour(juce::TextButton::textColourOffId, CustomLookAndFeel::colorBg);
        }
        else
        {
            pad->setColour(juce::TextButton::buttonColourId, isPlayhead ? CustomLookAndFeel::colorPanel2.brighter(0.3f) : CustomLookAndFeel::colorPanel2);
            pad->setColour(juce::TextButton::textColourOffId, CustomLookAndFeel::colorTextDim);
        }

        // Ratchet text & color
        stepControls[i].ratchetBtn->setButtonText(juce::String(step.ratchets) + "x");
        stepControls[i].ratchetBtn->setColour(juce::TextButton::textColourOffId, (step.ratchets > 1) ? CustomLookAndFeel::colorGreen : CustomLookAndFeel::colorTextDim);

        // Probability text & color
        stepControls[i].probBtn->setButtonText(juce::String(juce::roundToInt(step.probability * 100)) + "%");
        stepControls[i].probBtn->setColour(juce::TextButton::textColourOffId, (step.probability < 0.99f) ? CustomLookAndFeel::colorRed : CustomLookAndFeel::colorTextDim);

        // Reverse text & color
        stepControls[i].revBtn->setColour(juce::TextButton::textColourOffId, step.reverse ? CustomLookAndFeel::colorCyan : CustomLookAndFeel::colorTextDim);

        pad->repaint();
    }
}

} // namespace BreakStepUI
