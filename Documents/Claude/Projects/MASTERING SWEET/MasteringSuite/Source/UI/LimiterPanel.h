#pragma once
#include <JuceHeader.h>
#include "Theme.h"
#include "NeonLookAndFeel.h"

class MasteringSuiteProcessor;

class LimiterPanel : public juce::Component {
public:
    explicit LimiterPanel(MasteringSuiteProcessor& proc, NeonLookAndFeel& laf);
    ~LimiterPanel() override = default;

    void paint(juce::Graphics& g) override;
    void resized() override;
    void refresh();

private:
    MasteringSuiteProcessor& processor;
    NeonLookAndFeel& lookAndFeel;

    std::unique_ptr<juce::Slider> threshKnob, releaseKnob, makeupKnob;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> threshAttach, releaseAttach, makeupAttach;

    float currentGainReduction = 0.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LimiterPanel)
};
