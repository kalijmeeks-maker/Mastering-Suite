#pragma once
#include <JuceHeader.h>
#include "Theme.h"

class PillButton : public juce::ToggleButton {
public:
    enum class Variant { Filled, Outlined };

    PillButton();
    ~PillButton() override = default;

    void paint(juce::Graphics& g) override;
    void setSegmentGroup(bool isFirst, bool isLast);

    void setAccentColor(juce::Colour c) { accentColor = c; repaint(); }
    void setVariant(Variant v)          { variant = v;     repaint(); }
    // Optional leading LED dot. When `on==true` paints in accentColor, otherwise
    // dim gray. Use setShowLed(false) to hide entirely (default).
    void setShowLed(bool show)          { showLed = show;  repaint(); }

private:
    Variant variant = Variant::Filled;
    juce::Colour accentColor = juce::Colour(0xFFF3F3FA);  // textHigh default
    bool isFirstInGroup = false;
    bool isLastInGroup = false;
    bool showLed = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PillButton)
};
