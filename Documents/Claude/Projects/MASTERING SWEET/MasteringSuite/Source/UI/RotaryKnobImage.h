#pragma once
#include <JuceHeader.h>

class RotaryKnobImage : public juce::Slider {
public:
    RotaryKnobImage(const juce::Image& atlasImage);
    ~RotaryKnobImage() override = default;

    void paint(juce::Graphics& g) override;

private:
    juce::Image atlasImage;

    static constexpr int ATLAS_SIZE = 4096;
    static constexpr int FRAME_SIZE = 256;
    static constexpr int GRID_SIZE = 16;
    static constexpr int NUM_FRAMES = 256;
};
