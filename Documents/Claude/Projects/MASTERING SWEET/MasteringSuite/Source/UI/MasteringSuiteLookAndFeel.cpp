#include "MasteringSuiteLookAndFeel.h"
#include <cmath>

using namespace MasteringSuiteColors;

MasteringSuiteLookAndFeel::MasteringSuiteLookAndFeel()
{
    setColour (juce::ResizableWindow::backgroundColourId, juce::Colour(ColorBackground));
    setColour (juce::ComboBox::backgroundColourId, juce::Colour(ColorBackground));
    setColour (juce::ComboBox::outlineColourId, juce::Colour(ColorBorder));
    setColour (juce::ComboBox::textColourId, juce::Colour(ColorTextPrimary));
    setColour (juce::PopupMenu::backgroundColourId, juce::Colour(ColorBackground));
    setColour (juce::PopupMenu::textColourId, juce::Colour(ColorTextPrimary));
    setColour (juce::PopupMenu::highlightedBackgroundColourId, juce::Colour(ColorLUFSAccent));
    setColour (juce::PopupMenu::highlightedTextColourId, juce::Colour(ColorBackground));
    setColour (juce::Label::textColourId, juce::Colour(ColorTextPrimary));
    setColour (juce::Slider::backgroundColourId, juce::Colour(ColorBackground));
    setColour (juce::Slider::thumbColourId, juce::Colour(ColorLUFSAccent));
    setColour (juce::Slider::trackColourId, juce::Colour(ColorGrid));
}

void MasteringSuiteLookAndFeel::drawButtonBackground (juce::Graphics& g, juce::Button& button,
                                                       const juce::Colour& backgroundColour,
                                                       bool shouldDrawButtonAsHighlighted,
                                                       bool shouldDrawButtonAsDown)
{
    auto bounds = button.getLocalBounds().toFloat();
    auto cornerSize = 6.0f;

    if (shouldDrawButtonAsDown)
        g.setColour (juce::Colour(ColorLUFSAccent));
    else if (shouldDrawButtonAsHighlighted)
        g.setColour (juce::Colour(ColorBorder).brighter(0.3f));
    else
        g.setColour (juce::Colour(ColorBorder));

    g.fillRoundedRectangle (bounds, cornerSize);
    g.setColour (juce::Colour(ColorBorder));
    g.drawRoundedRectangle (bounds, cornerSize, 1.0f);
}

void MasteringSuiteLookAndFeel::drawComboBox (juce::Graphics& g, int width, int height, bool isButtonDown,
                                               int buttonX, int buttonY, int buttonW, int buttonH,
                                               juce::ComboBox& box)
{
    auto bounds = juce::Rectangle<int> (0, 0, width, height).toFloat();
    g.setColour (juce::Colour(ColorBackground));
    g.fillRoundedRectangle (bounds, 4.0f);

    g.setColour (isButtonDown ? juce::Colour(ColorLUFSAccent) : juce::Colour(ColorBorder));
    g.drawRoundedRectangle (bounds, 4.0f, 1.5f);

    auto arrowZone = juce::Rectangle<int> (buttonX, buttonY, buttonW, buttonH).toFloat();
    auto arrowColour = box.isEnabled() ? juce::Colour(ColorTextPrimary) : juce::Colour(ColorTextSecondary);

    g.setColour (arrowColour);
    auto arrowPath = juce::Path();
    arrowPath.startNewSubPath (arrowZone.getCentreX() - 3.0f, arrowZone.getCentreY() - 1.0f);
    arrowPath.lineTo (arrowZone.getCentreX() + 3.0f, arrowZone.getCentreY() - 1.0f);
    arrowPath.lineTo (arrowZone.getCentreX(), arrowZone.getCentreY() + 2.0f);
    arrowPath.closeSubPath();
    g.fillPath (arrowPath);
}

void MasteringSuiteLookAndFeel::drawLabel (juce::Graphics& g, juce::Label& label)
{
    g.setColour (label.findColour (juce::Label::textColourId));
    g.setFont (12.0f);
    g.drawFittedText (label.getText(), label.getLocalBounds(), juce::Justification::centredLeft, 1);
}

void MasteringSuiteLookAndFeel::drawLinearSlider (juce::Graphics& g, int x, int y, int width, int height,
                                                   float sliderPos, float minSliderPos, float maxSliderPos,
                                                   const juce::Slider::SliderStyle style, juce::Slider& slider)
{
    auto trackX = x;
    auto trackY = y + height / 2 - 2;
    auto trackWidth = width;
    auto trackHeight = 4;

    auto trackBounds = juce::Rectangle<int> (trackX, trackY, trackWidth, trackHeight).toFloat();

    g.setColour (juce::Colour(ColorGrid));
    g.fillRoundedRectangle (trackBounds, 2.0f);

    auto activeBounds = juce::Rectangle<float> (trackX, trackY, sliderPos - trackX, trackHeight);
    auto activeCyan = juce::Colour(ColorLUFSAccent);
    auto gradient = juce::ColourGradient (activeCyan, activeBounds.getX(), activeBounds.getY(),
                                          activeCyan.darker(0.3f), activeBounds.getRight(), activeBounds.getY(), false);
    g.setGradientFill (gradient);
    g.fillRoundedRectangle (activeBounds, 2.0f);

    auto thumbSize = 12;
    auto thumbX = sliderPos - thumbSize / 2;
    auto thumbY = y + height / 2 - thumbSize / 2;
    auto thumbBounds = juce::Rectangle<float> (thumbX, thumbY, thumbSize, thumbSize);

    g.setColour (juce::Colour(ColorLUFSAccent));
    g.fillRoundedRectangle (thumbBounds, 3.0f);

    g.setColour (juce::Colour(ColorLUFSAccent).brighter(0.4f));
    g.drawRoundedRectangle (thumbBounds, 3.0f, 1.0f);
}

