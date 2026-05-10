#include "RotaryKnob.h"
#include "Theme.h"

RotaryKnob::RotaryKnob()
    : juce::Slider(juce::Slider::RotaryHorizontalDrag, juce::Slider::TextBoxBelow)
{
    setRange(0.0f, 1.0f, 0.01f);
    setValue(0.5f);
    setVelocityBasedMode(true);
    setVelocityModeParameters(1.0, 1.0, 0.1, false);
    setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
}

RotaryKnob::~RotaryKnob() = default;

void RotaryKnob::mouseEnter(const juce::MouseEvent& event)
{
    isHovered = true;
    repaint();
    juce::Slider::mouseEnter(event);
}

void RotaryKnob::mouseExit(const juce::MouseEvent& event)
{
    isHovered = false;
    repaint();
    juce::Slider::mouseExit(event);
}

void RotaryKnob::mouseDown(const juce::MouseEvent& event)
{
    isDragging = true;
    repaint();
    juce::Slider::mouseDown(event);
}

void RotaryKnob::mouseUp(const juce::MouseEvent& event)
{
    isDragging = false;
    repaint();
    juce::Slider::mouseUp(event);
}

juce::String RotaryKnob::getTextFromValue(double value)
{
    return formatValue(value);
}

juce::String RotaryKnob::formatValue(double value) const
{
    auto name = getName();

    if (name.containsIgnoreCase("Frequency") || name.containsIgnoreCase("Freq"))
    {
        if (value >= 1000.0)
            return juce::String(value / 1000.0, 2) + " kHz";
        return juce::String((int)value) + " Hz";
    }
    else if (name.containsIgnoreCase("Release"))
    {
        return juce::String((int)value) + " ms";
    }
    else if (name.containsIgnoreCase("Q"))
    {
        return juce::String(value, 2);
    }
    else if (name.containsIgnoreCase("Threshold") || name.containsIgnoreCase("Gain") || name.containsIgnoreCase("Makeup"))
    {
        return juce::String(value, 1) + " dB";
    }

    return juce::String(value, 2);
}

