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

    // Get accent color from slider
    juce::Colour accent = slider.findColour(juce::Slider::rotarySliderFillColourId);
    if (accent == juce::Colours::transparentBlack) accent = juce::Colour(mst::theme::cEqGain);

    // Get touch state/glow alpha
    float glowAlpha = (float)slider.getProperties().getWithDefault("glowAlpha", 0.0f);
    bool isActive = (glowAlpha > 0.01f);

    // Knob body — radial gradient (v2: circle at 35% 30%, #2A2A3A → #1A1A24 55% → #0A0A12 100%)
    const float bodyR = radius - 6.0f;
    juce::ColourGradient body(
        juce::Colour(0xFF2A2A3A), centre.x - bodyR * 0.3f, centre.y - bodyR * 0.4f,
        juce::Colour(0xFF0A0A12), centre.x + bodyR, centre.y + bodyR,
        true);
    body.addColour(0.55f, juce::Colour(0xFF1A1A24));
    g.setGradientFill(body);
    g.fillEllipse(centre.x - bodyR, centre.y - bodyR, bodyR * 2.0f, bodyR * 2.0f);

    // 1px rim and faint inner ring (v2: 1px #2C2C3C border + 1px rgba(255,255,255,0.04) inner)
    g.setColour(juce::Colour(0xFF2C2C3C));
    g.drawEllipse(centre.x - bodyR, centre.y - bodyR, bodyR * 2.0f, bodyR * 2.0f, 1.0f);
    g.setColour(juce::Colours::white.withAlpha(0.04f));
    g.drawEllipse(centre.x - bodyR + 4, centre.y - bodyR + 4, (bodyR - 4) * 2.0f, (bodyR - 4) * 2.0f, 1.0f);

    // Value arc (lit arc)
    const float arcRadius = radius - 2.0f;
    const float arcWidth = 3.0f;
    const float startAngle = juce::degreesToRadians(45.0f);
    const float sweepLength = juce::degreesToRadians(270.0f);
    const float endAngle = startAngle + (sweepLength * sliderPos);

    juce::Path arc;
    arc.addCentredArc(centre.x, centre.y, arcRadius, arcRadius, 0.0f, startAngle, endAngle, true);

    // Lit arc: 40% at rest, 100% on touch
    float arcAlpha = isActive ? 1.0f : 0.4f;
    g.setColour(accent.withAlpha(arcAlpha));
    g.strokePath(arc, juce::PathStrokeType(arcWidth, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    // Glow and halo only if active
    if (isActive) {
        // Outer halo
        const float haloR = radius * 1.6f;
        juce::ColourGradient halo(
            accent.withAlpha(glowAlpha * 0.6f), centre.x, centre.y,
            accent.withAlpha(0.0f), centre.x + haloR, centre.y,
            true);
        g.setGradientFill(halo);
        g.fillEllipse(bounds.expanded(16.0f));

        // Inner glow
        juce::ColourGradient innerGlow(
            accent.withAlpha(glowAlpha * 0.3f), centre.x, centre.y + radius * 0.2f,
            accent.withAlpha(0.0f), centre.x, centre.y - radius * 0.5f,
            false);
        g.setGradientFill(innerGlow);
        g.fillEllipse(centre.x - bodyR + 6, centre.y - bodyR + 6, (bodyR - 6) * 2, (bodyR - 6) * 2);

        // Center dot
        g.setColour(accent.withAlpha(glowAlpha));
        g.fillEllipse(centre.x - 2.0f, centre.y - 2.0f, 4.0f, 4.0f);
    }
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
