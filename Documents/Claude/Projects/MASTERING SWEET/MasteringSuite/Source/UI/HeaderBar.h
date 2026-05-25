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
    // Called by the editor's 30Hz timer so we can refresh the preset modified bullet.
    void refresh();

private:
    // v1.1-1: + NEW... opens this; pops a juce::AlertWindow with a single text input.
    void showSavePresetDialog();
    juce::String currentDisplayName() const;  // factory name OR user preset name

    MasteringSuiteProcessor& processor;

    SegmentedControl abToggle;
    PillButton copyABButton;   // ↔ copies active bank → other bank
    PillButton presetButton;
    PillButton bypassButton;
    PillButton oversampleButton;

    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> bypassAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> oversampleAttachment;

    juce::Image brandMarkImage;          // cached radial gradient render (v2 spec)
    void renderBrandMark();              // builds the image once at construction

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(HeaderBar)
};
