#include "TransferCurveComponent.h"
#include <cmath>

TransferCurveComponent::TransferCurveComponent()
{
    rebuildCurve();
}

void TransferCurveComponent::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    
    g.setColour(Theme::Color::toColour(Theme::Color::BG_2));
    g.fillRect(bounds);
    g.setColour(Theme::Color::toColour(Theme::Color::BG_3));
    g.drawRoundedRectangle(bounds, 4.0f, 1.0f);
    
    g.setColour(Theme::Color::toColour(Theme::Color::LINE).withAlpha(0.4f));
    for (int i = 0; i <= 4; ++i) {
        float x = bounds.getX() + 20 + i * (bounds.getWidth() - 40) / 4.0f;
        float y = bounds.getY() + 20 + i * (bounds.getHeight() - 40) / 4.0f;
        g.drawVerticalLine((int)x, bounds.getY() + 20, bounds.getBottom() - 20);
        g.drawHorizontalLine((int)y, bounds.getX() + 20, bounds.getRight() - 20);
    }
    
    g.setColour(Theme::Color::toColour(Theme::Color::ACCENT));
    g.fillPath(curvePath);
    
    g.setColour(Theme::Color::toColour(Theme::Color::TEXT_DIM));
    g.setFont(Theme::Font::make(Theme::Font::SIZE_MICRO));
    g.drawText("IN dB", bounds.removeFromLeft(20), juce::Justification::centred);
    g.drawText("OUT dB", bounds.removeFromTop(20), juce::Justification::centred);
}

void TransferCurveComponent::setParameters(float threshold, float ratio, float knee, Mode mode)
{
    thresholdDb = threshold;
    ratioDb = ratio;
    kneeDb = knee;
    compressionMode = mode;
    rebuildCurve();
    repaint();
}

float TransferCurveComponent::computeGain(float inputDb) const
{
    if (compressionMode == Mode::Comp) {
        float kneeStart = thresholdDb - kneeDb / 2.0f;
        float kneeEnd = thresholdDb + kneeDb / 2.0f;
        
        if (inputDb < kneeStart) {
            return 0.0f;
        } else if (inputDb < kneeEnd) {
            float kneeProgress = (inputDb - kneeStart) / kneeDb;
            return -kneeProgress * kneeProgress * (thresholdDb + (inputDb - thresholdDb) * (1.0f - 1.0f / ratioDb));
        } else {
            return -(inputDb - thresholdDb) * (1.0f - 1.0f / ratioDb);
        }
    }
    return 0.0f;
}

void TransferCurveComponent::rebuildCurve()
{
    auto bounds = getLocalBounds().toFloat().reduced(20.0f);
    curvePath.clear();
    
    bool started = false;
    for (int i = 0; i <= 100; ++i) {
        float normalizedInput = i / 100.0f;
        float inputDb = -60.0f + normalizedInput * 60.0f;
        float gainDb = computeGain(inputDb);
        float outputDb = inputDb + gainDb;
        
        float x = bounds.getX() + normalizedInput * bounds.getWidth();
        float y = bounds.getBottom() - ((outputDb + 60.0f) / 60.0f) * bounds.getHeight();
        
        if (!started) {
            curvePath.startNewSubPath(x, y);
            started = true;
        } else {
            curvePath.lineTo(x, y);
        }
    }
}
