#include "EqPanel.h"
#include "../PluginProcessor.h"

EqPanel::EqPanel(MasteringSuiteProcessor& proc, NeonLookAndFeel& laf)
    : processor(proc), lookAndFeel(laf) {

    gainKnob = std::make_unique<juce::Slider>(juce::Slider::RotaryVerticalDrag, juce::Slider::TextBoxBelow);
    freqKnob = std::make_unique<juce::Slider>(juce::Slider::RotaryVerticalDrag, juce::Slider::TextBoxBelow);
    qKnob = std::make_unique<juce::Slider>(juce::Slider::RotaryVerticalDrag, juce::Slider::TextBoxBelow);

    // Set colors and attach to APVTS
    gainKnob->setColour(juce::Slider::rotarySliderFillColourId, juce::Colour(mst::theme::cEqGain));
    gainKnob->setLookAndFeel(&lookAndFeel);
    gainKnob->textFromValueFunction = [](double v) { return juce::String(v, 1) + " dB"; };
    gainAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        processor.getAPVTS(), "eq_gain", *gainKnob);
    addAndMakeVisible(*gainKnob);

    freqKnob->setColour(juce::Slider::rotarySliderFillColourId, juce::Colour(mst::theme::cEqFreq));
    freqKnob->setLookAndFeel(&lookAndFeel);
    freqKnob->textFromValueFunction = [](double v) {
        if (v >= 1000) return juce::String(v / 1000.0, 1) + " kHz";
        return juce::String((int)v) + " Hz";
    };
    freqAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        processor.getAPVTS(), "eq_freq", *freqKnob);
    addAndMakeVisible(*freqKnob);

    qKnob->setColour(juce::Slider::rotarySliderFillColourId, juce::Colour(mst::theme::cEqQ));
    qKnob->setLookAndFeel(&lookAndFeel);
    qKnob->textFromValueFunction = [](double v) { return juce::String(v, 1); };
    qAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        processor.getAPVTS(), "eq_q", *qKnob);
    addAndMakeVisible(*qKnob);
}

void EqPanel::paint(juce::Graphics& g) {
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
    g.drawText(juce::String(juce::CharPointer_UTF8(u8"EQ · 3-BAND PARAMETRIC")), 14, 8, bounds.getWidth() - 28, 14, juce::Justification::topLeft);

    g.setFont(juce::Font(8.0f));
    g.setColour(juce::Colour(mst::theme::textLow));
    g.drawText("PRE-LIMITER", 14, 20, bounds.getWidth() - 28, 10, juce::Justification::topLeft);

    // Labels above knobs
    g.setFont(juce::Font(10.0f).boldened());
    g.setColour(juce::Colour(mst::theme::textMid));

    auto labelY = 35;
    auto colWidth = bounds.getWidth() / 3;
    g.drawText("GAIN", 10, labelY, colWidth - 4, 12, juce::Justification::centredTop);
    g.drawText("FREQ", 10 + colWidth, labelY, colWidth - 4, 12, juce::Justification::centredTop);
    g.drawText("Q", 10 + colWidth * 2, labelY, colWidth - 4, 12, juce::Justification::centredTop);
}

void EqPanel::resized() {
    auto bounds = getLocalBounds();
    auto knobArea = bounds.reduced(10, 40);

    int knobW = knobArea.getWidth() / 3 - 4;
    int knobH = knobArea.getHeight() * 0.6f;

    gainKnob->setBounds(10, knobArea.getY(), knobW, knobH);
    freqKnob->setBounds(knobArea.getWidth() / 3 + 6, knobArea.getY(), knobW, knobH);
    qKnob->setBounds(knobArea.getWidth() * 2 / 3 + 2, knobArea.getY(), knobW, knobH);
}
