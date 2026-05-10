#pragma once
#include <JuceHeader.h>
#include "Theme.h"

class PillButton : public juce::ToggleButton {
public:
    PillButton();
    ~PillButton() override = default;

    void paint(juce::Graphics& g) override;
    void setSegmentGroup(bool isFirst, bool isLast);

private:
    bool isFirstInGroup = false;
    bool isLastInGroup = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PillButton)
};
