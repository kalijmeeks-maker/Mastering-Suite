#pragma once
#include <JuceHeader.h>
#include "Theme.h"

class SweetHeader : public juce::Component {
public:
    SweetHeader();
    ~SweetHeader() override = default;

    void paint(juce::Graphics& g) override;
    void setBypassActive(bool active) { bypassActive = active; repaint(); }

private:
    bool bypassActive = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SweetHeader)
};
