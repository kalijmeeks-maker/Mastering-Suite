#pragma once
#include <JuceHeader.h>

class VUMeter : public juce::Component, public juce::Timer
{
public:
    VUMeter (const juce::String& name);
    ~VUMeter() override;

    void paint (juce::Graphics& g) override;
    void timerCallback() override;
    void setValue (float newValue);
    void setPeakValue (float newPeak);

private:
    juce::String meterName;
    float targetValue = -60.0f;
    float currentValue = -60.0f;
    float peakValue = -60.0f;
    float peakDecay = -60.0f;

    void drawMeterBackground (juce::Graphics& g, juce::Rectangle<float> bounds);
    void drawNeedle (juce::Graphics& g, juce::Rectangle<float> bounds, float normalizedValue);
    void drawScale (juce::Graphics& g, juce::Rectangle<float> bounds);
    void drawPeakIndicator (juce::Graphics& g, juce::Rectangle<float> bounds, float peakNormalized);
};
