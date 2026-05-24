#include "NeonLookAndFeel.h"

NeonLookAndFeel::NeonLookAndFeel() {
    setColour(juce::Slider::rotarySliderFillColourId, juce::Colours::transparentBlack);
}

void NeonLookAndFeel::drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                                        float sliderPos,
                                        float rotaryStartAngle, float rotaryEndAngle,
                                        juce::Slider& slider) {
    auto bounds = juce::Rectangle<float>(x, y, width, height);
    auto centre = bounds.getCentre();
    auto radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) * 0.4f;

    // Get accent color from slider or use cyan default
    juce::Colour accent = slider.findColour(juce::Slider::rotarySliderFillColourId);
    if (accent == juce::Colours::transparentBlack) {
        accent = juce::Colour(mst::theme::cEqGain);
    }

    // Hover detection for stronger glow
    const bool isHovered = slider.isMouseOverOrDragging();
    const float haloAlpha = isHovered ? 0.85f : 0.55f;
    const float haloMul = isHovered ? 2.2f : 1.6f;

    // Outer halo (primary glow, v2 stronger)
    const float haloR = radius * haloMul;
    juce::ColourGradient halo(
        accent.withAlpha(haloAlpha), centre.x, centre.y,
        accent.withAlpha(0.0f), centre.x + haloR, centre.y,
        true);
    halo.addColour(0.45, accent.withAlpha(0.18f));
    g.setGradientFill(halo);
    g.fillEllipse(bounds.expanded(16.0f));

    // Knob body (dark circle with subtle radial gradient)
    const float bodyR = radius - 6.0f;
    juce::ColourGradient body(
        juce::Colour(0xFF1A1A23), centre.x, centre.y - bodyR * 0.3f,
        juce::Colour(0xFF0D0D14), centre.x, centre.y + bodyR * 0.3f,
        false);
    g.setGradientFill(body);
    g.fillEllipse(centre.x - bodyR, centre.y - bodyR, bodyR * 2.0f, bodyR * 2.0f);

    // Inner glow (subtle top-to-bottom bloom)
    juce::ColourGradient innerGlow(
        accent.withAlpha(0.18f), centre.x, centre.y + radius * 0.2f,
        accent.withAlpha(0.0f), centre.x, centre.y - radius * 0.5f,
        false);
    g.setGradientFill(innerGlow);
    g.fillEllipse(centre.x - bodyR + 6, centre.y - bodyR + 6,
                  (bodyR - 6) * 2, (bodyR - 6) * 2);

    // Value arc (270° from 45° to 315°)
    const float arcRadius = radius - 2.0f;
    const float arcWidth = 3.0f;
    const float startAngle = juce::degreesToRadians(45.0f);
    const float sweepLength = juce::degreesToRadians(270.0f);
    const float endAngle = startAngle + (sweepLength * sliderPos);

    juce::Path arc;
    arc.addCentredArc(centre.x, centre.y, arcRadius, arcRadius, 0.0f,
                      startAngle, endAngle, true);

    g.setColour(accent.withAlpha(0.9f));
    g.strokePath(arc, juce::PathStrokeType(arcWidth, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    // Bezel stroke
    g.setColour(accent.withAlpha(0.4f));
    g.drawEllipse(centre.x - bodyR, centre.y - bodyR, bodyR * 2.0f, bodyR * 2.0f, 0.8f);

    // Center indicator dot
    g.setColour(accent.withAlpha(0.7f));
    g.fillEllipse(centre.x - 2.0f, centre.y - 2.0f, 4.0f, 4.0f);
}

void NeonLookAndFeel::drawButtonBackground(juce::Graphics& g, juce::Button& button,
                                            const juce::Colour& backgroundColour,
                                            bool isMouseOverButton, bool isButtonDown) {
    auto bounds = button.getLocalBounds().toFloat();
    g.setColour(backgroundColour);
    g.fillRoundedRectangle(bounds, 4.0f);
}

void NeonLookAndFeel::drawLinearSlider(juce::Graphics& g, int x, int y, int width, int height,
                                        float sliderPos, float minSliderPos, float maxSliderPos,
                                        const juce::Slider::SliderStyle style,
                                        juce::Slider& slider) {
    auto bounds = juce::Rectangle<int>(x, y, width, height).toFloat();
    g.setColour(juce::Colour(0xFF23232D));
    g.fillRoundedRectangle(bounds, 2.0f);
}

void NeonLookAndFeel::drawLabel(juce::Graphics& g, juce::Label& label) {
    g.fillAll(label.findColour(juce::Label::backgroundColourId));

    if (!label.isBeingEdited()) {
        auto alpha = label.isEnabled() ? 1.0f : 0.5f;
        g.setColour(label.findColour(juce::Label::textColourId).withAlpha(alpha));
        g.setFont(label.getFont());
        g.drawFittedText(label.getText(), label.getLocalBounds(),
                        label.getJustificationType(), 1);
    }
}
