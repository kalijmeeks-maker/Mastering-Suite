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

    // Goniometer persistence-of-vision trail ring. Each entry holds a stereo
    // pair (x=L, y=R) — semantically a (l,r) sample, stored as Point<float>
    // because juce::deque<Point> avoids a custom type. Sized so it drains in
    // ~800 ms at 30 Hz repaint × ~25 decimation (per Design v1.0.2 §1).
    std::deque<juce::Point<float>> scopePoints;
    static constexpr int maxPoints = 960;
    juce::Rectangle<float> lastGonioArea;     // tracked so we can lay out the toggle relative to it
    float rmsSmoothed = -1.0f;                // EMA-smoothed RMS for goniometer auto-scale (-1 = uninit, snaps on first frame)

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ImagerPanel)
};
