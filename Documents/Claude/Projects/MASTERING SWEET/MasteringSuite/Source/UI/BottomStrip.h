#pragma once
#include <JuceHeader.h>
#include "Theme.h"
#include "RotaryKnob.h"

class BottomStrip : public juce::Component {
public:
    BottomStrip(juce::AudioProcessorValueTreeState& apvts);
    ~BottomStrip() override = default;

    void paint(juce::Graphics& g) override;
    void resized() override;

    void setMeterValues(float lufsI, float lufsS, float lufsM, float truePeak);
    void setCurrentParameter(const juce::String& name, const juce::String& value);

private:
    juce::AudioProcessorValueTreeState& apvts;
    std::unique_ptr<RotaryKnob> inGainKnob, outGainKnob;
    
    float lufsIntegrated = 0.0f, lufsShort = 0.0f, lufsMax = 0.0f, truePeakValue = -inf;
    juce::String currentParamName = "Threshold";
    juce::String currentParamValue = "-18.0 dB";
    
    static constexpr float inf = std::numeric_limits<float>::infinity();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BottomStrip)
};
