#pragma once
#include <JuceHeader.h>
#include "../Metering/EbuR128Meter.h"
#include <array>

class LoudnessGraph : public juce::Component, private juce::Timer
{
public:
    explicit LoudnessGraph(EbuR128Meter& meter);
    ~LoudnessGraph() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    void timerCallback() override;
    void drawGrid(juce::Graphics& g);
    void drawCurve(juce::Graphics& g);
    void drawTargetLine(juce::Graphics& g);

    EbuR128Meter& meter;
    std::array<double, 600> buffer{};
    int writeIndex = 0;
    int sampleCount = 0;

    static constexpr double MIN_LUFS = -30.0;
    static constexpr double MAX_LUFS = 0.0;
    static constexpr double TARGET_LUFS = -14.0;
    static constexpr int BUFFER_SIZE = 600;
};
