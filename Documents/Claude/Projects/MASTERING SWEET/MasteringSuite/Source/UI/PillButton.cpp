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

    // Optional LED dot at the left edge. Bright accent when toggled, dim otherwise.
    int textLeftPad = 0;
    if (showLed) {
        const float dotR = 6.0f;
        const float dotX = bounds.getX() + 10.0f;
        const float dotY = bounds.getCentreY() - dotR * 0.5f;
        g.setColour(getToggleState() ? accentColor : juce::Colour(0xFF2A2A3A));
        g.fillEllipse(dotX, dotY, dotR, dotR);
        textLeftPad = (int)(10.0f + dotR + 4.0f);
    }

    g.setColour(textColor);
    g.setFont(juce::Font(10.0f).boldened());
    auto textArea = bounds.toNearestInt();
    if (textLeftPad > 0)
        textArea = textArea.withTrimmedLeft(textLeftPad).withTrimmedRight(8);
    g.drawFittedText(getButtonText(), textArea,
                     showLed ? juce::Justification::centredLeft : juce::Justification::centred, 1);
}

void PillButton::setSegmentGroup(bool isFirst, bool isLast)
{
    isFirstInGroup = isFirst;
    isLastInGroup = isLast;
}
