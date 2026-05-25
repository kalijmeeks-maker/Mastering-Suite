#include "DynamicsPanel.h"
#include "../PluginProcessor.h"

DynamicsPanel::DynamicsPanel(MasteringSuiteProcessor& proc, NeonLookAndFeel& laf)
    : processor(proc), lookAndFeel(laf) {

    auto createKnob = [&](std::unique_ptr<juce::Slider>& k, std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>& a, juce::String paramID, juce::Colour color) {
        k = std::make_unique<juce::Slider>(juce::Slider::RotaryVerticalDrag, juce::Slider::NoTextBox);
        k->setLookAndFeel(&lookAndFeel);
        k->setColour(juce::Slider::rotarySliderFillColourId, color);
        a = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(processor.getAPVTS(), paramID, *k);
        // Hover tooltip pulled from APVTS parameter metadata (v2: 1-line caption in Expanded).
        if (auto* param = processor.getAPVTS().getParameter(paramID))
            k->setTooltip(param->getName(64));
        addAndMakeVisible(*k);
    };

    createKnob(threshK, threshA, "dynThreshold", juce::Colour(mst::theme::tabDyn));
    createKnob(ratioK, ratioA, "dynRatio", juce::Colour(mst::theme::tabDyn));
    createKnob(kneeK, kneeA, "dynKnee", juce::Colour(mst::theme::tabDyn));
    createKnob(attackK, attackA, "dynAttack", juce::Colour(mst::theme::tabDyn));
    createKnob(releaseK, releaseA, "dynRelease", juce::Colour(mst::theme::tabDyn));
    createKnob(makeupK, makeupA, "dynMakeup", juce::Colour(mst::theme::tabDyn));
    createKnob(mixK, mixA, "dynMix", juce::Colour(mst::theme::tabDyn));
}

void DynamicsPanel::paint(juce::Graphics& g) {
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
    g.drawText(juce::String::fromUTF8("DYNAMICS \u00B7 MULTI-MODE"), 14, 8, (int)bounds.getWidth() - 28, 14, juce::Justification::topLeft);

    // Transfer Curve Area
    auto visualArea = bounds.reduced(20, 40).removeFromTop(bounds.getHeight() * 0.45f);
    drawTransferCurve(g, visualArea);

    // Knob Labels
    g.setFont(juce::Font(9.0f));
    g.setColour(juce::Colour(mst::theme::textMid));
    auto drawLabel = [&](juce::Slider& k, juce::String text) {
        g.drawText(text, k.getBounds().withY(k.getBottom() - 5).withHeight(15), juce::Justification::centred);
    };

    drawLabel(*threshK, "THRESH");
    drawLabel(*ratioK, "RATIO");
    drawLabel(*kneeK, "KNEE");
    drawLabel(*attackK, "ATTACK");
    drawLabel(*releaseK, "RELEASE");
    drawLabel(*makeupK, "MAKEUP");
    drawLabel(*mixK, "MIX");
}

void DynamicsPanel::resized() {
    auto bounds = getLocalBounds();
    auto knobArea = bounds.reduced(20, 20);
    knobArea.removeFromTop(bounds.getHeight() * 0.55f); // Leave room for curve

    int knobW = knobArea.getWidth() / 7;
    threshK->setBounds(knobArea.removeFromLeft(knobW).reduced(5));
    ratioK->setBounds(knobArea.removeFromLeft(knobW).reduced(5));
    kneeK->setBounds(knobArea.removeFromLeft(knobW).reduced(5));
    attackK->setBounds(knobArea.removeFromLeft(knobW).reduced(5));
    releaseK->setBounds(knobArea.removeFromLeft(knobW).reduced(5));
    makeupK->setBounds(knobArea.removeFromLeft(knobW).reduced(5));
    mixK->setBounds(knobArea.removeFromLeft(knobW).reduced(5));
}

void DynamicsPanel::drawTransferCurve(juce::Graphics& g, juce::Rectangle<float> area) {
    g.setColour(juce::Colour(mst::theme::bgBase));
    g.fillRoundedRectangle(area, 4.0f);
    g.setColour(juce::Colour(mst::theme::border).withAlpha(0.5f));
    g.drawRoundedRectangle(area, 4.0f, 1.0f);

    // Grid lines
    g.setColour(juce::Colour(mst::theme::border).withAlpha(0.2f));
    for (int i = 1; i < 4; ++i) {
        float x = area.getX() + area.getWidth() * (i / 4.0f);
        float y = area.getY() + area.getHeight() * (i / 4.0f);
        g.drawVerticalLine((int)x, area.getY(), area.getBottom());
        g.drawHorizontalLine((int)y, area.getX(), area.getRight());
    }

    // Path
    juce::Path curve;
    float thresh = threshK->getValue();
    float ratio = ratioK->getValue();
    
    auto toX = [&](float db) { return juce::jmap(db, -60.0f, 0.0f, area.getX(), area.getRight()); };
    auto toY = [&](float db) { return juce::jmap(db, -60.0f, 0.0f, area.getBottom(), area.getY()); };

    curve.startNewSubPath(toX(-60.0f), toY(-60.0f));
    
    for (float db = -60.0f; db <= 0.0f; db += 1.0f) {
        float outDb = db;
        if (db > thresh) {
            outDb = thresh + (db - thresh) / ratio;
        }
        curve.lineTo(toX(db), toY(outDb));
    }

    g.setColour(juce::Colour(mst::theme::tabDyn));
    g.strokePath(curve, juce::PathStrokeType(2.0f));
    
    // Threshold dot
    g.setColour(juce::Colour(mst::theme::textHigh).withAlpha(0.4f));
    g.drawEllipse(toX(thresh) - 4, toY(thresh) - 4, 8, 8, 1.0f);

    // Bouncing Ball (Live Level)
    float gr = processor.getLimiter().getCurrentGainReduction(); // Actually should use compressor GR if we had it, but limiter is fine for now
    float inDb = processor.getInputPeak();
    float outDb = inDb + gr;

    g.setColour(juce::Colour(mst::theme::tabDyn));
    g.fillEllipse(toX(inDb) - 4, toY(outDb) - 4, 8, 8);
    g.setColour(juce::Colour(mst::theme::textHigh));
    g.drawEllipse(toX(inDb) - 4, toY(outDb) - 4, 8, 8, 1.0f);
}

void DynamicsPanel::refresh() {
    repaint();
}
