#include "HeaderBar.h"
#include "../PluginProcessor.h"
#include "PillButton.cpp"
#include "SegmentedControl.cpp"

HeaderBar::HeaderBar(MasteringSuiteProcessor& proc) : processor(proc) {
    // A/B Toggle — magenta fill on active segment only
    abToggle.setVariant(PillButton::Variant::Filled);
    abToggle.setAccentColor(juce::Colour(mst::theme::cEqFreq));  // Magenta
    abToggle.addButton("A");
    abToggle.addButton("B");
    abToggle.setSelectedIndex(0);
    addAndMakeVisible(abToggle);

    // Preset Button — neutral pill
    presetButton.setButtonText("PRESET: DEFAULT");
    addAndMakeVisible(presetButton);

    // Bypass Button — amber outlined border when active
    bypassButton.setButtonText("BYPASS");
    bypassButton.setVariant(PillButton::Variant::Outlined);
    bypassButton.setAccentColor(juce::Colour(0xFFFF9500));  // Amber
    addAndMakeVisible(bypassButton);
    bypassAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        processor.getAPVTS(), "bypass", bypassButton);

    // Oversample Button — cyan outlined border + text when ON (i.e., not 1x)
    oversampleButton.setVariant(PillButton::Variant::Outlined);
    oversampleButton.setAccentColor(juce::Colour(0xFF6CD6FF));  // Cyan
    addAndMakeVisible(oversampleButton);

    auto refreshOversampleLabel = [this] {
        if (auto* param = dynamic_cast<juce::AudioParameterChoice*>(processor.getAPVTS().getParameter("oversample"))) {
            oversampleButton.setButtonText(param->choices[param->getIndex()].toUpperCase() + " OVERSAMPLE");
            // ON state = any setting other than 1x (index 0)
            oversampleButton.setToggleState(param->getIndex() > 0, juce::dontSendNotification);
        }
    };

    oversampleButton.onClick = [this, refreshOversampleLabel] {
        if (auto* param = dynamic_cast<juce::AudioParameterChoice*>(processor.getAPVTS().getParameter("oversample"))) {
            int nextIndex = (param->getIndex() + 1) % param->choices.size();
            param->setValueNotifyingHost(param->getNormalisableRange().convertTo0to1((float)nextIndex));
            refreshOversampleLabel();
        }
    };

    refreshOversampleLabel();
}

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

    // Brand mark: 28×28 radial-gradient rounded square at top-left
    auto markX = 14.0f;
    auto markY = (bounds.getHeight() - 28.0f) * 0.5f;
    auto markRect = juce::Rectangle<float>(markX, markY, 28.0f, 28.0f);
    
    juce::ColourGradient markGrad(
        juce::Colour(0xFFFF00FF), markRect.getCentreX(), markRect.getCentreY(),
        juce::Colour(0xFF880088), markRect.getX(), markRect.getY(),
        true);
    g.setGradientFill(markGrad);
    g.fillRoundedRectangle(markRect, 6.0f);

    // Wordmark "MASTERING SWEET"
    g.setFont(juce::Font(14.0f).boldened());
    g.setColour(juce::Colour(mst::theme::textHigh));
    g.drawText("MASTERING ", markX + 42.0f, 0, 150, (int)bounds.getHeight(), juce::Justification::centredLeft);

    g.setColour(juce::Colour(mst::theme::cEqFreq)); // Magenta for "SWEET"
    g.drawText("SWEET", markX + 152.0f, 0, 100, (int)bounds.getHeight(), juce::Justification::centredLeft);

    // Tag text "v1.0 · STEREO · 48 kHz"
    g.setFont(juce::Font(9.0f));
    g.setColour(juce::Colour(mst::theme::textLow));
    g.drawText(juce::String::fromUTF8("v1.0 \u00B7 STEREO \u00B7 48 kHz"), (int)markX + 250, 0, 150, (int)bounds.getHeight(), juce::Justification::centredLeft);
}

void HeaderBar::resized() {
    auto r = getLocalBounds().reduced(14, 0);
    auto rightSide = r.removeFromRight(450);
    
    // Position from right to left
    oversampleButton.setBounds(rightSide.removeFromRight(120).reduced(0, 14));
    rightSide.removeFromRight(8); // gap
    bypassButton.setBounds(rightSide.removeFromRight(80).reduced(0, 14));
    rightSide.removeFromRight(8); // gap
    presetButton.setBounds(rightSide.removeFromRight(140).reduced(0, 14));
    rightSide.removeFromRight(16); // larger gap
    abToggle.setBounds(rightSide.removeFromRight(60).reduced(0, 14));
}
