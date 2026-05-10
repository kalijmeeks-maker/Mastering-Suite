#pragma once
#include <JuceHeader.h>
#include "../Processing/MasteringLimiter.h"

class GainReductionMeter : public juce::Component, private juce::Timer
{
public:
    explicit GainReductionMeter(MasteringLimiter& limiter);
    ~GainReductionMeter() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    void timerCallback() override;
    juce::Colour getSegmentColour(double gainReduction);

    MasteringLimiter& limiter;
    double currentGainReduction = 0.0;
    double peakHold = 0.0;
    int peakHoldCounter = 0;

    static constexpr int NUM_SEGMENTS = 40;
    static constexpr int SEGMENT_WIDTH = 4;
    static constexpr int PEAK_HOLD_TIME = 30;
};
