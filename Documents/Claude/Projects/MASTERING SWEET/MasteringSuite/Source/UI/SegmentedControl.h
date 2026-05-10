#pragma once
#include <JuceHeader.h>
#include "PillButton.h"

class SegmentedControl : public juce::Component {
public:
    SegmentedControl();
    ~SegmentedControl() override = default;

    void paint(juce::Graphics& g) override;
    void resized() override;

    void addButton(const juce::String& label);
    void setSelectedIndex(int index);
    int getSelectedIndex() const { return selectedIndex; }

    void connectParameter(juce::AudioProcessorValueTreeState& apvts, const juce::String& paramID);
    std::function<void(int)> onSelectionChanged;

private:
    std::vector<std::unique_ptr<PillButton>> buttons;
    int selectedIndex = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SegmentedControl)
};
