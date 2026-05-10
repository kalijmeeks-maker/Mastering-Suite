#pragma once

#include <JuceHeader.h>
#include "Theme.h"
#include "FrequencyResponseCurve.h"
#include "RotaryKnob.h"

class MasteringSuiteProcessor;

class MasteringSuiteEditor : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    explicit MasteringSuiteEditor(MasteringSuiteProcessor&);
    ~MasteringSuiteEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;
    void timerCallback() override;

private:
    MasteringSuiteProcessor& processor;
    std::unique_ptr<FrequencyResponseCurve> freqCurve;
    std::array<std::unique_ptr<RotaryKnob>, 6> bandGainKnobs;
    std::array<std::unique_ptr<RotaryKnob>, 6> bandFreqKnobs;
    std::array<std::unique_ptr<RotaryKnob>, 6> bandQKnobs;
    std::array<std::unique_ptr<juce::ComboBox>, 6> bandTypeBoxes;
    std::array<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>, 6> gainAttachments;
    std::array<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>, 6> freqAttachments;
    std::array<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>, 6> qAttachments;

    juce::Label lufsLabel, grLabel;
    float lufsIntegrated = -std::numeric_limits<float>::infinity();
    float grCurrent = 0.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MasteringSuiteEditor)
};
