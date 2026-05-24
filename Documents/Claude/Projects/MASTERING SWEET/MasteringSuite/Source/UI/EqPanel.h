#pragma once
#include <JuceHeader.h>
#include "Theme.h"
#include "NeonLookAndFeel.h"

class MasteringSuiteProcessor;

class EqPanel : public juce::Component {
public:
    explicit EqPanel(MasteringSuiteProcessor& proc, NeonLookAndFeel& laf);
    ~EqPanel() override = default;

    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    MasteringSuiteProcessor& processor;
    NeonLookAndFeel& lookAndFeel;

    std::unique_ptr<juce::Slider> gainKnob, freqKnob, qKnob;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> gainAttach, freqAttach, qAttach;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EqPanel)
};
