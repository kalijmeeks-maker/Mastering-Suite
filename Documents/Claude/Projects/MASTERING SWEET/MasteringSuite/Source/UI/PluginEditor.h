#pragma once

#include <JuceHeader.h>
#include "Theme.h"
#include "NeonLookAndFeel.h"
#include "HeaderBar.h"
#include "FooterBar.h"
#include "LufsPanel.h"
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

    MasteringSuiteProcessor& processor;
    NeonLookAndFeel laf;
    juce::TooltipWindow tooltips { this, 500 };
    std::vector<juce::Slider*> trackedSliders;

    std::unique_ptr<HeaderBar> header;
    std::unique_ptr<FooterBar> footer;
    std::unique_ptr<LufsPanel> lufsPanel;
    std::unique_ptr<LoudnessGraph> graphPanel;
    
    std::unique_ptr<TabBarComponent> tabBar;
    
    std::unique_ptr<EqPanel> eqPanel;
    std::unique_ptr<DynamicsPanel> dynPanel;
    std::unique_ptr<ImagerPanel> imgPanel;
    std::unique_ptr<LimiterPanel> limPanel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PluginEditor)
};
