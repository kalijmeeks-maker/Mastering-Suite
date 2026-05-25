#pragma once
#include <JuceHeader.h>
#include "Theme.h"
#include "NeonLookAndFeel.h"

class MasteringSuiteProcessor;

class LimiterPanel : public juce::Component {
public:
    explicit LimiterPanel(MasteringSuiteProcessor& proc, NeonLookAndFeel& laf);
    ~LimiterPanel() override = default;

    void paint(juce::Graphics& g) override;
    void resized() override;
    void mouseDown(const juce::MouseEvent& e) override;
    void refresh();

private:
    MasteringSuiteProcessor& processor;
    NeonLookAndFeel& lookAndFeel;

    std::unique_ptr<juce::Slider> threshK, releaseK, makeupK, ceilingK;
    std::unique_ptr<juce::ComboBox> styleC;
    std::unique_ptr<juce::ToggleButton> truePeakB;

    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> threshA, releaseA, makeupA, ceilingA;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> styleA;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> truePeakA;

    // Mode picker cells (left side, per Design v2: TRANSPARENT/PUNCHY/WARM with descriptions)
    void drawModePicker(juce::Graphics& g, juce::Rectangle<int> area);
    juce::Rectangle<int> modeArea;
    juce::Rectangle<int> meterArea;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LimiterPanel)
};