void MasteringSuiteLookAndFeel::drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height,
                                                   float sliderPosProportional, float rotaryStartAngle,
                                                   float rotaryEndAngle, juce::Slider& slider)
{
    auto bounds = juce::Rectangle<int> (x, y, width, height).toFloat();
    auto centreX = bounds.getCentreX();
    auto centreY = bounds.getCentreY();
    auto radius = juce::jmin (bounds.getWidth(), bounds.getHeight()) / 2.1f;

    // Determine knob color based on parameter type
    juce::Colour knobColour = juce::Colour(ColorGrid);
    juce::Colour accentColour = juce::Colour(ColorLUFSAccent);

    auto name = slider.getName();
    if (name.containsIgnoreCase("Gain") || name.containsIgnoreCase("Makeup"))
    {
        knobColour = juce::Colour(ColorPeakWarning);
        accentColour = juce::Colour(ColorPeakWarning);
    }
    else if (name.containsIgnoreCase("Threshold"))
    {
        knobColour = juce::Colour(ColorGainReduction);
        accentColour = juce::Colour(ColorGainReduction);
    }
    else if (name.containsIgnoreCase("Type") || name.containsIgnoreCase("Bypass"))
    {
        knobColour = juce::Colour(ColorClipDanger);
        accentColour = juce::Colour(ColorClipDanger);
    }
    else if (name.containsIgnoreCase("Q") || name.containsIgnoreCase("Freq"))
    {
        knobColour = juce::Colour(ColorLUFSAccent);
        accentColour = juce::Colour(ColorLUFSAccent);
    }

    // Draw background circle for knob base
    g.setColour (juce::Colour(ColorBackground));
    g.fillEllipse (centreX - radius - 2, centreY - radius - 2, radius * 2 + 4, radius * 2 + 4);

    // Draw main knob body with radial gradient
    juce::ColourGradient knobGradient (
        knobColour.brighter(0.4f),
        centreX - radius * 0.3f, centreY - radius * 0.3f,
        knobColour.darker(0.3f),
        centreX + radius * 0.5f, centreY + radius * 0.5f,
        true
    );
    g.setGradientFill (knobGradient);
    g.fillEllipse (centreX - radius, centreY - radius, radius * 2.0f, radius * 2.0f);

    // Draw knob outline
    g.setColour (juce::Colour(ColorBorder).withAlpha(0.6f));
    g.drawEllipse (centreX - radius, centreY - radius, radius * 2.0f, radius * 2.0f, 1.5f);

    // Draw highlight/shine
    auto shinePath = juce::Path();
    auto shineRadius = radius * 0.35f;
    shinePath.addEllipse (centreX - shineRadius * 0.7f - radius * 0.3f,
                         centreY - shineRadius * 1.2f - radius * 0.3f,
                         shineRadius * 1.4f, shineRadius * 1.0f);
    g.setColour (juce::Colour(ColorTextPrimary).withAlpha(0.4f));
    g.fillPath (shinePath);

    // Draw rotating needle indicator
    auto currentAngle = rotaryStartAngle + (rotaryEndAngle - rotaryStartAngle) * sliderPosProportional;
    auto needleLength = radius * 0.85f;
    auto needleThickness = 3.0f;

    g.setColour (accentColour.brighter(0.3f));
    g.drawLine (centreX, centreY,
               centreX + needleLength * std::cos(currentAngle - juce::MathConstants<float>::halfPi),
               centreY + needleLength * std::sin(currentAngle - juce::MathConstants<float>::halfPi),
               needleThickness);

    // Draw center pivot point
    g.setColour (juce::Colour(ColorBorder));
    g.fillEllipse (centreX - 4, centreY - 4, 8, 8);
    g.setColour (accentColour.withAlpha(0.7f));
    g.fillEllipse (centreX - 2.5f, centreY - 2.5f, 5, 5);

    // Draw scale marks
    g.setColour (juce::Colour(ColorGrid).withAlpha(0.5f));
    auto numMarks = 11;
    for (int i = 0; i < numMarks; ++i)
    {
        auto angle = rotaryStartAngle + (rotaryEndAngle - rotaryStartAngle) * (i / (numMarks - 1.0f));
        auto markInnerRadius = radius * 0.75f;
        auto markOuterRadius = radius * 0.95f;

        auto x1 = centreX + markInnerRadius * std::cos(angle - juce::MathConstants<float>::halfPi);
        auto y1 = centreY + markInnerRadius * std::sin(angle - juce::MathConstants<float>::halfPi);
        auto x2 = centreX + markOuterRadius * std::cos(angle - juce::MathConstants<float>::halfPi);
        auto y2 = centreY + markOuterRadius * std::sin(angle - juce::MathConstants<float>::halfPi);

        g.drawLine (x1, y1, x2, y2, 0.8f);
    }
}
