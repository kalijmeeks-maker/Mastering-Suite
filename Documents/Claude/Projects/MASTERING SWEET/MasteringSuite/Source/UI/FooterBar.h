#pragma once
#include <JuceHeader.h>
#include "Theme.h"

class FooterBar : public juce::Component {
public:
    FooterBar();
    ~FooterBar() override = default;

    void paint(juce::Graphics& g) override;

    void setStatusText(const juce::String& status);
    void setCpuUsage(float percent);

private:
    juce::String statusText = "SR 48.0K · 32-BIT FLOAT · LATENCY 0.0 MS";
    float cpuPercent = 0.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FooterBar)
};
