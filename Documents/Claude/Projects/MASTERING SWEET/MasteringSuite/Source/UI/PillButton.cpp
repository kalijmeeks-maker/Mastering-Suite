#include "PillButton.h"

PillButton::PillButton() : juce::ToggleButton()
{
}

void PillButton::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    auto radius = isFirstInGroup || isLastInGroup ? 12.0f : 4.0f;
    
    if (getToggleState()) {
        g.setColour(Theme::Color::toColour(Theme::Color::ACCENT));
        g.fillRoundedRectangle(bounds, radius);
        g.setColour(Theme::Color::toColour(Theme::Color::BG_0));
    } else {
        g.setColour(Theme::Color::toColour(Theme::Color::BG_2));
        g.fillRoundedRectangle(bounds, radius);
        g.setColour(Theme::Color::toColour(Theme::Color::LINE));
        g.drawRoundedRectangle(bounds, radius, 1.0f);
        g.setColour(Theme::Color::toColour(Theme::Color::TEXT_DIM));
    }
    
    g.setFont(Theme::Font::make(Theme::Font::SIZE_LABEL, Theme::Font::WEIGHT_BOLD));
    g.drawFittedText(getButtonText(), bounds.toNearestInt(), juce::Justification::centred, 1);
}

void PillButton::setSegmentGroup(bool isFirst, bool isLast)
{
    isFirstInGroup = isFirst;
    isLastInGroup = isLast;
}
