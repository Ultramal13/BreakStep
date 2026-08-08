#include "ChopperWaveformComponent.h"
#include "CustomLookAndFeel.h"

namespace BreakStepUI
{

ChopperWaveformComponent::ChopperWaveformComponent(BreakStepAudio::AudioEngine& engine)
    : audioEngine(engine)
{
    // Slot Buttons (SLOT 1 to 4)
    for (int i = 0; i < 4; ++i)
    {
        slotButtons[i].setButtonText("SLOT " + juce::String(i + 1));
        slotButtons[i].setColour(juce::TextButton::buttonColourId, CustomLookAndFeel::colorPanel2);
        slotButtons[i].setColour(juce::TextButton::textColourOffId, CustomLookAndFeel::colorTextDim);
        int slotIdx = i;
        slotButtons[i].onClick = [this, slotIdx]() {
            audioEngine.getSlicer().setActiveSlot(slotIdx);
            updateSlotButtonStyles();
            fileNameLabel.setText(audioEngine.getSlicer().getLoadedFileName(slotIdx), juce::dontSendNotification);
            updatePadLabels();
            repaint();
        };
        addAndMakeVisible(slotButtons[i]);
    }
    updateSlotButtonStyles();

    loadLoopButton.setColour(juce::TextButton::buttonColourId, CustomLookAndFeel::colorPanel2);
    loadLoopButton.setColour(juce::TextButton::textColourOffId, CustomLookAndFeel::colorCyan);
    loadLoopButton.onClick = [this]() { openLoopChooser(); };
    addAndMakeVisible(loadLoopButton);

    resetTransientsButton.setColour(juce::TextButton::buttonColourId, CustomLookAndFeel::colorPanel2);
    resetTransientsButton.setColour(juce::TextButton::textColourOffId, CustomLookAndFeel::colorLed);
    resetTransientsButton.onClick = [this]() {
        if (sensitivityKnob)
        {
            audioEngine.getSlicer().detectTransients(static_cast<float>(sensitivityKnob->getValue()));
            updatePadLabels();
            repaint();
        }
    };
    addAndMakeVisible(resetTransientsButton);

    fileNameLabel.setText("Amen Break (170 BPM)", juce::dontSendNotification);
    fileNameLabel.setFont(juce::FontOptions(11.0f).withStyle("Bold"));
    fileNameLabel.setColour(juce::Label::textColourId, CustomLookAndFeel::colorText);
    addAndMakeVisible(fileNameLabel);

    selectedSliceLabel.setText("EDITING: SLICE 1", juce::dontSendNotification);
    selectedSliceLabel.setFont(juce::FontOptions(10.0f).withStyle("Bold"));
    selectedSliceLabel.setColour(juce::Label::textColourId, CustomLookAndFeel::colorLed);
    addAndMakeVisible(selectedSliceLabel);

    // Nudge Slice Line Knob
    nudgeKnob = std::make_unique<RotaryKnob>("NUDGE", -500.0, 500.0, 10.0, 0.0, [](double v) {
        int val = juce::roundToInt(v);
        return (val > 0 ? "+" : "") + juce::String(val);
    });
    nudgeKnob->onValueChange = [this](double val) {
        if (activeAuditionSlice > 0)
        {
            audioEngine.getSlicer().nudgeSliceMarker(activeAuditionSlice, juce::roundToInt(val));
            repaint();
        }
    };
    addAndMakeVisible(*nudgeKnob);

    // Transient Sensitivity Knob
    sensitivityKnob = std::make_unique<RotaryKnob>("SENSITIVITY", 0.05, 0.95, 0.01, 0.5, [](double v) {
        return juce::String(juce::roundToInt(v * 100.0)) + "%";
    });
    sensitivityKnob->onValueChange = [this](double val) {
        audioEngine.getSlicer().detectTransients(static_cast<float>(val));
        updatePadLabels();
        repaint();
    };
    addAndMakeVisible(*sensitivityKnob);

    // Volume Knob
    volumeKnob = std::make_unique<RotaryKnob>("VOL", 0.0, 1.0, 0.01, 0.85, [](double v) {
        return juce::String(juce::roundToInt(v * 100.0));
    });
    volumeKnob->onValueChange = [this](double val) {
        audioEngine.getSlicer().setVolume(static_cast<float>(val));
    };
    addAndMakeVisible(*volumeKnob);

    // Cutoff Knob
    cutoffKnob = std::make_unique<RotaryKnob>("FILTER", 200.0, 20000.0, 100.0, 20000.0, [](double v) {
        return (v >= 1000.0) ? juce::String(juce::roundToInt(v / 1000.0)) + "k" : juce::String(juce::roundToInt(v));
    });
    cutoffKnob->onValueChange = [this](double val) {
        audioEngine.getSlicer().setCutoff(static_cast<float>(val));
    };
    addAndMakeVisible(*cutoffKnob);

    // Reverb Knob
    reverbKnob = std::make_unique<RotaryKnob>("REV", 0.0, 1.0, 0.01, 0.1, [](double v) {
        return juce::String(juce::roundToInt(v * 100.0));
    });
    reverbKnob->onValueChange = [this](double val) {
        audioEngine.getSlicer().setReverbSend(static_cast<float>(val));
    };
    addAndMakeVisible(*reverbKnob);

    // 16 MPC Slice Pads
    for (int i = 0; i < 16; ++i)
    {
        auto pad = std::make_unique<juce::TextButton>("P" + juce::String(i + 1));
        pad->setColour(juce::TextButton::buttonColourId, CustomLookAndFeel::colorPanel2);
        pad->setColour(juce::TextButton::textColourOffId, CustomLookAndFeel::colorLed);

        int sliceIdx = i;
        pad->onClick = [this, sliceIdx]() {
            activeAuditionSlice = sliceIdx;
            selectedSliceLabel.setText("EDITING: SLICE " + juce::String(sliceIdx + 1), juce::dontSendNotification);
            audioEngine.getSlicer().triggerSlice(sliceIdx, 1.0f, 0.0f, false);
            repaint();
        };

        addAndMakeVisible(*pad);
        slicePads.push_back(std::move(pad));
    }
}

void ChopperWaveformComponent::updateSlotButtonStyles()
{
    int curSlot = audioEngine.getSlicer().getActiveSlot();
    for (int i = 0; i < 4; ++i)
    {
        if (i == curSlot)
        {
            slotButtons[i].setColour(juce::TextButton::buttonColourId, CustomLookAndFeel::colorCyan);
            slotButtons[i].setColour(juce::TextButton::textColourOffId, CustomLookAndFeel::colorBg);
        }
        else
        {
            slotButtons[i].setColour(juce::TextButton::buttonColourId, CustomLookAndFeel::colorPanel2);
            slotButtons[i].setColour(juce::TextButton::textColourOffId, CustomLookAndFeel::colorTextDim);
        }
    }
}

void ChopperWaveformComponent::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();

