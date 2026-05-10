#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "Theme.h"

class NeonLookAndFeel : public juce::LookAndFeel_V4 {
public:
    NeonLookAndFeel();
    ~NeonLookAndFeel() override = default;

    // Core method: draw rotary slider as a "neon knob" with arc, glow, and indicator
    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                          float sliderPos,
                          float rotaryStartAngle, float rotaryEndAngle,
                          juce::Slider& slider) override;

    // Optional: Style other components for consistency
    void drawButtonBackground(juce::Graphics& g, juce::Button& button,
                              const juce::Colour& backgroundColour,
                              bool isMouseOverButton, bool isButtonDown) override;

    void drawLinearSlider(juce::Graphics& g, int x, int y, int width, int height,
                          float sliderPos, float minSliderPos, float maxSliderPos,
                          const juce::Slider::SliderStyle style,
                          juce::Slider& slider) override;

    void drawLabel(juce::Graphics& g, juce::Label& label) override;

    // Helper: Get accent color for a component (can be overridden per-slider)
    juce::Colour getAccentColourForSlider(const juce::Slider& slider);

private:
    // Default accent color (neon green)
    juce::Colour defaultAccent = juce::Colour(Theme::Color::ACCENT);
};
