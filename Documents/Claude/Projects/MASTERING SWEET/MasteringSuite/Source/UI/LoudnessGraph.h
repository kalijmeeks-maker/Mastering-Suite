#pragma once
#include <JuceHeader.h>
#include "Theme.h"
#include <deque>

class MasteringSuiteProcessor;

// v1.0.4 Loudness Row Restack: this single component replaces the old
// LufsPanel + LoudnessGraph side-by-side pair. It draws everything in the
// unified bottom row per "Mastering Sweet v1.0.4 Layout Restack Direction":
//
//   ┌──Title row (28 px)──────────────────────────────────────┐
//   │ LOUDNESS HISTORY        SHORT · MOM · RANGE · TP chips  │
//   ├──History zone (90 px)──┬─Y-axis─┬─L/R per-channel meter ┤
//   │  magenta bars          │ dB     │ shipped 5-color ramp  │
//   │  ─ ─ ─ −14 ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ (single dashed line) │
//   ├──Hero zone (90 px)─────┘        │                       │
//   │  −14.2  LUFS                    │ (continues from hist) │
//   │  INTEGRATED                     │                       │
//   └─────────────────────────────────┴───────────────────────┘
//
// Class name kept as LoudnessGraph to avoid .jucer / .xcodeproj edits.
// A rename to LoudnessPanel is a follow-up cleanup task.
class LoudnessGraph : public juce::Component {
public:
    explicit LoudnessGraph(MasteringSuiteProcessor& proc);
    ~LoudnessGraph() override = default;

    void paint(juce::Graphics& g) override;
    void resized() override;
    void pushSample(float lufsValue);
    void refresh() { repaint(); }   // alias so editor can call uniformly

private:
    MasteringSuiteProcessor& processor;

    // Short-term LUFS ring used for the magenta history bars (top zone).
    // 45 bars per v1.0.4 lock (was 240 in v1.0.x).
    std::deque<float> history;
    static constexpr int maxSamples = 45;

    // L/R per-channel peak-hold state — migrated from LufsPanel.
    // Hold 3 s, then decay 30 dB / 2 s linear (same ballistics as shipped).
    float  peakHoldL     = -100.0f;
    float  peakHoldR     = -100.0f;
    double peakHoldTimeL = 0.0;
    double peakHoldTimeR = 0.0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LoudnessGraph)
};