    g.setColour(CustomLookAndFeel::colorPanel);
    g.fillRoundedRectangle(bounds, 6.0f);
    g.setColour(CustomLookAndFeel::colorLine);
    g.drawRoundedRectangle(bounds, 6.0f, 1.0f);

    // Waveform display area
    waveArea = juce::Rectangle<int>(14, 38, getWidth() - 28, 92);
    g.setColour(CustomLookAndFeel::colorPanel2);
    g.fillRoundedRectangle(waveArea.toFloat(), 4.0f);
    g.setColour(CustomLookAndFeel::colorLine);
    g.drawRoundedRectangle(waveArea.toFloat(), 4.0f, 1.0f);

    const auto& buffer = audioEngine.getSlicer().getAudioBuffer();
    int numSamples = buffer.getNumSamples();

    if (numSamples > 0)
    {
        const float* readPtr = buffer.getReadPointer(0);
        int w = waveArea.getWidth();
        float midY = waveArea.getCentreY();
        float halfH = waveArea.getHeight() * 0.45f;

        int numSlices = audioEngine.getSlicer().getNumSlices();

        // 1. Draw Alternating Shaded Slice Regions
        for (int i = 0; i < numSlices; ++i)
        {
            auto slice = audioEngine.getSlicer().getSlice(i);
            float startX = waveArea.getX() + (static_cast<float>(slice.startSample) / numSamples) * w;
            float endX = waveArea.getX() + (static_cast<float>(slice.endSample) / numSamples) * w;

            juce::Colour bgShade = (i % 2 == 0) ? CustomLookAndFeel::colorPanel2.brighter(0.04f) : CustomLookAndFeel::colorPanel2;
            if (i == activeAuditionSlice)
            {
                bgShade = CustomLookAndFeel::colorLedDim.withAlpha(0.35f);
            }

            g.setColour(bgShade);
            g.fillRect(juce::Rectangle<float>(startX, waveArea.getY() + 1, endX - startX, waveArea.getHeight() - 2));
        }

        // 2. Draw Waveform Peaks
        juce::Path wavePath;
        wavePath.startNewSubPath(waveArea.getX(), midY);

        int samplesPerPixel = std::max(1, numSamples / w);
        for (int x = 0; x < w; ++x)
        {
            int startSample = x * samplesPerPixel;
            float maxVal = 0.0f;
            for (int s = 0; s < samplesPerPixel && (startSample + s) < numSamples; ++s)
            {
                float val = std::abs(readPtr[startSample + s]);
                if (val > maxVal) maxVal = val;
            }
            wavePath.lineTo(waveArea.getX() + x, midY - maxVal * halfH);
        }
        for (int x = w - 1; x >= 0; --x)
        {
            int startSample = x * samplesPerPixel;
            float maxVal = 0.0f;
            for (int s = 0; s < samplesPerPixel && (startSample + s) < numSamples; ++s)
            {
                float val = std::abs(readPtr[startSample + s]);
                if (val > maxVal) maxVal = val;
            }
            wavePath.lineTo(waveArea.getX() + x, midY + maxVal * halfH);
        }
        wavePath.closeSubPath();

        g.setColour(CustomLookAndFeel::colorCyan.withAlpha(0.6f));
        g.fillPath(wavePath);
        g.setColour(CustomLookAndFeel::colorCyan);
        g.strokePath(wavePath, juce::PathStrokeType(1.0f));

        // 3. Draw Slice Markers
        for (int i = 0; i < numSlices; ++i)
        {
            auto slice = audioEngine.getSlicer().getSlice(i);
            float sliceX = waveArea.getX() + (static_cast<float>(slice.startSample) / numSamples) * w;

            bool isSelected = (activeAuditionSlice == i);
            bool isDragged = (draggedSliceMarker == i);

            g.setColour(isDragged ? CustomLookAndFeel::colorGreen : (isSelected ? CustomLookAndFeel::colorCyan : CustomLookAndFeel::colorLed));
            g.drawVerticalLine(juce::roundToInt(sliceX), waveArea.getY(), waveArea.getBottom());

            auto badgeRect = juce::Rectangle<float>(sliceX + 2, waveArea.getY() + 2, 16, 12);
            g.setColour(CustomLookAndFeel::colorBg.withAlpha(0.75f));
            g.fillRoundedRectangle(badgeRect, 2.0f);

            g.setFont(juce::FontOptions(8.0f).withStyle("Bold"));
            g.setColour(isSelected ? CustomLookAndFeel::colorCyan : CustomLookAndFeel::colorLed);
            g.drawText(juce::String(i + 1), badgeRect.toNearestInt(), juce::Justification::centred);
        }
    }
}

