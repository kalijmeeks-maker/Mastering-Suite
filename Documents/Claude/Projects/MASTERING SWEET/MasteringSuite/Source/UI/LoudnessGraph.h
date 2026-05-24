#pragma once
#include <JuceHeader.h>
#include "Theme.h"
#include <deque>

class MasteringSuiteProcessor;

class LoudnessGraph : public juce::Component {
public:
    explicit LoudnessGraph(MasteringSuiteProcessor& proc);
    ~LoudnessGraph() override = default;

    void paint(juce::Graphics& g) override;
    void resized() override;
    void pushSample(float lufsValue);

private:
    MasteringSuiteProcessor& processor;
    std::deque<float> history;
    static constexpr int maxSamples = 240;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LoudnessGraph)
};
