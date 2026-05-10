#pragma once
#include <JuceHeader.h>
#include "Colors.h"

class MasteringSuiteLookAndFeel : public juce::LookAndFeel_V4
{
public:
    MasteringSuiteLookAndFeel();

    void drawButtonBackground (juce::Graphics&, juce::Button&, const juce::Colour& backgroundColour,
                              bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override;

    void drawComboBox (juce::Graphics&, int width, int height, bool isButtonDown,
                       int buttonX, int buttonY, int buttonW, int buttonH, juce::ComboBox& box) override;

    void drawLabel (juce::Graphics&, juce::Label&) override;

    void drawLinearSlider (juce::Graphics&, int x, int y, int width, int height,
                          float sliderPos, float minSliderPos, float maxSliderPos,
                          const juce::Slider::SliderStyle style, juce::Slider&) override;

    void drawRotarySlider (juce::Graphics&, int x, int y, int width, int height,
                          float sliderPosProportional, float rotaryStartAngle, float rotaryEndAngle,
                          juce::Slider&) override;
};