void ChopperWaveformComponent::resized()
{
    int slotX = 14;
    for (int i = 0; i < 4; ++i)
    {
        slotButtons[i].setBounds(slotX, 8, 54, 24);
        slotX += 58;
    }

    loadLoopButton.setBounds(slotX + 4, 8, 100, 24);
    resetTransientsButton.setBounds(slotX + 108, 8, 82, 24);
    fileNameLabel.setBounds(slotX + 194, 8, 150, 24);
    selectedSliceLabel.setBounds(slotX + 348, 8, 100, 24);

    int knobX = getWidth() - 280;
    int knobW = 42;
    int knobGap = 8;

    nudgeKnob->setBounds(knobX, 2, knobW, 34); knobX += knobW + knobGap;
    sensitivityKnob->setBounds(knobX, 2, knobW, 34); knobX += knobW + knobGap;
    volumeKnob->setBounds(knobX, 2, knobW, 34); knobX += knobW + knobGap;
    cutoffKnob->setBounds(knobX, 2, knobW, 34); knobX += knobW + knobGap;
    reverbKnob->setBounds(knobX, 2, knobW, 34);

    // 16 MPC Slice Pads (2 rows of 8 pads, plenty of vertical room)
    int padAreaY = 138;
    int padW = (getWidth() - 28 - (7 * 6)) / 8;
    int padH = 28;

    for (int i = 0; i < 16; ++i)
    {
        int row = i / 8;
        int col = i % 8;
        slicePads[i]->setBounds(14 + col * (padW + 6), padAreaY + row * (padH + 6), padW, padH);
    }
}

