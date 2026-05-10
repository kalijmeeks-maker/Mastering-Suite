#pragma once
#include <JuceHeader.h>

class RotaryKnob : public juce::Slider
{
public:
    RotaryKnob();
    ~RotaryKnob() override;

    void paint (juce::Graphics& g) override;
    void mouseEnter(const juce::MouseEvent& event) override;
    void mouseExit(const juce::MouseEvent& event) override;
    void mouseDown(const juce::MouseEvent& event) override;
    void mouseUp(const juce::MouseEvent& event) override;
    juce::String getTextFromValue(double value) override;

private:
    bool isHovered = false;
    bool isDragging = false;
    juce::String formatValue(double value) const;
};
