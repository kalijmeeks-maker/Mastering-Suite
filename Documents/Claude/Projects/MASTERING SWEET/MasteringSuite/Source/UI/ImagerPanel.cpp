#include "ImagerPanel.h"
#include "../PluginProcessor.h"

ImagerPanel::ImagerPanel(MasteringSuiteProcessor& proc, NeonLookAndFeel& laf)
    : processor(proc), lookAndFeel(laf) {

    auto createKnob = [&](std::unique_ptr<juce::Slider>& k, std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>& a, juce::String paramID, juce::Colour color) {
        k = std::make_unique<juce::Slider>(juce::Slider::RotaryVerticalDrag, juce::Slider::TextBoxBelow);
        k->setLookAndFeel(&lookAndFeel);
        k->setColour(juce::Slider::rotarySliderFillColourId, color);
        a = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(processor.getAPVTS(), paramID, *k);
        if (auto* param = processor.getAPVTS().getParameter(paramID))
            k->setTooltip(param->getName(64));
        addAndMakeVisible(*k);
    };

    createKnob(widthK, widthA, "imgWidth", juce::Colour(mst::theme::tabImg));
    createKnob(panK, panA, "imgPan", juce::Colour(mst::theme::tabImg));
}

void ImagerPanel::paint(juce::Graphics& g) {
    auto bounds = getLocalBounds().toFloat();

    // Background
    juce::ColourGradient bg(
        juce::Colour(mst::theme::panelTop), 0.0f, 0.0f,
        juce::Colour(mst::theme::panelInner), 0.0f, bounds.getHeight(),
        false);
    g.setGradientFill(bg);
    g.fillRoundedRectangle(bounds, 8.0f);
    g.setColour(juce::Colour(mst::theme::border).withAlpha(0.3f));
    g.drawRoundedRectangle(bounds, 8.0f, 1.0f);

    // Header
    g.setFont(juce::Font(11.0f).boldened());
    g.setColour(juce::Colour(mst::theme::textHigh));
    g.drawText(juce::String::fromUTF8("IMAGER \u00B7 STEREO FIELD"), 14, 8, (int)bounds.getWidth() - 28, 14, juce::Justification::topLeft);

    // Goniometer Area
    auto visualArea = bounds.reduced(100, 40).withY(40).withHeight(bounds.getHeight() - 150);
    drawGoniometer(g, visualArea);
}

void ImagerPanel::drawGoniometer(juce::Graphics& g, juce::Rectangle<float> area) {
    g.setColour(juce::Colour(mst::theme::bgBase));
    g.fillEllipse(area);
    g.setColour(juce::Colour(mst::theme::border).withAlpha(0.5f));
    g.drawEllipse(area, 1.0f);

    // Crosshair
    g.drawVerticalLine((int)area.getCentreX(), area.getY(), area.getBottom());
    g.drawHorizontalLine((int)area.getCentreY(), area.getX(), area.getRight());

    // Goniometer Drawing with real samples
    std::array<float, 1024> left, right;
    processor.getGoniometerSamples(left.data(), right.data());
    
    g.setColour(juce::Colour(mst::theme::tabImg).withAlpha(0.6f));
    
    // Draw scatter points
    for (int i = 0; i < 1024; i += 2) {
        float l = left[i];
        float r = right[i];

        // x = (l - r) / sqrt(2), y = (l + r) / sqrt(2)
        // Scaled to fit comfortably in the circle
        float x = (l - r) * 0.7071f * area.getWidth() * 0.4f;
        float y = (l + r) * 0.7071f * area.getHeight() * 0.4f;

        float drawX = area.getCentreX() + x;
        float drawY = area.getCentreY() - y;

        if (area.contains(drawX, drawY)) {
            g.fillEllipse(drawX, drawY, 1.5f, 1.5f);
        }
    }
}

void ImagerPanel::resized() {
    auto bounds = getLocalBounds();
    auto knobArea = bounds.reduced(20, 20);
    knobArea.removeFromTop(bounds.getHeight() * 0.7f);

    int knobW = 80;
    widthK->setBounds(bounds.getWidth() / 2 - knobW - 10, knobArea.getY(), knobW, knobW);
    panK->setBounds(bounds.getWidth() / 2 + 10, knobArea.getY(), knobW, knobW);
}

void ImagerPanel::refresh() {
    repaint();
}
