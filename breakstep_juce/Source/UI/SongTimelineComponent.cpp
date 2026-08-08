#include "SongTimelineComponent.h"
#include "CustomLookAndFeel.h"

namespace BreakStepUI
{

SongTimelineComponent::SongTimelineComponent(BreakStepAudio::AudioEngine& engine)
    : audioEngine(engine)
{
    titleLabel.setText("SONG TIMELINE (REASON-STYLE ARRANGER - 32 BARS)", juce::dontSendNotification);
    titleLabel.setFont(juce::FontOptions(11.0f).withStyle("Bold"));
    titleLabel.setColour(juce::Label::textColourId, CustomLookAndFeel::colorCyan);
    addAndMakeVisible(titleLabel);

    clearButton.setColour(juce::TextButton::buttonColourId, CustomLookAndFeel::colorPanel2);
    clearButton.setColour(juce::TextButton::textColourOffId, CustomLookAndFeel::colorRed);
    clearButton.onClick = [this]() {
        audioEngine.getSongTimeline().clearTimeline();
        repaint();
    };
    addAndMakeVisible(clearButton);

    stdArrButton.setColour(juce::TextButton::buttonColourId, CustomLookAndFeel::colorPanel2);
    stdArrButton.setColour(juce::TextButton::textColourOffId, CustomLookAndFeel::colorGreen);
    stdArrButton.onClick = [this]() {
        audioEngine.getSongTimeline().fillStandardArrangement();
        repaint();
    };
    addAndMakeVisible(stdArrButton);

    repeat4Button.setColour(juce::TextButton::buttonColourId, CustomLookAndFeel::colorPanel2);
    repeat4Button.setColour(juce::TextButton::textColourOffId, CustomLookAndFeel::colorLed);
    repeat4Button.onClick = [this]() {
        auto& tl = audioEngine.getSongTimeline();
        for (int b = 4; b < BreakStepAudio::SongTimeline::NUM_BARS; ++b)
        {
            tl.setDrumBlock(b, tl.getDrumBlock(b % 4));
            tl.setSlicerBlock(b, tl.getSlicerBlock(b % 4));
        }
        repaint();
    };
    addAndMakeVisible(repeat4Button);
}

void SongTimelineComponent::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();

    g.setColour(CustomLookAndFeel::colorPanel);
    g.fillRoundedRectangle(bounds, 6.0f);
    g.setColour(CustomLookAndFeel::colorLine);
    g.drawRoundedRectangle(bounds, 6.0f, 1.0f);

    int numBars = BreakStepAudio::SongTimeline::NUM_BARS;
    int gridX = 84;
    int gridW = getWidth() - gridX - 16;
    float barW = static_cast<float>(gridW) / numBars;

    rulerArea = juce::Rectangle<int>(gridX, 32, gridW, 16);
    drumLaneArea = juce::Rectangle<int>(gridX, 50, gridW, 26);
    slicerLaneArea = juce::Rectangle<int>(gridX, 78, gridW, 26);

    // Track Lane Headers on the left
    g.setFont(juce::FontOptions(10.0f).withStyle("Bold"));
    g.setColour(CustomLookAndFeel::colorLed);
    g.drawText("DRUMS", 12, drumLaneArea.getY(), 68, drumLaneArea.getHeight(), juce::Justification::centredLeft);

    g.setColour(CustomLookAndFeel::colorCyan);
    g.drawText("CHOPS", 12, slicerLaneArea.getY(), 68, slicerLaneArea.getHeight(), juce::Justification::centredLeft);

    auto& tl = audioEngine.getSongTimeline();
    int currentBar = tl.getCurrentBar();
    float progress = tl.getBarProgress();

