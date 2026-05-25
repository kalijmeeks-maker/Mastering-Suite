#include "EqPanel.h"
#include "../PluginProcessor.h"
#include "EqCurveDisplay.cpp"

EqPanel::EqPanel(MasteringSuiteProcessor& proc, NeonLookAndFeel& laf)
    : processor(proc), lookAndFeel(laf) {

    curveDisplay = std::make_unique<EqCurveDisplay>(proc);
    addAndMakeVisible(*curveDisplay);

    for (int i = 0; i < 6; ++i) {
        juce::Colour bandColor = (i == 0) ? juce::Colour(0xFFFF00FF) : 
                                 (i == 1) ? juce::Colour(0xFF00D9FF) :
                                 (i == 2) ? juce::Colour(0xFF00FF41) :
                                 (i == 3) ? juce::Colour(0xFFFF9500) :
                                 (i == 4) ? juce::Colour(0xFFBB00FF) :
                                 juce::Colour(0xFF00FFCC);
        
        auto cell = std::make_unique<EqBandCell>(i, bandColor, processor);
        addAndMakeVisible(*cell);
        bandCells.push_back(std::move(cell));
    }
}

void EqPanel::paint(juce::Graphics& g) {
    auto bounds = getLocalBounds().toFloat();

    // Background gradient
    juce::ColourGradient bg(
        juce::Colour(mst::theme::panelTop), 0.0f, 0.0f,
        juce::Colour(mst::theme::panelInner), 0.0f, bounds.getHeight(),
        false);
    g.setGradientFill(bg);
    g.fillRoundedRectangle(bounds, 8.0f);

    // Border
    g.setColour(juce::Colour(mst::theme::border).withAlpha(0.3f));
    g.drawRoundedRectangle(bounds, 8.0f, 1.0f);

    // Header
    g.setFont(juce::Font(11.0f).boldened());
    g.setColour(juce::Colour(mst::theme::textHigh));
    g.drawText(juce::String::fromUTF8("EQ \u00B7 6-BAND PRECISION"), 14, 8, (int)bounds.getWidth() - 28, 14, juce::Justification::topLeft);
}

void EqPanel::resized() {
    auto bounds = getLocalBounds();
    
    auto curveArea = bounds.reduced(10);
    curveArea.removeFromTop(30); // header
    auto stripArea = curveArea.removeFromBottom(70).reduced(0, 5);
    curveArea.removeFromBottom(10); // gap
    
    curveDisplay->setBounds(curveArea);
    
    int stripW = stripArea.getWidth() / 6;
    for (int i = 0; i < 6; ++i) {
        bandCells[i]->setBounds(stripArea.removeFromLeft(stripW).reduced(2));
    }
}

void EqPanel::refreshSpectrum() {
    curveDisplay->refreshSpectrum();
}
