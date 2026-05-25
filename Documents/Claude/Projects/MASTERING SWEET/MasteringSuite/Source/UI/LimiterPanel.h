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

    // GR meter motion state (v1.0.2 §2 — Design polish direction).
    //   Pulse: single-shot rising-edge fire when GR crosses 3 dB instant,
    //          100 ms debounce, 240 ms ease-out, amber→red→amber, scaleY 1.0→1.025.
    //   Throb: continuous after GR > 3 dB sustained 500 ms,
    //          1.2 s sin cycle, +60% glow intensity, 250 ms hysteresis on release.
    //   Throb wins; pulses are suppressed while throbbing.
    void tickGrAnimation();
    float  prevGr            = 0.0f;
    bool   isPulsing         = false;
    bool   isThrobbing       = false;
    double pulseStartMs      = 0.0;
    double lastPulseMs       = 0.0;
    double sustainStartMs    = 0.0;  // when did GR first cross threshold this run
    double sustainReleaseMs  = 0.0;  // when did GR last drop below threshold (for hysteresis)
    double throbRefMs        = 0.0;  // throb sin phase reference

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LimiterPanel)
};
