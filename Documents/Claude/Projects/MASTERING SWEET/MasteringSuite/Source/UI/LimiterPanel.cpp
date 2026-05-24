#include "LimiterPanel.h"
#include "../PluginProcessor.h"

LimiterPanel::LimiterPanel(MasteringSuiteProcessor& proc, NeonLookAndFeel& laf)
    : processor(proc), lookAndFeel(laf) {

    threshKnob = std::make_unique<juce::Slider>(juce::Slider::RotaryVerticalDrag, juce::Slider::TextBoxBelow);
    releaseKnob = std::make_unique<juce::Slider>(juce::Slider::RotaryVerticalDrag, juce::Slider::TextBoxBelow);
    makeupKnob = std::make_unique<juce::Slider>(juce::Slider::RotaryVerticalDrag, juce::Slider::TextBoxBelow);

    // Threshold - Orange
    threshKnob->setColour(juce::Slider::rotarySliderFillColourId, juce::Colour(mst::theme::cLimThresh));
    threshKnob->setLookAndFeel(&lookAndFeel);
    threshKnob->textFromValueFunction = [](double v) { return juce::String(v, 1) + " dB"; };
    threshAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        processor.getAPVTS(), "lim_thresh", *threshKnob);
    addAndMakeVisible(*threshKnob);

    // Release - Violet
    releaseKnob->setColour(juce::Slider::rotarySliderFillColourId, juce::Colour(mst::theme::cLimRelease));
    releaseKnob->setLookAndFeel(&lookAndFeel);
    releaseKnob->textFromValueFunction = [](double v) { return juce::String((int)v) + " ms"; };
    releaseAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        processor.getAPVTS(), "lim_release", *releaseKnob);
    addAndMakeVisible(*releaseKnob);

    // Makeup - Mint
    makeupKnob->setColour(juce::Slider::rotarySliderFillColourId, juce::Colour(mst::theme::cLimMakeup));
    makeupKnob->setLookAndFeel(&lookAndFeel);
    makeupKnob->textFromValueFunction = [](double v) { return juce::String(v, 1) + " dB"; };
    makeupAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        processor.getAPVTS(), "lim_makeup", *makeupKnob);
    addAndMakeVisible(*makeupKnob);
}

void LimiterPanel::paint(juce::Graphics& g) {
    auto bounds = getLocalBounds().toFloat();

    // Background gradient
    juce::ColourGradient bg(
        juce::Colour(mst::theme::panelTop), 0.0f, 0.0f,
        juce::Colour(mst::theme::panelInner), 0.0f, bounds.getHeight(),
        false);
    g.setGradientFill(bg);
    g.fillRoundedRectangle(bounds, 8.0f);

    // Border
    g.setColour(juce::Colour(mst::theme::border).withAlpha(0.3f));
    g.drawRoundedRectangle(bounds, 8.0f, 1.0f);

    // Header
    g.setFont(juce::Font(11.0f).boldened());
    g.setColour(juce::Colour(mst::theme::textHigh));
    g.drawText(juce::String(juce::CharPointer_UTF8(u8"LIMITER · TRUE PEAK")), 14, 8, bounds.getWidth() * 0.6f, 14, juce::Justification::topLeft);

    g.setFont(juce::Font(8.0f));
    g.setColour(juce::Colour(mst::theme::textLow));
    g.drawText("CEILING −1.0 dBTP", 14, 20, bounds.getWidth() * 0.6f, 10, juce::Justification::topLeft);

    // Knob labels above
    g.setFont(juce::Font(10.0f).boldened());
    g.setColour(juce::Colour(mst::theme::textMid));

    auto labelY = 35;
    auto colWidth = bounds.getWidth() * 0.33f;
    g.drawText("THRESHOLD", 10, labelY, colWidth - 4, 12, juce::Justification::centredTop);
    g.drawText("RELEASE", 10 + colWidth, labelY, colWidth - 4, 12, juce::Justification::centredTop);
    g.drawText("MAKEUP", 10 + colWidth * 2, labelY, colWidth - 4, 12, juce::Justification::centredTop);

    // GR meter on the right
    auto meterX = bounds.getWidth() * 0.75f;
    auto meterW = bounds.getWidth() - meterX - 10.0f;
    auto meterH = bounds.getHeight() - 50.0f;
    auto meterY = bounds.getY() + 40.0f;

    // GR meter background
    g.setColour(juce::Colour(mst::theme::panelInner));
    g.fillRoundedRectangle(meterX, meterY, meterW, meterH, 4.0f);

    // Draw GR segments (0 to 40 dB reduction)
    const int segmentCount = 12;
    const float segmentH = (meterH - 4.0f) / segmentCount;
    const float normalized = juce::jmin(1.0f, currentGainReduction / 40.0f);
    const int litSegments = (int)(normalized * segmentCount);

    g.setColour(juce::Colour(mst::theme::cLimThresh).withAlpha(0.8f));
    for (int i = 0; i < litSegments; ++i) {
        float y = meterY + meterH - (i + 1) * segmentH;
        g.fillRoundedRectangle(meterX + 2, y, meterW - 4, segmentH - 1.5f, 1.0f);
    }

    // GR label and value
    g.setFont(juce::Font(9.0f).boldened());
    g.setColour(juce::Colour(mst::theme::textMid));
    g.drawText("GR", meterX + 2, (int)(meterY + meterH + 2), (int)meterW - 4, 12, juce::Justification::centredTop);

    g.setFont(juce::Font(8.0f));
    g.drawText(juce::String(currentGainReduction, 1) + " dB", meterX + 2, (int)(meterY + meterH + 14), (int)meterW - 4, 10, juce::Justification::centredTop);
}

void LimiterPanel::resized() {
    auto bounds = getLocalBounds();
    auto knobArea = bounds.reduced(10, 40);
    int knobW = (int)(knobArea.getWidth() * 0.25f);
    int knobH = knobArea.getHeight() * 0.6f;

    threshKnob->setBounds(10, knobArea.getY(), knobW, knobH);
    releaseKnob->setBounds(10 + knobW + 5, knobArea.getY(), knobW, knobH);
    makeupKnob->setBounds(10 + (knobW + 5) * 2, knobArea.getY(), knobW, knobH);
}

void LimiterPanel::refresh() {
    repaint();
}
