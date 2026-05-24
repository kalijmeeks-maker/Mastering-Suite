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

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LufsPanel)
};
