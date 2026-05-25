#pragma once
#include <JuceHeader.h>
#include "Theme.h"

#include "SegmentedControl.h"

class MasteringSuiteProcessor;

class FooterBar : public juce::Component {
public:
    explicit FooterBar(MasteringSuiteProcessor& proc);
    ~FooterBar() override = default;

    void paint(juce::Graphics& g) override;
    void resized() override;
    void refresh();

    // v1.0.2 §3 — EQ Drag Toast. Engineer is dragging a knob / EQ handle;
    // show a context-aware status line in the footer center (covering the
    // density toggle). Color is the module accent. mouseUp triggers a
    // 300 ms ease-out fade; idle-state toggle crossfades back over 200 ms
    // starting 150 ms into the toast fade.
    void setToast(juce::String text, juce::Colour accent);
    void clearToast();

private:
    MasteringSuiteProcessor& processor;
    std::unique_ptr<SegmentedControl> densityToggle;

    // Toast state. Timestamps are juce::Time::getMillisecondCounterHiRes()
    // so we can interpolate cleanly between paint frames.
    juce::String toastText;
    juce::Colour toastColor { 0xFF00E5FF };  // sensible cyan default
    bool   toastActive       = false;
    double toastFadeStartMs  = 0.0;          // when mouseUp fired (start of fade-out)

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FooterBar)
};
