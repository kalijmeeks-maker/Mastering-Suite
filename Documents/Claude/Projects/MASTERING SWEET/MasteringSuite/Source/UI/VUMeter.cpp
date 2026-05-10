#include "VUMeter.h"

VUMeter::VUMeter (const juce::String& name) : meterName (name)
{
    startTimer (30);
}

VUMeter::~VUMeter()
{
    stopTimer();
}

void VUMeter::paint (juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    auto meterArea = bounds.removeFromTop (bounds.getHeight() * 0.85f);
    auto labelArea = bounds;

    drawMeterBackground (g, meterArea);
    auto normalizedValue = (currentValue + 60.0f) / 60.0f;
    drawNeedle (g, meterArea, juce::jlimit (0.0f, 1.0f, normalizedValue));
    drawScale (g, meterArea);

    auto peakNormalized = (peakDecay + 60.0f) / 60.0f;
    drawPeakIndicator (g, meterArea, juce::jlimit (0.0f, 1.0f, peakNormalized));

    g.setColour (juce::Colour(0xffe8e8e8));
    g.setFont (11.0f);
    g.drawFittedText (meterName, labelArea.toNearestInt(), juce::Justification::centred, 1);
}

void VUMeter::timerCallback()
{
    if (std::abs (currentValue - targetValue) > 0.05f)
    {
        currentValue += (targetValue - currentValue) * 0.2f;
        repaint();
    }

    peakDecay -= 0.5f;
    if (peakDecay < -60.0f) peakDecay = -60.0f;
}

void VUMeter::setValue (float newValue)
{
    targetValue = juce::jlimit (-60.0f, 12.0f, newValue);
    if (targetValue > peakValue)
    {
        peakValue = targetValue;
        peakDecay = targetValue;
    }
}

void VUMeter::setPeakValue (float newPeak)
{
    peakValue = newPeak;
    peakDecay = newPeak;
}

void VUMeter::drawMeterBackground (juce::Graphics& g, juce::Rectangle<float> bounds)
{
    g.setColour (juce::Colour(0xff1a1a1a));
    g.fillEllipse (bounds);

    auto gradient = juce::ColourGradient (
        juce::Colour(0xff2a2a2a),
        bounds.getCentreX(), bounds.getY(),
        juce::Colour(0xff1a1a1a),
        bounds.getCentreX(), bounds.getBottom(),
        false
    );
    g.setGradientFill (gradient);
    g.fillEllipse (bounds.reduced (2.0f));

    g.setColour (juce::Colour(0xff00d4ff).withAlpha (0.3f));
    g.drawEllipse (bounds, 2.0f);

    auto innerBounds = bounds.reduced (bounds.getHeight() * 0.15f);
    g.setColour (juce::Colour(0xff00d4ff).withAlpha (0.15f));
    g.drawEllipse (innerBounds, 1.0f);
}

void VUMeter::drawScale (juce::Graphics& g, juce::Rectangle<float> bounds)
{
    auto centre = bounds.getCentre();
    auto radius = bounds.getWidth() * 0.35f;

    g.setColour (juce::Colour(0xff666666));
    for (int i = 0; i <= 10; ++i)
    {
        auto angle = juce::MathConstants<float>::pi + (i / 10.0f) * juce::MathConstants<float>::pi;
        auto cos = std::cos (angle);
        auto sin = std::sin (angle);

        auto outerPoint = centre.translated (cos * radius, sin * radius);
        auto innerPoint = centre.translated (cos * (radius - 8.0f), sin * (radius - 8.0f));

        g.drawLine (innerPoint.x, innerPoint.y, outerPoint.x, outerPoint.y, 1.5f);
    }

    g.setColour (juce::Colour(0xff888888));
    g.setFont (8.0f);
    for (int i = 0; i <= 10; ++i)
    {
        auto angle = juce::MathConstants<float>::pi + (i / 10.0f) * juce::MathConstants<float>::pi;
        auto cos = std::cos (angle);
        auto sin = std::sin (angle);

        auto textRadius = radius - 18.0f;
        auto textPoint = centre.translated (cos * textRadius, sin * textRadius);
        auto lufsValue = -60 + (i * 12);

        g.drawFittedText (juce::String (lufsValue),
                         juce::Rectangle<int> (textPoint.x - 15, textPoint.y - 8, 30, 16),
                         juce::Justification::centred, 1);
    }
}

void VUMeter::drawNeedle (juce::Graphics& g, juce::Rectangle<float> bounds, float normalizedValue)
{
    auto centre = bounds.getCentre();
    auto radius = bounds.getWidth() * 0.35f;

    auto angle = juce::MathConstants<float>::pi + (normalizedValue * juce::MathConstants<float>::pi);
    auto cos = std::cos (angle);
    auto sin = std::sin (angle);

    auto needleEnd = centre.translated (cos * (radius * 0.8f), sin * (radius * 0.8f));

    g.setColour (juce::Colour(0xffff3333));
    g.drawLine (centre.x, centre.y, needleEnd.x, needleEnd.y, 3.0f);

    g.setColour (juce::Colour(0xffff6666));
    g.drawLine (centre.x, centre.y, needleEnd.x, needleEnd.y, 1.5f);

    g.setColour (juce::Colour(0xff222222));
    g.fillEllipse (centre.x - 4.0f, centre.y - 4.0f, 8.0f, 8.0f);

    g.setColour (juce::Colour(0xff666666));
    g.drawEllipse (centre.x - 4.0f, centre.y - 4.0f, 8.0f, 8.0f, 1.0f);
}

void VUMeter::drawPeakIndicator (juce::Graphics& g, juce::Rectangle<float> bounds, float peakNormalized)
{
    auto centre = bounds.getCentre();
    auto radius = bounds.getWidth() * 0.35f;

    auto angle = juce::MathConstants<float>::pi + (peakNormalized * juce::MathConstants<float>::pi);
    auto cos = std::cos (angle);
    auto sin = std::sin (angle);

    auto peakPoint = centre.translated (cos * (radius * 0.85f), sin * (radius * 0.85f));

    g.setColour (juce::Colour(0xffff0000).withAlpha (0.8f));
    g.fillEllipse (peakPoint.x - 3.0f, peakPoint.y - 3.0f, 6.0f, 6.0f);

    g.setColour (juce::Colour(0xffffff00));
    g.drawEllipse (peakPoint.x - 3.0f, peakPoint.y - 3.0f, 6.0f, 6.0f, 1.5f);
}
