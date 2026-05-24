#pragma once
#include <JuceHeader.h>
#include "Theme.h"

class MasteringSuiteProcessor;

class HeaderBar : public juce::Component {
public:
    explicit HeaderBar(MasteringSuiteProcessor& proc);
    ~HeaderBar() override = default;

    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    MasteringSuiteProcessor& processor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(HeaderBar)
};
