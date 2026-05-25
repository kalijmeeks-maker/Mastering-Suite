#pragma once
#include <JuceHeader.h>
#include "Theme.h"
#include "PillButton.h"
#include "SegmentedControl.h"

class MasteringSuiteProcessor;

class HeaderBar : public juce::Component {
public:
    explicit HeaderBar(MasteringSuiteProcessor& proc);
    ~HeaderBar() override = default;

    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    MasteringSuiteProcessor& processor;

    SegmentedControl abToggle;
    PillButton presetButton;
    PillButton bypassButton;
    PillButton oversampleButton;

    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> bypassAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> oversampleAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(HeaderBar)
};
