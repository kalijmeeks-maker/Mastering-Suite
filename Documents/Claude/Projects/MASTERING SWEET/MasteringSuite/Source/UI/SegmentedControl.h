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
    void setAccentColor(juce::Colour c);
    void setVariant(PillButton::Variant v);

    std::function<void(int)> onSelectionChanged;

private:
    std::vector<std::unique_ptr<PillButton>> buttons;
    int selectedIndex = -1;
    std::unique_ptr<juce::ParameterAttachment> paramAttachment;
    PillButton::Variant buttonVariant = PillButton::Variant::Filled;
    juce::Colour buttonAccent = juce::Colour(0xFFF3F3FA);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SegmentedControl)
};
