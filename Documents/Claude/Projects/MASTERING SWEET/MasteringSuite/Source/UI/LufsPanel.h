#pragma once
#include <JuceHeader.h>
#include "Theme.h"

class MasteringSuiteProcessor;

class LufsPanel : public juce::Component {
public:
    explicit LufsPanel(MasteringSuiteProcessor& proc);
    ~LufsPanel() override = default;

    void paint(juce::Graphics& g) override;
    void resized() override;
    void refresh();

private:
    MasteringSuiteProcessor& processor;
    // Peak-hold state per channel: highest dBFS observed + the time it was hit.
    // Spec: hold for 3s at full brightness, then decay linearly over 2s.
    float peakHoldL = -100.0f, peakHoldR = -100.0f;
    double peakHoldTimeL = 0.0, peakHoldTimeR = 0.0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LufsPanel)
};