void RotaryKnob::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    if (bounds.isEmpty())
        return;

    auto radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) / 2.0f - 2.0f;
    auto centreX = bounds.getCentreX();
    auto centreY = bounds.getCentreY();
    auto rx = centreX - radius;
    auto ry = centreY - radius;
    auto rw = radius * 2.0f;

    auto arcWidth = Theme::Knob::ARC_WIDTH;

    // Determine accent color based on control name
    auto name = getName();
    juce::Colour colour = juce::Colour(Theme::Color::ACCENT);  // Default neon green

    if (name.containsIgnoreCase("Threshold")) {
        colour = juce::Colour(Theme::Color::MAGENTA);  // Limiter threshold = pink
    } else if (name.containsIgnoreCase("Release") || name.containsIgnoreCase("Makeup")) {
        colour = juce::Colour(Theme::Color::MAGENTA);  // Limiter controls = pink
    } else if (name.containsIgnoreCase("Freq") || name.containsIgnoreCase("Q")) {
        colour = juce::Colour(Theme::Color::CYAN);     // EQ controls = cyan
    }

    // ─────────────────────────────── Background Arc (full range, dim) ────────────
    g.setColour(juce::Colour(Theme::Color::LINE_2));
    g.drawEllipse(rx, ry, rw, rw, arcWidth);

    // ─────────────────────────────── Foreground Arc (lit, up to position) ────────
    auto minVal = getMinimum();
    auto maxVal = getMaximum();
    auto currentVal = getValue();
    auto sliderPosProportional = (maxVal > minVal) ?
        static_cast<float>((currentVal - minVal) / (maxVal - minVal)) : 0.5f;
    sliderPosProportional = juce::jlimit(0.0f, 1.0f, sliderPosProportional);

    auto rotaryStartAngle = Theme::Knob::START_ANGLE * juce::MathConstants<float>::pi / 180.0f;
    auto rotaryEndAngle = (Theme::Knob::START_ANGLE + Theme::Knob::SWEEP_DEGREES) * juce::MathConstants<float>::pi / 180.0f;
    auto sliderAngleRadians = rotaryStartAngle + sliderPosProportional * (rotaryEndAngle - rotaryStartAngle);

    juce::Path arc;
    arc.addCentredArc(centreX, centreY, radius, radius, 0.0f,
                      rotaryStartAngle, sliderAngleRadians, true);

    g.setColour(colour);
    g.strokePath(arc, juce::PathStrokeType(arcWidth, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    // ─────────────────────────────── Enhanced Glow Effect ────────────────────────
    // Outer glow: wider falloff with higher alpha (0.55 instead of 0.25)
    g.setColour(colour.withAlpha(0.55f));
    g.strokePath(arc, juce::PathStrokeType(arcWidth * 2.0f,
                                           juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    // Middle glow layer
    g.setColour(colour.withAlpha(0.35f));
    g.strokePath(arc, juce::PathStrokeType(arcWidth * 1.5f,
                                           juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    // ─────────────────────────────── Central Circle (gradient) ───────────────────
    juce::ColourGradient grad(juce::Colour(Theme::Color::BG_2), centreX, centreY - radius * 0.4f,
                              juce::Colour(Theme::Color::BG_1), centreX, centreY + radius * 0.4f, false);
    g.setGradientFill(grad);
    g.fillEllipse(centreX - radius + arcWidth, centreY - radius + arcWidth,
                  rw - arcWidth * 2.0f, rw - arcWidth * 2.0f);

    // ─────────────────────────────── Inner-Body Bloom (on hover/drag) ────────────
    if (isHovered || isDragging) {
        // Bloom effect: inner circle glow
        g.setColour(colour.withAlpha(0.15f));
        g.fillEllipse(centreX - radius + arcWidth * 2.0f, centreY - radius + arcWidth * 2.0f,
                      rw - arcWidth * 4.0f, rw - arcWidth * 4.0f);
    }

    // ─────────────────────────────── Border of Central Circle ───────────────────
    g.setColour(juce::Colour(Theme::Color::LINE_2));
    g.drawEllipse(centreX - radius + arcWidth, centreY - radius + arcWidth,
                  rw - arcWidth * 2.0f, rw - arcWidth * 2.0f, 1.0f);

    // Inner highlight (subtle)
    g.setColour(juce::Colour(Theme::Color::LINE_2).withAlpha(0.3f));
    g.drawEllipse(centreX - radius + arcWidth, centreY - radius + arcWidth,
                  rw - arcWidth * 2.0f, rw - arcWidth * 2.0f, 1.0f);

    // ─────────────────────────────── Indicator Dot ──────────────────────────────
    auto innerRadius = radius - arcWidth - 2.0f;
    auto angle = rotaryStartAngle + sliderPosProportional * (rotaryEndAngle - rotaryStartAngle);
    auto dotX = centreX + innerRadius * std::cos(angle - juce::MathConstants<float>::halfPi);
    auto dotY = centreY + innerRadius * std::sin(angle - juce::MathConstants<float>::halfPi);

    // Glow around indicator (enhanced on interaction)
    auto indicatorGlowAlpha = (isHovered || isDragging) ? 0.4f : 0.2f;
    g.setColour(colour.withAlpha(indicatorGlowAlpha));
    g.fillEllipse(dotX - 3.5f, dotY - 3.5f, 7.0f, 7.0f);

    // Indicator dot itself
    g.setColour(colour);
    g.fillEllipse(dotX - Theme::Knob::INDICATOR_SIZE / 2.0f, dotY - Theme::Knob::INDICATOR_SIZE / 2.0f,
                  Theme::Knob::INDICATOR_SIZE, Theme::Knob::INDICATOR_SIZE);

    // ─────────────────────────────── Value Bubble (on hover/drag) ────────────────
    if (isHovered || isDragging) {
        auto valueStr = formatValue(currentVal);
        auto font = juce::Font("Courier New", 15.0f, juce::Font::plain);
        juce::GlyphArrangement glyphs;
        glyphs.addLineOfText(font, valueStr, 0, 0);
        auto textWidth = glyphs.getBoundingBox(0, glyphs.getNumGlyphs(), false).getWidth();
        auto textHeight = font.getHeight();

        // Bubble bounds: above the knob
        auto bubbleX = centreX - textWidth / 2.0f - 8.0f;
        auto bubbleY = centreY - radius - textHeight - 12.0f;
        auto bubbleWidth = textWidth + 16.0f;
        auto bubbleHeight = textHeight + 8.0f;

        // Bubble background
        g.setColour(juce::Colour(Theme::Color::BG_1).withAlpha(0.9f));
        g.fillRoundedRectangle(bubbleX, bubbleY, bubbleWidth, bubbleHeight, 4.0f);


        // Value text
        g.setColour(colour);
        g.setFont(font);
        g.drawText(valueStr, bubbleX + 8.0f, bubbleY + 4.0f, textWidth, textHeight, juce::Justification::centred);
    }
}
