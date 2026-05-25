#pragma once
#include <JuceHeader.h>
#include "Theme.h"
#include "NeonLookAndFeel.h"
#include "PillButton.h"

class MasteringSuiteProcessor;

class ImagerPanel : public juce::Component {
public:
    explicit ImagerPanel(MasteringSuiteProcessor& proc, NeonLookAndFeel& laf);
    ~ImagerPanel() override = default;

    void paint(juce::Graphics& g) override;
    void resized() override;
    void refresh();

private:
    MasteringSuiteProcessor& processor;
    NeonLookAndFeel& lookAndFeel;

    void drawGoniometer(juce::Graphics& g, juce::Rectangle<float> area);

    std::unique_ptr<juce::Slider> widthK, panK;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> widthA, panA;
    std::unique_ptr<PillButton> scaleToggle;  // AUTO / -20 dB toggle below goniometer

    std::deque<juce::Point<float>> scopePoints;
    static constexpr int maxPoints = 512;
    juce::Rectangle<float> lastGonioArea;     // tracked so we can lay out the toggle relative to it

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ImagerPanel)
};
