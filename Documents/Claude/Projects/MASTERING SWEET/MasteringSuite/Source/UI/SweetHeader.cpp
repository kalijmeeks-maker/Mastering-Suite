#include "SweetHeader.h"

SweetHeader::SweetHeader()
{
}

void SweetHeader::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    
    g.setColour(Theme::Color::toColour(Theme::Color::BG_1));
    g.fillRect(bounds);
    g.setColour(Theme::Color::toColour(Theme::Color::LINE));
    g.drawHorizontalLine((int)bounds.getBottom() - 1, bounds.getX(), bounds.getRight());
    
    // Left: Logo
    g.setColour(Theme::Color::toColour(Theme::Color::TEXT_BRIGHT));
    g.setFont(Theme::Font::make(18.0f, Theme::Font::WEIGHT_BOLD));
    g.drawText("SWEET", juce::Rectangle<float>(10, 0, 200, bounds.getHeight()), juce::Justification::centredLeft);
    
    g.setColour(Theme::Color::toColour(Theme::Color::TEXT_DIM));
    g.setFont(Theme::Font::make(10.0f));
    g.drawText("MASTERING SUITE", juce::Rectangle<float>(10, 18, 200, bounds.getHeight()), juce::Justification::centredLeft);
}
