#include "BottomStrip.h"

BottomStrip::BottomStrip(juce::AudioProcessorValueTreeState& apvts_) : apvts(apvts_)
{
    inGainKnob = std::make_unique<RotaryKnob>();
    outGainKnob = std::make_unique<RotaryKnob>();
    inGainKnob->setName("IN");
    outGainKnob->setName("OUT");
    addAndMakeVisible(*inGainKnob);
    addAndMakeVisible(*outGainKnob);
}

void BottomStrip::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    
    g.setColour(Theme::Color::toColour(Theme::Color::BG_1));
    g.fillRect(bounds);
    g.setColour(Theme::Color::toColour(Theme::Color::LINE));
    g.drawHorizontalLine(0, bounds.getX(), bounds.getRight());
    
    auto leftSection = bounds.removeFromLeft(bounds.getWidth() * 0.2f);
    g.setColour(Theme::Color::toColour(Theme::Color::TEXT_DIM));
    g.setFont(Theme::Font::make(9.0f));
    g.drawText("GAIN STAGING", leftSection.removeFromTop(14), juce::Justification::centredLeft);
    
    auto centerSection = bounds.removeFromLeft(bounds.getWidth() * 50.0f / 80.0f);
    g.setColour(Theme::Color::toColour(Theme::Color::TEXT_BRIGHT));
    g.setFont(Theme::Font::make(12.0f, Theme::Font::WEIGHT_BOLD));
    g.drawText(currentParamName, centerSection.removeFromTop(40), juce::Justification::centredLeft);
    
    g.setColour(Theme::Color::toColour(Theme::Color::TEXT_DIM));
    g.setFont(Theme::Font::makeMono(10.0f));
    g.drawText(currentParamValue, centerSection, juce::Justification::centredLeft);
    
    auto rightSection = bounds;
    g.setColour(Theme::Color::toColour(Theme::Color::BG_2));
    g.drawRect(rightSection.reduced(8), 1.0f);
    
    g.setColour(Theme::Color::toColour(Theme::Color::TEXT_DIM));
    g.setFont(Theme::Font::make(9.0f));
    g.drawText("LOUDNESS", rightSection.reduced(12).removeFromTop(12), juce::Justification::topLeft);
    
    g.setColour(Theme::Color::toColour(Theme::Color::ACCENT));
    g.setFont(Theme::Font::makeMono(13.0f));
    g.drawText(juce::String::formatted("–%.1f", std::abs(lufsIntegrated)), 
               rightSection.removeFromLeft(50), juce::Justification::centredLeft);
}

void BottomStrip::resized()
{
    auto bounds = getLocalBounds();
    auto leftBounds = bounds.removeFromLeft(bounds.getWidth() * 20 / 100);
    leftBounds.removeFromTop(14);
    
    inGainKnob->setBounds(leftBounds.removeFromLeft(50).reduced(2));
    outGainKnob->setBounds(leftBounds.removeFromLeft(50).reduced(2));
}

void BottomStrip::setMeterValues(float lufsI, float lufsS, float lufsM, float truePeak)
{
    lufsIntegrated = lufsI;
    lufsShort = lufsS;
    lufsMax = lufsM;
    truePeakValue = truePeak;
    repaint();
}

void BottomStrip::setCurrentParameter(const juce::String& name, const juce::String& value)
{
    currentParamName = name;
    currentParamValue = value;
    repaint();
}
