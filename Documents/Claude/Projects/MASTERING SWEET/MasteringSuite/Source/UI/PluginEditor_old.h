#pragma once

#include <juce_gui_extra/juce_gui_extra.h>
#include "Theme.h"
#include "NeonLookAndFeel.h"
#include "SweetHeader.h"
#include "ModuleTabStrip.h"
#include "ModuleHeader.h"
#include "DynamicsModule.h"
#include "BottomStrip.h"
#include "FooterBar.h"

class MasteringSuiteProcessor;

class MasteringSuiteEditor : public juce::AudioProcessorEditor,
                             private juce::Timer
{
public:
    MasteringSuiteEditor(MasteringSuiteProcessor&);
    ~MasteringSuiteEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;
    void showModule(int moduleIndex);

    MasteringSuiteProcessor& processor;
    NeonLookAndFeel lookAndFeel;

    // 5-zone UI layout
    std::unique_ptr<SweetHeader> sweetHeader;
    std::unique_ptr<ModuleTabStrip> moduleTabStrip;
    std::unique_ptr<ModuleHeader> moduleHeader;
    std::unique_ptr<BottomStrip> bottomStrip;
    std::unique_ptr<FooterBar> footerBar;

    // Module content
    std::unique_ptr<DynamicsModule> dynamicsModule;

    int activeModule = 1; // Default to DYNAMICS

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MasteringSuiteEditor)
};
