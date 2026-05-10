#include "NeonLookAndFeel.h"

NeonLookAndFeel::NeonLookAndFeel() {
    setColour(juce::Slider::rotarySliderFillColourId, defaultAccent);
    setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colour(Theme::Color::LINE_2));
    setColour(juce::TextButton::buttonColourId, juce::Colour(Theme::Color::BG_2));
    setColour(juce::TextButton::textColourOffId, juce::Colour(Theme::Color::TEXT));
    setColour(juce::TextButton::textColourOnId, juce::Colour(Theme::Color::ACCENT));
    setColour(juce::Label::textColourId, juce::Colour(Theme::Color::TEXT));
}

void NeonLookAndFeel::drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                                        float sliderPos,
                                        float rotaryStartAngle, float rotaryEndAngle,
                                        juce::Slider& slider) {
    auto radius = (float)juce::jmin(width / 2, height / 2) - 2.0f;
    auto centreX = (float)x + (float)width * 0.5f;
    auto centreY = (float)y + (float)height * 0.5f;
    auto rx = centreX - radius;
    auto ry = centreY - radius;
    auto rw = radius * 2.0f;

    auto arcWidth = Theme::Knob::ARC_WIDTH;

    // Get the accent color (can be customized per slider via colour ID)
    auto colour = slider.isEnabled() ? getAccentColourForSlider(slider)
                                     : juce::Colour(Theme::Color::TEXT_DIM);

    // ─────────────────────────────── Background Arc (full range, dim) ────────────
    g.setColour(juce::Colour(Theme::Color::LINE_2));
    g.drawEllipse(rx, ry, rw, rw, arcWidth);

    // ─────────────────────────────── Foreground Arc (lit, up to position) ────────
    auto sliderAngleRadians = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);

    juce::Path arc;
    arc.addCentredArc(centreX, centreY, radius, radius, 0.0f,
                      rotaryStartAngle, sliderAngleRadians, true);

    g.setColour(colour);
    g.strokePath(arc, juce::PathStrokeType(arcWidth, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    // ─────────────────────────────── Glow Effect (outer halo) ────────────────────
    g.setColour(colour.withAlpha(0.25f));
    g.strokePath(arc, juce::PathStrokeType(arcWidth * Theme::Knob::GLOW_SCALE,
                                           juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    // ─────────────────────────────── Central Circle (gradient) ───────────────────
    juce::ColourGradient grad(juce::Colour(Theme::Color::BG_2), centreX, centreY - radius * 0.4f,
                              juce::Colour(Theme::Color::BG_1), centreX, centreY + radius * 0.4f, false);
    g.setGradientFill(grad);
    g.fillEllipse(centreX - radius + arcWidth, centreY - radius + arcWidth,
                  rw - arcWidth * 2.0f, rw - arcWidth * 2.0f);

    // ─────────────────────────────── Border of Central Circle ───────────────────
    g.setColour(juce::Colour(Theme::Color::LINE_2));
    g.drawEllipse(centreX - radius + arcWidth, centreY - radius + arcWidth,
                  rw - arcWidth * 2.0f, rw - arcWidth * 2.0f, 1.0f);

    // Inner highlight (subtle top reflection)
    g.setColour(juce::Colour(Theme::Color::LINE_2).withAlpha(0.3f));
    g.drawEllipse(centreX - radius + arcWidth, centreY - radius + arcWidth,
                  rw - arcWidth * 2.0f, rw - arcWidth * 2.0f, 1.0f);

    // ─────────────────────────────── Indicator Dot ──────────────────────────────
    // Position: inner radius along current angle, from top (90°)
    auto innerRadius = radius - arcWidth - 2.0f;
    auto angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
    auto dotX = centreX + innerRadius * std::cos(angle - juce::MathConstants<float>::halfPi);
    auto dotY = centreY + innerRadius * std::sin(angle - juce::MathConstants<float>::halfPi);

    // Glow around indicator
    g.setColour(colour.withAlpha(0.2f));
    g.fillEllipse(dotX - 3.5f, dotY - 3.5f, 7.0f, 7.0f);

    // Indicator dot itself
    g.setColour(colour);
    g.fillEllipse(dotX - Theme::Knob::INDICATOR_SIZE / 2.0f, dotY - Theme::Knob::INDICATOR_SIZE / 2.0f,
                  Theme::Knob::INDICATOR_SIZE, Theme::Knob::INDICATOR_SIZE);
}

void NeonLookAndFeel::drawButtonBackground(juce::Graphics& g, juce::Button& button,
                                            const juce::Colour& backgroundColour,
                                            bool isMouseOverButton, bool isButtonDown) {
    auto buttonArea = button.getLocalBounds().toFloat().reduced(1.0f);
    auto baseColour = backgroundColour.withMultipliedSaturation(button.hasKeyboardFocus(true) ? 1.3f : 0.9f);

    if (isButtonDown) {
        baseColour = baseColour.darker(0.2f);
    } else if (isMouseOverButton) {
        baseColour = baseColour.brighter(0.1f);
    }

    g.setColour(baseColour);
    g.fillRoundedRectangle(buttonArea, Theme::Layout::BORDER_RADIUS_BUTTON);

    g.setColour(button.findColour(juce::ComboBox::outlineColourId));
    g.drawRoundedRectangle(buttonArea, Theme::Layout::BORDER_RADIUS_BUTTON, 1.0f);
}

void NeonLookAndFeel::drawLinearSlider(juce::Graphics& g, int x, int y, int width, int height,
                                        float sliderPos,
                                        float minSliderPos, float maxSliderPos,
                                        const juce::Slider::SliderStyle style,
                                        juce::Slider& slider) {
    // For horizontal bars (level meters, gain reduction displays)
    auto colour = slider.findColour(juce::Slider::thumbColourId);
    auto trackColour = slider.findColour(juce::Slider::trackColourId);

    if (style == juce::Slider::LinearHorizontal) {
        // Track background
        g.setColour(trackColour);
        g.fillRect(x, y + height / 2 - 2, width, 4);

        // Slider thumb
        g.setColour(colour);
        g.fillRect((int)sliderPos, y + height / 2 - 3, 5, 6);
    }
}

void NeonLookAndFeel::drawLabel(juce::Graphics& g, juce::Label& label) {
    // Use default Label rendering (text on transparent background)
    g.fillAll(label.findColour(juce::Label::backgroundColourId));

    if (!label.isBeingEdited()) {
        auto alpha = label.isEnabled() ? 1.0f : 0.5f;
        g.setColour(label.findColour(juce::Label::textColourId).withAlpha(alpha));

        g.setFont(label.getFont());

        auto textArea = getLabelBorderSize(label).subtractedFrom(label.getLocalBounds());
        g.drawFittedText(label.getText(), textArea, label.getJustificationType(),
                         juce::jmax(1, (int)((float)textArea.getHeight() / label.getFont().getHeight())),
                         label.getMinimumHorizontalScale());
    } else if (label.isEnabled()) {
        g.setColour(label.findColour(juce::Label::outlineColourId));
        g.drawRect(label.getLocalBounds());
    }
}

juce::Colour NeonLookAndFeel::getAccentColourForSlider(const juce::Slider& slider) {
    // Check if slider has a custom color set via findColour()
    auto customColour = slider.findColour(juce::Slider::rotarySliderFillColourId);

    // If it's been explicitly set to something other than default, use it
    if (customColour != defaultAccent) {
        return customColour;
    }

    // Otherwise use default neon green
    return defaultAccent;
}
