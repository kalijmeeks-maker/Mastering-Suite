#pragma once
#include <JuceHeader.h>
#include "../Metering/EbuR128Meter.h"

class LUFSDisplay : public juce::Component, private juce::Timer
{
public:
    explicit LUFSDisplay(EbuR128Meter& meter);
    ~LUFSDisplay() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    void timerCallback() override;

    EbuR128Meter& meter;
    double momentary = -inf;
    double shortTerm = -inf;
    double integrated = -inf;

    static constexpr double inf = -200.0;
};
