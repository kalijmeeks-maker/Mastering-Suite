#include "RotaryKnobImage.h"
#include <algorithm>

RotaryKnobImage::RotaryKnobImage(const juce::Image& atlasImage)
    : atlasImage(atlasImage)
{
    setSliderStyle(juce::Slider::RotaryVerticalDrag);
    setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    setSize(256, 256);
}

void RotaryKnobImage::paint(juce::Graphics& g)
{
    if (atlasImage.isNull())
        return;

    auto normalisedValue = (getValue() - getMinimum()) / (getMaximum() - getMinimum());
    normalisedValue = std::clamp(static_cast<float>(normalisedValue), 0.0f, 1.0f);

    int frameIndex = static_cast<int>(normalisedValue * (NUM_FRAMES - 1));
    frameIndex = std::clamp(frameIndex, 0, (NUM_FRAMES - 1));

    int gridX = (frameIndex % GRID_SIZE) * FRAME_SIZE;
    int gridY = (frameIndex / GRID_SIZE) * FRAME_SIZE;

    juce::Rectangle<int> sourceArea(gridX, gridY, FRAME_SIZE, FRAME_SIZE);
    auto frameImage = atlasImage.getClippedImage(sourceArea);

    g.drawImageAt(frameImage, 0, 0);
}
