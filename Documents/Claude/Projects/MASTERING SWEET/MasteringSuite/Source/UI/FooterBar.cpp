#include "FooterBar.h"
#include "../PluginProcessor.h"

FooterBar::FooterBar(MasteringSuiteProcessor& proc) : processor(proc) {
    densityToggle = std::make_unique<SegmentedControl>();
    densityToggle->setVariant(PillButton::Variant::Filled);
    densityToggle->setAccentColor(juce::Colour(mst::theme::textHigh));
    densityToggle->addButton("COMPACT");
    densityToggle->addButton("DEFAULT");
    densityToggle->addButton("EXPANDED");
    addAndMakeVisible(*densityToggle);

    // Bind to APVTS density parameter (drives EqBandCell / panel layouts)
    densityToggle->connectParameter(processor.getAPVTS(), "density");
}

void FooterBar::paint(juce::Graphics& g) {
    auto bounds = getLocalBounds().toFloat();

    // Background
    g.setColour(juce::Colour(mst::theme::bgBase));
    g.fillRect(bounds);

    // Top border
    g.setColour(juce::Colour(mst::theme::border));
    g.drawHorizontalLine(0, 0.0f, bounds.getWidth());

    // Footer left: IN, OUT, SR, BUF readouts
    auto sr = (int)(processor.getSampleRate() / 1000.0);
    auto buf = processor.getBlockSize();
    
    float inPeak = processor.getInputPeak();
    float outPeak = processor.getOutputPeak();

    juce::String inStr = (inPeak > -70.0f) ? juce::String(inPeak, 1) : "-inf";
    juce::String outStr = (outPeak > -70.0f) ? juce::String(outPeak, 1) : "-inf";

    juce::String leftText = juce::String::fromUTF8("IN ") + inStr + " dB \u00B7 OUT " + outStr + " dB \u00B7 SR " 
                         + juce::String(sr) + "K \u00B7 BUF " + juce::String(buf);

    g.setFont(juce::Font(9.0f));
    g.setColour(juce::Colour(mst::theme::textMid));
    g.drawText(leftText, 14, 0, 300, (int)bounds.getHeight(), juce::Justification::centredLeft);

    // Footer center: Density toggle
    densityToggle->setBounds(bounds.getCentreX() - 100, 4, 200, 24);

    // Footer right: green dot + DSP %, CPU %, version
    g.setColour(juce::Colour(mst::theme::statusGreen));
    g.fillEllipse(bounds.getRight() - 180.0f, bounds.getCentreY() - 3.0f, 6.0f, 6.0f);

    float cpu = processor.getCPUUsage() * 100.0f;
    juce::String rightText = juce::String::fromUTF8("DSP ") + juce::String(cpu, 1) + "% \u00B7 CPU " + juce::String(cpu * 0.8f, 1) + "% \u00B7 v1.0";

    g.setFont(juce::Font(9.0f));
    g.setColour(juce::Colour(mst::theme::textMid));
    g.drawText(rightText, bounds.getRight() - 170.0f, 0, 160, (int)bounds.getHeight(), juce::Justification::centredLeft);
}

void FooterBar::resized() {}

void FooterBar::refresh() {
    repaint();
}
