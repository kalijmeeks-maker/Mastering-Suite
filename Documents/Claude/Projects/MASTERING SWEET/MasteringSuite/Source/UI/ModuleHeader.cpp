#include "ModuleHeader.h"

ModuleHeader::ModuleHeader()
{
}

void ModuleHeader::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    
    g.setColour(Theme::Color::toColour(Theme::Color::BG_1));
    g.fillRect(bounds);
    g.setColour(Theme::Color::toColour(Theme::Color::LINE));
    g.drawHorizontalLine((int)bounds.getBottom() - 1, bounds.getX(), bounds.getRight());
    
    g.setColour(Theme::Color::toColour(Theme::Color::TEXT_BRIGHT));
    g.setFont(Theme::Font::make(12.0f, Theme::Font::WEIGHT_BOLD));
    g.drawText(moduleName, juce::Rectangle<float>(10, 0, 300, 22), juce::Justification::centredLeft);
    
    g.setColour(Theme::Color::toColour(Theme::Color::TEXT_DIM));
    g.setFont(Theme::Font::make(10.0f));
    g.drawText(moduleSubtitle, juce::Rectangle<float>(10, 22, 300, 22), juce::Justification::centredLeft);
}

void ModuleHeader::setTitle(const juce::String& name, const juce::String& subtitle)
{
    moduleName = name;
    moduleSubtitle = subtitle;
    repaint();
}
