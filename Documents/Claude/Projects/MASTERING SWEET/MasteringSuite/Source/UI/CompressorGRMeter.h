#pragma once
#include <JuceHeader.h>
#include "Theme.h"

class MasteringCompressor;

class CompressorGRMeter : public juce::Component, private juce::Timer {
public:
    CompressorGRMeter(MasteringCompressor& comp);
    ~CompressorGRMeter() override;

    void paint(juce::Graphics& g) override;
    void timerCallback() override;

private:
    MasteringCompressor& compressor;
    float currentGainReduction = 0.0f;
    float peakGainReduction = 0.0f;
    int peakHoldFrames = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CompressorGRMeter)
};
