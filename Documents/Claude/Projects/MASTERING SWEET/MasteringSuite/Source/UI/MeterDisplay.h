#pragma once
#include <JuceHeader.h>

class MeterDisplay : public juce::Component, public juce::Timer
{
public:
    MeterDisplay (const juce::String& meterName, const juce::String& unit);
    ~MeterDisplay() override;

    void paint (juce::Graphics& g) override;
    void timerCallback() override;

    void setValue (float newValue);
    float getCurrentValue() const { return currentValue; }

    void setAccentColour (juce::Colour newColour) { accentColour = newColour; }

private:
    juce::String labelText;
    juce::String unitText;
    float targetValue = -60.0f;
    float currentValue = -60.0f;
    float peakValue = -60.0f;
    juce::Colour accentColour;

    void drawMeterBar (juce::Graphics& g, juce::Rectangle<float> bounds, float normalizedValue);
    void drawPeakIndicator (juce::Graphics& g, juce::Rectangle<float> bounds);
};
