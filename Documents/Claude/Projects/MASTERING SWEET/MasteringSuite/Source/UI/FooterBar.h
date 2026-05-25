#pragma once
#include <JuceHeader.h>
#include "Theme.h"

#include "SegmentedControl.h"

class MasteringSuiteProcessor;

class FooterBar : public juce::Component {
public:
    explicit FooterBar(MasteringSuiteProcessor& proc);
    ~FooterBar() override = default;

    void paint(juce::Graphics& g) override;
    void resized() override;
    void refresh();

private:
    MasteringSuiteProcessor& processor;
    std::unique_ptr<SegmentedControl> densityToggle;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FooterBar)
};