    // 1. Draw 32 Bars Grid & Ruler
    for (int b = 0; b < numBars; ++b)
    {
        float x = gridX + b * barW;
        bool isDownbeat = (b % 4 == 0);

        // Ruler Bar Number
        g.setFont(juce::FontOptions(8.0f).withStyle(isDownbeat ? "Bold" : "Plain"));
        g.setColour(isDownbeat ? CustomLookAndFeel::colorText : CustomLookAndFeel::colorTextDim);
        g.drawText(juce::String(b + 1), juce::Rectangle<float>(x, rulerArea.getY(), barW, rulerArea.getHeight()), juce::Justification::centred);

        // Drum Lane Block
        int dBlock = tl.getDrumBlock(b);
        auto drumBlockRect = juce::Rectangle<float>(x + 1, drumLaneArea.getY() + 1, barW - 2, drumLaneArea.getHeight() - 2);

        if (dBlock > 0)
        {
            g.setColour(CustomLookAndFeel::colorLedDim.withAlpha(0.6f));
            g.fillRoundedRectangle(drumBlockRect, 2.0f);
            g.setColour(CustomLookAndFeel::colorLed);
            g.drawRoundedRectangle(drumBlockRect, 2.0f, 1.0f);
            g.setFont(juce::FontOptions(9.0f).withStyle("Bold"));
            g.drawText("D" + juce::String(dBlock), drumBlockRect, juce::Justification::centred);
        }
        else
        {
            g.setColour(CustomLookAndFeel::colorPanel2);
            g.fillRoundedRectangle(drumBlockRect, 2.0f);
            g.setColour(CustomLookAndFeel::colorLine);
            g.drawRoundedRectangle(drumBlockRect, 2.0f, 0.5f);
        }

        // Slicer Lane Block
        int sBlock = tl.getSlicerBlock(b);
        auto slicerBlockRect = juce::Rectangle<float>(x + 1, slicerLaneArea.getY() + 1, barW - 2, slicerLaneArea.getHeight() - 2);

        if (sBlock > 0)
        {
            g.setColour(CustomLookAndFeel::colorCyan.withAlpha(0.4f));
            g.fillRoundedRectangle(slicerBlockRect, 2.0f);
            g.setColour(CustomLookAndFeel::colorCyan);
            g.drawRoundedRectangle(slicerBlockRect, 2.0f, 1.0f);
            g.setFont(juce::FontOptions(9.0f).withStyle("Bold"));
            g.drawText("S" + juce::String(sBlock), slicerBlockRect, juce::Justification::centred);
        }
        else
        {
            g.setColour(CustomLookAndFeel::colorPanel2);
            g.fillRoundedRectangle(slicerBlockRect, 2.0f);
            g.setColour(CustomLookAndFeel::colorLine);
            g.drawRoundedRectangle(slicerBlockRect, 2.0f, 0.5f);
        }
    }

    // 2. Draw Song Playhead if Song Mode is active and playing
    if (audioEngine.getSequencer().isPlaying() &&
        audioEngine.getPlaybackMode() == BreakStepAudio::PlaybackMode::SongTimeline)
    {
        float playheadX = gridX + (currentBar + progress) * barW;
        g.setColour(CustomLookAndFeel::colorGreen);
        g.drawLine(playheadX, rulerArea.getY(), playheadX, slicerLaneArea.getBottom(), 2.0f);

        // Arrow marker on top of ruler
        juce::Path arrow;
        arrow.startNewSubPath(playheadX - 4, rulerArea.getY());
        arrow.lineTo(playheadX + 4, rulerArea.getY());
        arrow.lineTo(playheadX, rulerArea.getY() + 6);
        arrow.closeSubPath();
        g.fillPath(arrow);
    }
}

void SongTimelineComponent::resized()
{
    titleLabel.setBounds(14, 6, 320, 20);

    int btnW = 90;
    int btnH = 20;
    int rightX = getWidth() - 14 - (3 * (btnW + 6));

    clearButton.setBounds(rightX, 6, btnW, btnH);
    rightX += btnW + 6;

    stdArrButton.setBounds(rightX, 6, btnW, btnH);
    rightX += btnW + 6;

    repeat4Button.setBounds(rightX, 6, btnW, btnH);
}

void SongTimelineComponent::mouseDown(const juce::MouseEvent& e)
{
    int numBars = BreakStepAudio::SongTimeline::NUM_BARS;
    if (drumLaneArea.getWidth() <= 0) return;

    float barW = static_cast<float>(drumLaneArea.getWidth()) / numBars;
    int barIndex = static_cast<int>((e.x - drumLaneArea.getX()) / barW);

    if (barIndex < 0 || barIndex >= numBars) return;

    auto& tl = audioEngine.getSongTimeline();

    if (drumLaneArea.contains(e.getPosition()))
    {
        if (e.mods.isPopupMenu() || e.mods.isCtrlDown())
        {
            tl.setDrumBlock(barIndex, 0); // Mute
        }
        else
        {
            int cur = tl.getDrumBlock(barIndex);
            tl.setDrumBlock(barIndex, (cur + 1) % 9); // Cycle 0..8
        }
        repaint();
    }
    else if (slicerLaneArea.contains(e.getPosition()))
    {
        if (e.mods.isPopupMenu() || e.mods.isCtrlDown())
        {
            tl.setSlicerBlock(barIndex, 0); // Mute
        }
        else
        {
            int cur = tl.getSlicerBlock(barIndex);
            tl.setSlicerBlock(barIndex, (cur + 1) % 9); // Cycle 0..8
        }
        repaint();
    }
}

void SongTimelineComponent::updatePlayhead()
{
    repaint();
}

} // namespace BreakStepUI
