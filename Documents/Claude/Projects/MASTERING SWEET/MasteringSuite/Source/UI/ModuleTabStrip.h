#pragma once
#include <JuceHeader.h>
#include "Theme.h"

class ModuleTabStrip : public juce::Component {
public:
    ModuleTabStrip();
    ~ModuleTabStrip() override = default;

    void paint(juce::Graphics& g) override;
    void resized() override;

    void setActiveModule(int index);
    void updateSystemInfo(double sampleRate, int bufferSize);
    std::function<void(int)> onModuleSelected;

private:
    int activeModule = 1;
    double sampleRate = 48000;
    int bufferSize = 256;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ModuleTabStrip)
};
