#include "PillButton.h"

PillButton::PillButton() : juce::ToggleButton()
{
}

void PillButton::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    float cornerSize = bounds.getHeight() * 0.5f;
    juce::Colour textColor;

    if (getToggleState()) {
        if (variant == Variant::Filled) {
            g.setColour(accentColor);
            g.fillRoundedRectangle(bounds, cornerSize);
            textColor = juce::Colour(mst::theme::bgBase);
        } else {  // Outlined
            g.setColour(juce::Colour(mst::theme::panelTop));
            g.fillRoundedRectangle(bounds, cornerSize);
            g.setColour(accentColor);
            g.drawRoundedRectangle(bounds.reduced(0.75f), cornerSize, 1.5f);
            textColor = accentColor;
        }
    } else {
        g.setColour(juce::Colour(mst::theme::panelTop));
        g.fillRoundedRectangle(bounds, cornerSize);
        g.setColour(juce::Colour(mst::theme::border));
        g.drawRoundedRectangle(bounds, cornerSize, 1.0f);
        textColor = juce::Colour(mst::theme::textMid);
    }

    g.setColour(textColor);
    g.setFont(juce::Font(10.0f).boldened());
    g.drawFittedText(getButtonText(), bounds.toNearestInt(), juce::Justification::centred, 1);
}

void PillButton::setSegmentGroup(bool isFirst, bool isLast)
{
    isFirstInGroup = isFirst;
    isLastInGroup = isLast;
}
