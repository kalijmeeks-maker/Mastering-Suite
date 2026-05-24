#include "FooterBar.h"
#include "../PluginProcessor.h"

FooterBar::FooterBar(MasteringSuiteProcessor& proc) : processor(proc) {}

void FooterBar::paint(juce::Graphics& g) {
    auto bounds = getLocalBounds().toFloat();

    // Background
    g.setColour(juce::Colour(mst::theme::bgBase));
    g.fillRect(bounds);

    // Top border
    g.setColour(juce::Colour(mst::theme::border));
    g.drawHorizontalLine(0, 0.0f, bounds.getWidth());

    // Footer left: IN, OUT, SR, BUF readouts
    g.setFont(juce::Font(9.0f));
    g.setColour(juce::Colour(mst::theme::textMid));
    g.drawText(juce::String(juce::CharPointer_UTF8(u8"IN · OUT · SR 48K · BUF 256")), 14, 0, 200, (int)bounds.getHeight(), juce::Justification::centredLeft);

    // Footer right: green dot + DSP %, CPU %, version
    g.setColour(juce::Colour(mst::theme::statusGreen));
    g.fillEllipse(bounds.getRight() - 180.0f, bounds.getCentreY() - 3.0f, 6.0f, 6.0f);

    g.setFont(juce::Font(9.0f));
    g.setColour(juce::Colour(mst::theme::textMid));
    g.drawText(juce::String(juce::CharPointer_UTF8(u8"DSP 1.2% · CPU 0.8% · v1.0")), bounds.getRight() - 170.0f, 0, 160, (int)bounds.getHeight(), juce::Justification::centredLeft);
}

void FooterBar::resized() {}

void FooterBar::refresh() {
    repaint();
}
