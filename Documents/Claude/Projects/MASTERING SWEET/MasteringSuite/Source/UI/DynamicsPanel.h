#pragma once
#include <JuceHeader.h>
#include "Theme.h"
#include "NeonLookAndFeel.h"

class MasteringSuiteProcessor;

class DynamicsPanel : public juce::Component {
public:
    explicit DynamicsPanel(MasteringSuiteProcessor& proc, NeonLookAndFeel& laf);
    ~DynamicsPanel() override = default;

    void paint(juce::Graphics& g) override;
    void resized() override;
    void refresh();

private:
    MasteringSuiteProcessor& processor;
    NeonLookAndFeel& lookAndFeel;

    void drawTransferCurve(juce::Graphics& g, juce::Rectangle<float> area);

    std::unique_ptr<juce::Slider> threshK, ratioK, kneeK, attackK, releaseK, makeupK, mixK;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> threshA, ratioA, kneeA, attackA, releaseA, makeupA, mixA;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DynamicsPanel)
};
