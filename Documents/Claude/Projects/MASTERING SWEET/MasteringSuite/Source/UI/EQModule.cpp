#include "EQModule.h"

EQModule::EQModule(juce::AudioProcessorValueTreeState& apvtsRef) : apvts(apvtsRef)
{
    for (int i = 0; i < BAND_COUNT; ++i) {
        // Type combo box
        bands[i].typeBox = std::make_unique<juce::ComboBox>();
        bands[i].typeBox->addItem("Bypass", 1);
        bands[i].typeBox->addItem("H-Shelf", 2);
        bands[i].typeBox->addItem("L-Shelf", 3);
        bands[i].typeBox->addItem("Peaking", 4);
        juce::String typeId = juce::String(bandNames[i]) + "Type";
        bands[i].typeBox->setSelectedItemIndex((int)*apvts.getRawParameterValue(typeId.toStdString().c_str()), juce::dontSendNotification);
        addAndMakeVisible(*bands[i].typeBox);

        // Frequency knob
        bands[i].freqKnob = std::make_unique<RotaryKnob>();
        juce::String freqId = juce::String(bandNames[i]) + "Freq";
        bands[i].freqAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            apvts, freqId.toStdString(), *bands[i].freqKnob
        );
        addAndMakeVisible(*bands[i].freqKnob);

        // Gain knob
        bands[i].gainKnob = std::make_unique<RotaryKnob>();
        juce::String gainId = juce::String(bandNames[i]) + "Gain";
        bands[i].gainAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            apvts, gainId.toStdString(), *bands[i].gainKnob
        );
        addAndMakeVisible(*bands[i].gainKnob);

        // Q knob
        bands[i].qKnob = std::make_unique<RotaryKnob>();
        juce::String qId = juce::String(bandNames[i]) + "Q";
        bands[i].qAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            apvts, qId.toStdString(), *bands[i].qKnob
        );
        addAndMakeVisible(*bands[i].qKnob);
    }
}

void EQModule::paint(juce::Graphics& g)
{
    g.fillAll(Theme::Color::toColour(Theme::Color::BG_1));

    auto bounds = getLocalBounds().toFloat();
    int bandWidth = bounds.getWidth() / BAND_COUNT;

    for (int i = 0; i < BAND_COUNT; ++i) {
        auto bandBounds = bounds.withLeft(bounds.getX() + i * bandWidth).withRight(bounds.getX() + (i + 1) * bandWidth);

        // Band label
        g.setColour(Theme::Color::toColour(bandColors[i]));
        g.setFont(Theme::Font::make(Theme::Font::SIZE_MICRO, Theme::Font::WEIGHT_BOLD));
        g.drawText(bandLabels[i], bandBounds.removeFromTop(20), juce::Justification::centred);

        // Band border
        g.setColour(Theme::Color::toColour(Theme::Color::LINE_2));
        if (i < BAND_COUNT - 1) {
            float x = bandBounds.getRight();
            g.drawLine(x, bandBounds.getY(), x, bandBounds.getBottom(), 1.0f);
        }
    }
}

void EQModule::resized()
{
    auto bounds = getLocalBounds();
    int bandWidth = bounds.getWidth() / BAND_COUNT;
    int labelHeight = 20;
    int spacing = 10;

    for (int i = 0; i < BAND_COUNT; ++i) {
        auto bandBounds = bounds.withLeft(i * bandWidth).withRight((i + 1) * bandWidth);
        bandBounds = bandBounds.withTop(bandBounds.getY() + labelHeight);

        auto contentBounds = bandBounds.reduced(spacing);
        int knobSize = Theme::Layout::KNOB_SIZE_SMALL;
        int knobGap = (contentBounds.getHeight() - (knobSize * 3)) / 4;

        // Type box
        bands[i].typeBox->setBounds(contentBounds.removeFromTop(knobGap + 20));

        // Frequency knob
        bands[i].freqKnob->setBounds(contentBounds.removeFromTop(knobSize + knobGap).removeFromLeft(knobSize).translated(spacing, 0));

        // Gain knob
        bands[i].gainKnob->setBounds(contentBounds.removeFromTop(knobSize + knobGap).removeFromLeft(knobSize).translated(spacing, 0));

        // Q knob
        bands[i].qKnob->setBounds(contentBounds.removeFromTop(knobSize + knobGap).removeFromLeft(knobSize).translated(spacing, 0));
    }
}
