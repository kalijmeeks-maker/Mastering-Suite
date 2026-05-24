#include "HeaderBar.h"
#include "../PluginProcessor.h"

HeaderBar::HeaderBar(MasteringSuiteProcessor& proc) : processor(proc) {}

void HeaderBar::paint(juce::Graphics& g) {
    auto bounds = getLocalBounds().toFloat();

    // Background gradient
    juce::ColourGradient bg(
        juce::Colour(mst::theme::panelTop), 0.0f, 0.0f,
        juce::Colour(mst::theme::bgBase), 0.0f, bounds.getHeight(),
        false);
    g.setGradientFill(bg);
    g.fillAll();

    // Bottom border
    g.setColour(juce::Colour(mst::theme::border));
    g.drawHorizontalLine((int)bounds.getBottom() - 1, 0.0f, bounds.getWidth());

    // Brand mark: 28×28 magenta-violet rounded square at top-left
    auto markX = 14.0f;
    auto markY = (bounds.getHeight() - 28.0f) * 0.5f;
    juce::ColourGradient markGrad(
        juce::Colour(0xFFBB00FF), markX, markY,
        juce::Colour(0xFFFF00FF), markX + 28.0f, markY + 28.0f,
        true);
    g.setGradientFill(markGrad);
    g.fillRoundedRectangle(markX, markY, 28.0f, 28.0f, 4.0f);

    // Wordmark "MASTERING SWEET"
    g.setFont(juce::Font(14.0f).boldened());
    g.setColour(juce::Colour(mst::theme::textHigh));
    g.drawText("MASTERING ", markX + 42.0f, 0, 150, (int)bounds.getHeight(), juce::Justification::centredLeft);

    g.setColour(juce::Colour(mst::theme::cEqFreq)); // Magenta for "SWEET"
    g.drawText("SWEET", markX + 152.0f, 0, 100, (int)bounds.getHeight(), juce::Justification::centredLeft);

    // Tag text "v1.0 · STEREO · 48 kHz"
    g.setFont(juce::Font(9.0f));
    g.setColour(juce::Colour(mst::theme::textLow));
    g.drawText("v1.0 - STEREO - 48 kHz", markX + 250.0f, 0, 150, (int)bounds.getHeight(), juce::Justification::centredLeft);
}

void HeaderBar::resized() {}
