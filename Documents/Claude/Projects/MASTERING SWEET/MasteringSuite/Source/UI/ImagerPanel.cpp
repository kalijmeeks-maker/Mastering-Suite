#include "ImagerPanel.h"
#include "../PluginProcessor.h"

ImagerPanel::ImagerPanel(MasteringSuiteProcessor& proc, NeonLookAndFeel& laf)
    : processor(proc), lookAndFeel(laf) {

    auto createKnob = [&](std::unique_ptr<juce::Slider>& k, std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>& a, juce::String paramID, juce::Colour color) {
        k = std::make_unique<juce::Slider>(juce::Slider::RotaryVerticalDrag, juce::Slider::TextBoxBelow);
        k->setLookAndFeel(&lookAndFeel);
        k->setColour(juce::Slider::rotarySliderFillColourId, color);
        a = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(processor.getAPVTS(), paramID, *k);
        if (auto* param = processor.getAPVTS().getParameter(paramID))
            k->setTooltip(param->getName(64));
        addAndMakeVisible(*k);
    };

    createKnob(widthK, widthA, "imgWidth", juce::Colour(mst::theme::tabImg));
    createKnob(panK, panA, "imgPan", juce::Colour(mst::theme::tabImg));
}

void ImagerPanel::paint(juce::Graphics& g) {
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
    g.drawText(juce::String::fromUTF8("IMAGER \u00B7 STEREO FIELD"), 14, 8, (int)bounds.getWidth() - 28, 14, juce::Justification::topLeft);

    // Goniometer Area
    auto visualArea = bounds.reduced(100, 40).withY(40).withHeight(bounds.getHeight() - 150);
    drawGoniometer(g, visualArea);
}

void ImagerPanel::drawGoniometer(juce::Graphics& g, juce::Rectangle<float> area) {
    // Backdrop + framing
    g.setColour(juce::Colour(mst::theme::bgBase));
    g.fillEllipse(area);
    g.setColour(juce::Colour(mst::theme::border).withAlpha(0.5f));
    g.drawEllipse(area, 1.0f);
    g.drawVerticalLine((int)area.getCentreX(), area.getY(), area.getBottom());
    g.drawHorizontalLine((int)area.getCentreY(), area.getX(), area.getRight());

    std::array<float, 1024> left, right;
    processor.getGoniometerSamples(left.data(), right.data());

    // Auto-scale: normalize by the block's peak so a -20 dBFS signal still fills
    // the field instead of collapsing into a tiny center smudge.
    // Floor at 0.1 (-20 dBFS) so silence doesn't blow up to full-scale.
    float peak = 0.0f;
    for (int i = 0; i < 1024; ++i) {
        peak = std::max(peak, std::max(std::abs(left[i]), std::abs(right[i])));
    }
    const float norm = 1.0f / std::max(0.1f, peak);

    // Field radius: 45% of min(width, height) — keep dots inside the ellipse.
    const float radius = juce::jmin(area.getWidth(), area.getHeight()) * 0.45f;
    const float cx = area.getCentreX();
    const float cy = area.getCentreY();
    const juce::Colour scatter = juce::Colour(mst::theme::tabImg);

    // Connected-line phosphor trace (every sample) + dots at samples for grain.
    juce::Path trace;
    bool started = false;
    for (int i = 0; i < 1024; ++i) {
        float l = left[i] * norm;
        float r = right[i] * norm;
        // Standard M/S basis: vertical = (L+R)/√2 (mid), horizontal = (L-R)/√2 (side)
        float sx = (l - r) * 0.7071f * radius;
        float sy = (l + r) * 0.7071f * radius;
        float dx = cx + sx;
        float dy = cy - sy;
        if (!std::isfinite(dx) || !std::isfinite(dy)) continue;
        if (!started) { trace.startNewSubPath(dx, dy); started = true; }
        else            trace.lineTo(dx, dy);
    }
    g.setColour(scatter.withAlpha(0.45f));
    g.strokePath(trace, juce::PathStrokeType(0.8f));

    // Bright dots every 2nd sample on top of the trace for the classic look.
    g.setColour(scatter.withAlpha(0.85f));
    for (int i = 0; i < 1024; i += 2) {
        float l = left[i] * norm;
        float r = right[i] * norm;
        float sx = (l - r) * 0.7071f * radius;
        float sy = (l + r) * 0.7071f * radius;
        float dx = cx + sx;
        float dy = cy - sy;
        if (!std::isfinite(dx) || !std::isfinite(dy)) continue;
        g.fillEllipse(dx - 1.0f, dy - 1.0f, 2.0f, 2.0f);
    }
}

void ImagerPanel::resized() {
    auto bounds = getLocalBounds();
    auto knobArea = bounds.reduced(20, 20);
    knobArea.removeFromTop(bounds.getHeight() * 0.7f);

    int knobW = 80;
    widthK->setBounds(bounds.getWidth() / 2 - knobW - 10, knobArea.getY(), knobW, knobW);
    panK->setBounds(bounds.getWidth() / 2 + 10, knobArea.getY(), knobW, knobW);
}

void ImagerPanel::refresh() {
    repaint();
}
