#include "LufsPanel.h"
#include "../PluginProcessor.h"

LufsPanel::LufsPanel(MasteringSuiteProcessor& proc) : processor(proc) {}

void LufsPanel::paint(juce::Graphics& g) {
    auto bounds = getLocalBounds().toFloat();

    // Background gradient
    juce::ColourGradient bg(
        juce::Colour(mst::theme::panelTop), 0.0f, 0.0f,
        juce::Colour(mst::theme::panelInner), 0.0f, bounds.getHeight(),
        false);
    g.setGradientFill(bg);
    g.fillRoundedRectangle(bounds, 8.0f);

    // Border
    g.setColour(juce::Colour(mst::theme::border).withAlpha(0.3f));
    g.drawRoundedRectangle(bounds, 8.0f, 1.0f);

    // Panel header "LOUDNESS · LUFS" with subtext
    g.setFont(juce::Font(11.0f).boldened());
    g.setColour(juce::Colour(mst::theme::textHigh));
    g.drawText(juce::String(juce::CharPointer_UTF8(u8"LOUDNESS · LUFS")), 14, 8, bounds.getWidth() - 28, 14, juce::Justification::topLeft);

    g.setFont(juce::Font(8.0f));
    g.setColour(juce::Colour(mst::theme::textLow));
    g.drawText(juce::String(juce::CharPointer_UTF8(u8"EBU R128 · 400 ms")), 14, 20, bounds.getWidth() - 28, 10, juce::Justification::topLeft);

    // Giant LUFS number (center-left)
    g.setFont(juce::Font(48.0f).boldened());
    g.setColour(juce::Colour(mst::theme::cEqGain)); // Cyan
    g.drawText("-14.2", 14, 40, (bounds.getWidth() - 28) / 2, bounds.getHeight() - 50, juce::Justification::centredLeft);

    // Secondary readouts (right side)
    g.setFont(juce::Font(9.0f));
    g.setColour(juce::Colour(mst::theme::textMid));
    auto rightX = bounds.getWidth() / 2 + 14;
    g.drawText("Short: -15.1", rightX, 50, bounds.getWidth() / 2 - 28, 12, juce::Justification::topLeft);
    g.drawText("Momentary: -12.8", rightX, 65, bounds.getWidth() / 2 - 28, 12, juce::Justification::topLeft);
    g.drawText("Range: 8.2", rightX, 80, bounds.getWidth() / 2 - 28, 12, juce::Justification::topLeft);
    g.drawText("True Peak: -0.8", rightX, 95, bounds.getWidth() / 2 - 28, 12, juce::Justification::topLeft);
}

void LufsPanel::resized() {}

void LufsPanel::refresh() {
    repaint();
}