int ChopperWaveformComponent::getSliceMarkerNearX(float x, float tolerancePx) const
{
    const auto& buffer = audioEngine.getSlicer().getAudioBuffer();
    int numSamples = buffer.getNumSamples();
    if (numSamples <= 0 || waveArea.getWidth() <= 0)
        return -1;

    int numSlices = audioEngine.getSlicer().getNumSlices();
    float w = waveArea.getWidth();

    for (int i = 1; i < numSlices; ++i)
    {
        auto slice = audioEngine.getSlicer().getSlice(i);
        float sliceX = waveArea.getX() + (static_cast<float>(slice.startSample) / numSamples) * w;
        if (std::abs(x - sliceX) <= tolerancePx)
        {
            return i;
        }
    }
    return -1;
}

int ChopperWaveformComponent::getSliceIndexAtX(float x) const
{
    const auto& buffer = audioEngine.getSlicer().getAudioBuffer();
    int numSamples = buffer.getNumSamples();
    if (numSamples <= 0 || waveArea.getWidth() <= 0)
        return -1;

    float relX = (x - waveArea.getX()) / waveArea.getWidth();
    int sample = static_cast<int>(relX * numSamples);

    int numSlices = audioEngine.getSlicer().getNumSlices();
    for (int i = 0; i < numSlices; ++i)
    {
        auto slice = audioEngine.getSlicer().getSlice(i);
        if (sample >= slice.startSample && sample < slice.endSample)
        {
            return i;
        }
    }
    return -1;
}

void ChopperWaveformComponent::mouseDown(const juce::MouseEvent& e)
{
    if (!waveArea.contains(e.getPosition()))
        return;

    draggedSliceMarker = getSliceMarkerNearX(static_cast<float>(e.x));

    if (draggedSliceMarker != -1)
    {
        activeAuditionSlice = draggedSliceMarker;
        selectedSliceLabel.setText("EDITING: SLICE " + juce::String(activeAuditionSlice + 1), juce::dontSendNotification);

        if (e.mods.isPopupMenu())
        {
            audioEngine.getSlicer().removeSliceMarker(draggedSliceMarker);
            draggedSliceMarker = -1;
            updatePadLabels();
            repaint();
            return;
        }
        repaint();
    }
    else
    {
        int sliceIdx = getSliceIndexAtX(static_cast<float>(e.x));
        if (sliceIdx != -1)
        {
            activeAuditionSlice = sliceIdx;
            selectedSliceLabel.setText("EDITING: SLICE " + juce::String(activeAuditionSlice + 1), juce::dontSendNotification);
            audioEngine.getSlicer().triggerSlice(sliceIdx, 1.0f, 0.0f, false);
            repaint();
        }
    }
}

void ChopperWaveformComponent::mouseDrag(const juce::MouseEvent& e)
{
    if (draggedSliceMarker != -1 && waveArea.getWidth() > 0)
    {
        const auto& buffer = audioEngine.getSlicer().getAudioBuffer();
        int numSamples = buffer.getNumSamples();
        if (numSamples > 0)
        {
            float relX = juce::jlimit(0.0f, 1.0f, (e.x - waveArea.getX()) / static_cast<float>(waveArea.getWidth()));
            int newSample = static_cast<int>(relX * numSamples);

            audioEngine.getSlicer().moveSliceMarker(draggedSliceMarker, newSample);
            repaint();
        }
    }
}

