#pragma once

#include <JuceHeader.h>
#include "Theme.h"
#include "NeonLookAndFeel.h"
#include "HeaderBar.h"
#include "FooterBar.h"
// LufsPanel is now superseded by the unified LoudnessGraph (v1.0.4) — file
// kept on disk for git-revert safety but no longer wired into the editor.
#include "LoudnessGraph.h"
#include "EqPanel.h"
#include "LimiterPanel.h"
#include "DynamicsPanel.h"
#include "ImagerPanel.h"
#include "TabBarComponent.h"

class MasteringSuiteProcessor;

class PluginEditor : public juce::AudioProcessorEditor,
                     private juce::Timer,
                     private juce::Slider::Listener,
                     private juce::KeyListener {
public:
    explicit PluginEditor(MasteringSuiteProcessor&);
    ~PluginEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;
    bool keyPressed(const juce::KeyPress& k, juce::Component*) override;

private:
    void timerCallback() override;
    void sliderValueChanged(juce::Slider* s) override;
    void attachKnobListeners(juce::Component* root);

    // v1.0.2 §3 — EQ Drag Toast wiring. Editor-level MouseListener is added on
    // itself with wantsChildEvents=true so any descendant Slider drag routes
    // here. EQ canvas handles aren't Sliders; EqCurveDisplay pipes its handle
    // drags through the same setToast/clearToast plumbing directly.
    void mouseDown(const juce::MouseEvent& e) override;
    void mouseDrag(const juce::MouseEvent& e) override;
    void mouseUp  (const juce::MouseEvent& e) override;
    juce::Slider* toastDragSrc      = nullptr;
    double        lastToastUpdateMs = 0.0;

    MasteringSuiteProcessor& processor;
    NeonLookAndFeel laf;
    juce::TooltipWindow tooltips { this, 800 };  // long delay; bubble is the primary affordance
    std::vector<juce::Slider*> trackedSliders;

    // Floating value bubble that appears above the currently-dragged knob.
    // Per Design's spec: 13pt SF Mono numeric + unit suffix, 2px accent stroke,
    // dark panelTop bg, 6px corner radius, hides on mouseUp + 200ms.
    struct KnobValueBubble : public juce::Component {
        juce::Slider* tracked = nullptr;
        void paint(juce::Graphics& g) override;
    };
    KnobValueBubble valueBubble;
    juce::Slider* lastDragged = nullptr;
    double lastDragTimeMs = 0.0;

    std::unique_ptr<HeaderBar> header;
    std::unique_ptr<FooterBar> footer;
    // v1.0.4 Loudness Row Restack: LufsPanel + LoudnessGraph are merged into
    // a single unified panel. lufsPanel is removed; graphPanel (kept as
    // member name for migration ease) is the new LoudnessGraph that draws
    // history bars, hero LUFS readout, L/R per-channel meters, Y-axis labels,
    // secondary chips, and the single dashed -14 target line.
    std::unique_ptr<LoudnessGraph> graphPanel;
    
    std::unique_ptr<TabBarComponent> tabBar;
    
    std::unique_ptr<EqPanel> eqPanel;
    std::unique_ptr<DynamicsPanel> dynPanel;
    std::unique_ptr<ImagerPanel> imgPanel;
    std::unique_ptr<LimiterPanel> limPanel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PluginEditor)
};
