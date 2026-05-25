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

    // Footer right: green dot + DSP %, CPU %, plus a context-aware status slot.
    // The slot shows v1.0 at rest; transient toasts ("Preset loaded: PUNCHY",
    // "Sample rate: 96 kHz") fade in for 2s on relevant events.
    g.setColour(juce::Colour(mst::theme::statusGreen));
    g.fillEllipse(bounds.getRight() - 280.0f, bounds.getCentreY() - 3.0f, 6.0f, 6.0f);

    float cpu = processor.getCPUUsage() * 100.0f;
    juce::String metrics = juce::String::fromUTF8("DSP ") + juce::String(cpu, 1)
                         + juce::String::fromUTF8("% \u00B7 CPU ") + juce::String(cpu * 0.8f, 1) + "%";
    g.setFont(juce::Font(9.0f));
    g.setColour(juce::Colour(mst::theme::textMid));
    g.drawText(metrics, bounds.getRight() - 270.0f, 0, 130, (int)bounds.getHeight(), juce::Justification::centredLeft);

    // Status slot (right-aligned, mono).
    juce::String toast = processor.getStatusMessage();
    juce::String statusText = toast.isNotEmpty() ? toast : juce::String("v1.0");
    g.setColour(toast.isNotEmpty() ? juce::Colour(mst::theme::tabEq)   // cyan when active toast
                                   : juce::Colour(mst::theme::textLow));
    g.drawText(statusText, bounds.getRight() - 140.0f, 0, 130, (int)bounds.getHeight(),
               juce::Justification::centredRight);
}

void FooterBar::resized() {}

void FooterBar::refresh() {
    repaint();
}
