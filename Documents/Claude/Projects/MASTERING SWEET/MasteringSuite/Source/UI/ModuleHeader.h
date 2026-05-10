#pragma once
#include <JuceHeader.h>
#include "Theme.h"

class ModuleHeader : public juce::Component {
public:
    ModuleHeader();
    ~ModuleHeader() override = default;

    void paint(juce::Graphics& g) override;
    void setTitle(const juce::String& name, const juce::String& subtitle);
    void setModuleEnabled(bool enabled) { isEnabled = enabled; repaint(); }

private:
    juce::String moduleName = "DYNAMICS";
    juce::String moduleSubtitle = "Compression & Expansion";
    bool isEnabled = true;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ModuleHeader)
};
