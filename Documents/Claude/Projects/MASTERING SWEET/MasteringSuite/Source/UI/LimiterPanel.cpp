#include "LimiterPanel.h"
#include "../PluginProcessor.h"

LimiterPanel::LimiterPanel(MasteringSuiteProcessor& proc, NeonLookAndFeel& laf)
    : processor(proc), lookAndFeel(laf) {

    auto createKnob = [&](std::unique_ptr<juce::Slider>& k, std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>& a, juce::String paramID, juce::Colour color) {
        k = std::make_unique<juce::Slider>(juce::Slider::RotaryVerticalDrag, juce::Slider::NoTextBox);
        k->setLookAndFeel(&lookAndFeel);
        k->setColour(juce::Slider::rotarySliderFillColourId, color);
        a = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(processor.getAPVTS(), paramID, *k);
        if (auto* param = processor.getAPVTS().getParameter(paramID))
            k->setTooltip(param->getName(64));
        addAndMakeVisible(*k);
    };

    createKnob(threshK, threshA, "limThreshold", juce::Colour(mst::theme::tabLim));
    createKnob(releaseK, releaseA, "limRelease", juce::Colour(mst::theme::tabLim));
    createKnob(ceilingK, ceilingA, "limCeiling", juce::Colour(mst::theme::tabLim));
    createKnob(makeupK, makeupA, "limMakeup", juce::Colour(mst::theme::tabLim));

    styleC = std::make_unique<juce::ComboBox>();
    styleC->addItemList(juce::StringArray("Transparent", "Punchy", "Warm"), 1);
    styleA = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(processor.getAPVTS(), "limStyle", *styleC);
    addAndMakeVisible(*styleC);

    truePeakB = std::make_unique<juce::ToggleButton>("TRUE PEAK");
    truePeakA = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(processor.getAPVTS(), "limTruePeak", *truePeakB);
    addAndMakeVisible(*truePeakB);
}

void LimiterPanel::paint(juce::Graphics& g) {
    auto bounds = getLocalBounds().toFloat();

    // Background
    juce::ColourGradient bg(
        juce::Colour(mst::theme::panelTop), 0.0f, 0.0f,
        juce::Colour(mst::theme::panelInner), 0.0f, bounds.getHeight(),
        false);
    g.setGradientFill(bg);
    g.fillRoundedRectangle(bounds, 8.0f);
    g.setColour(juce::Colour(mst::theme::border).withAlpha(0.3f));
    g.drawRoundedRectangle(bounds, 8.0f, 1.0f);

    // Header
    g.setFont(juce::Font(11.0f).boldened());
    g.setColour(juce::Colour(mst::theme::textHigh));
    g.drawText(juce::String::fromUTF8("LIMITER \u00B7 TRUE PEAK"), 14, 8, (int)bounds.getWidth() - 28, 14, juce::Justification::topLeft);

    // GR meter on the right
    auto meterX = bounds.getWidth() * 0.85f;
    auto meterW = bounds.getWidth() - meterX - 14.0f;
    auto meterH = bounds.getHeight() - 60.0f;
    auto meterY = 40.0f;

    g.setColour(juce::Colour(mst::theme::panelInner));
    g.fillRoundedRectangle(meterX, meterY, meterW, meterH, 4.0f);

    float currentGainReduction = processor.getLimiter().getCurrentGainReduction();
    const int segmentCount = 16;
    const float segmentH = (meterH - 4.0f) / segmentCount;
    const float normalized = juce::jmin(1.0f, std::abs(currentGainReduction) / 24.0f);
    const int litSegments = (int)(normalized * segmentCount);

    // Draw inactive segments
    g.setColour(juce::Colour(mst::theme::panelInner).brighter(0.2f));
    for (int i = 0; i < segmentCount; ++i) {
        float y = meterY + 2 + i * segmentH;
        g.fillRoundedRectangle(meterX + 2, y, meterW - 4, segmentH - 1.5f, 1.0f);
    }

    g.setColour(juce::Colour(mst::theme::tabLim).withAlpha(0.8f));
    for (int i = 0; i < litSegments; ++i) {
        float y = meterY + 2 + i * segmentH;
        g.fillRoundedRectangle(meterX + 2, y, meterW - 4, segmentH - 1.5f, 1.0f);
    }

    // Knob Labels
    g.setFont(juce::Font(9.0f));
    g.setColour(juce::Colour(mst::theme::textMid));
    auto drawLabel = [&](juce::Component& k, juce::String text) {
        g.drawText(text, k.getBounds().withY(k.getBottom() - 5).withHeight(15), juce::Justification::centred);
    };

    drawLabel(*threshK, "THRESH");
    drawLabel(*ceilingK, "CEILING");
    drawLabel(*releaseK, "RELEASE");
    drawLabel(*makeupK, "MAKEUP");
}

void LimiterPanel::resized() {
    auto bounds = getLocalBounds();
    auto knobArea = bounds.reduced(20, 40);
    knobArea.removeFromRight(bounds.getWidth() * 0.2f);

    int knobW = knobArea.getWidth() / 4;
    threshK->setBounds(knobArea.removeFromLeft(knobW).reduced(10));
    ceilingK->setBounds(knobArea.removeFromLeft(knobW).reduced(10));
    releaseK->setBounds(knobArea.removeFromLeft(knobW).reduced(10));
    makeupK->setBounds(knobArea.removeFromLeft(knobW).reduced(10));

    styleC->setBounds(20, bounds.getHeight() - 40, 120, 20);
    truePeakB->setBounds(160, bounds.getHeight() - 40, 100, 20);
}

void LimiterPanel::refresh() {
    repaint();
}