void ChopperWaveformComponent::mouseUp(const juce::MouseEvent& /*e*/)
{
    if (draggedSliceMarker != -1)
    {
        draggedSliceMarker = -1;
        updatePadLabels();
        repaint();
    }
}

void ChopperWaveformComponent::mouseDoubleClick(const juce::MouseEvent& e)
{
    if (!waveArea.contains(e.getPosition()))
        return;

    const auto& buffer = audioEngine.getSlicer().getAudioBuffer();
    int numSamples = buffer.getNumSamples();
    if (numSamples > 0 && waveArea.getWidth() > 0)
    {
        float relX = juce::jlimit(0.0f, 1.0f, (e.x - waveArea.getX()) / static_cast<float>(waveArea.getWidth()));
        int newSample = static_cast<int>(relX * numSamples);

        audioEngine.getSlicer().addSliceMarker(newSample);
        updatePadLabels();
        repaint();
    }
}

void ChopperWaveformComponent::updatePadLabels()
{
    int numSlices = audioEngine.getSlicer().getNumSlices();
    for (int i = 0; i < 16; ++i)
    {
        if (i < numSlices)
        {
            auto slice = audioEngine.getSlicer().getSlice(i);
            double ms = ((slice.endSample - slice.startSample) / 44100.0) * 1000.0;
            slicePads[i]->setButtonText("PAD " + juce::String(i + 1) + " (" + juce::String(juce::roundToInt(ms)) + "m)");
            slicePads[i]->setEnabled(true);
        }
        else
        {
            slicePads[i]->setButtonText("PAD " + juce::String(i + 1));
            slicePads[i]->setEnabled(false);
        }
    }
}

void ChopperWaveformComponent::updateAllControls()
{
    auto& slicer = audioEngine.getSlicer();
    fileNameLabel.setText(slicer.getLoadedFileName(slicer.getActiveSlot()), juce::dontSendNotification);
    volumeKnob->setValue(slicer.getVolume(), juce::dontSendNotification);
    cutoffKnob->setValue(slicer.getCutoff(), juce::dontSendNotification);
    reverbKnob->setValue(slicer.getReverbSend(), juce::dontSendNotification);
    updateSlotButtonStyles();
    updatePadLabels();
    repaint();
}

void ChopperWaveformComponent::openLoopChooser()
{
    fileChooser = std::make_unique<juce::FileChooser>(
        "Load Audio to Slot " + juce::String(audioEngine.getSlicer().getActiveSlot() + 1),
        juce::File::getSpecialLocation(juce::File::userHomeDirectory),
        "*.wav;*.aif;*.aiff;*.mp3;*.flac"
    );

    auto flags = juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles;

    fileChooser->launchAsync(flags, [this](const juce::FileChooser& fc) {
        auto file = fc.getResult();
        if (file.existsAsFile())
        {
            int slot = audioEngine.getSlicer().getActiveSlot();
            if (audioEngine.getSlicer().loadAudioFileToSlot(slot, file, audioEngine.getFormatManager()))
            {
                fileNameLabel.setText(file.getFileName(), juce::dontSendNotification);
                updatePadLabels();
                repaint();
            }
        }
    });
}

bool ChopperWaveformComponent::isInterestedInFileDrag(const juce::StringArray& files)
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

void ChopperWaveformComponent::filesDropped(const juce::StringArray& files, int /*x*/, int /*y*/)
{
    if (files.size() > 0)
    {
        juce::File file(files[0]);
        int slot = audioEngine.getSlicer().getActiveSlot();
        if (audioEngine.getSlicer().loadAudioFileToSlot(slot, file, audioEngine.getFormatManager()))
        {
            fileNameLabel.setText(file.getFileName(), juce::dontSendNotification);
            updatePadLabels();
            repaint();
        }
    }
}

} // namespace BreakStepUI
