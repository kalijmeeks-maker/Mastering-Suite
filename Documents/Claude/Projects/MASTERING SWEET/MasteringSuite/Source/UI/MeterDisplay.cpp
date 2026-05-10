#include "MeterDisplay.h"

MeterDisplay::MeterDisplay (const juce::String& meterName, const juce::String& unit)
    : labelText (meterName), unitText (unit), accentColour (juce::Colour(0xff00d4ff))
{
    startTimer (33);
}

MeterDisplay::~MeterDisplay()
{
    stopTimer();
}

void MeterDisplay::paint (juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();

    g.setColour (juce::Colour(0xff242424));
    g.fillRoundedRectangle (bounds, 8.0f);

    g.setColour (juce::Colour(0xff333333));
    g.drawRoundedRectangle (bounds, 8.0f, 1.0f);

    auto labelArea = bounds.removeFromTop (24.0f);
    g.setColour (juce::Colour(0xffe8e8e8));
    g.setFont (11.0f);
    g.drawFittedText (labelText, labelArea.toNearestInt(), juce::Justification::centredLeft, 1);

    auto meterArea = bounds.reduced (8.0f).removeFromTop (30.0f);
    drawMeterBar (g, meterArea, (currentValue + 60.0f) / 60.0f);

    auto valueArea = bounds.reduced (8.0f);
    g.setColour (accentColour.brighter(0.3f));
    g.setFont (13.0f);
    auto valueStr = juce::String (currentValue, 1) + " " + unitText;
    g.drawFittedText (valueStr, valueArea.toNearestInt(), juce::Justification::centredLeft, 1);
}

void MeterDisplay::timerCallback()
{
    if (std::abs (currentValue - targetValue) > 0.1f)
    {
        currentValue += (targetValue - currentValue) * 0.15f;
        repaint();
    }
}

void MeterDisplay::setValue (float newValue)
{
    targetValue = juce::jlimit (-60.0f, 12.0f, newValue);
    if (targetValue > peakValue)
        peakValue = targetValue;
}

void MeterDisplay::drawMeterBar (juce::Graphics& g, juce::Rectangle<float> bounds, float normalizedValue)
{
    g.setColour (juce::Colour(0xff1a1a1a));
    g.fillRoundedRectangle (bounds, 3.0f);

    auto activeBounds = bounds.removeFromLeft (bounds.getWidth() * normalizedValue);

    auto gradient = juce::ColourGradient (
        accentColour,
        activeBounds.getX(), activeBounds.getY(),
        accentColour.darker(0.4f),
        activeBounds.getRight(), activeBounds.getY(),
        false
    );
    g.setGradientFill (gradient);
    g.fillRoundedRectangle (activeBounds, 3.0f);

    g.setColour (accentColour.brighter(0.5f));
    g.drawRoundedRectangle (activeBounds, 3.0f, 1.0f);
}

void MeterDisplay::drawPeakIndicator (juce::Graphics& g, juce::Rectangle<float> bounds)
{
    auto peakX = bounds.getX() + (bounds.getWidth() * ((peakValue + 60.0f) / 60.0f));
    g.setColour (juce::Colour(0xffff3333).brighter(0.2f));
    g.drawLine (peakX, bounds.getY(), peakX, bounds.getBottom(), 2.0f);
}
