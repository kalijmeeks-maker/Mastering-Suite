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

    // HPF / LPF readout \u2014 these bands live as edge handles on the curve canvas,
    // so we surface their cutoff freqs once in dim mono in the top-right of the panel header.
    auto* hpf = processor.getAPVTS().getRawParameterValue("eq0Freq");
    auto* lpf = processor.getAPVTS().getRawParameterValue("eq5Freq");
    if (hpf && lpf) {
        float hf = *hpf, lf = *lpf;
        juce::String hStr = (hf < 1000.0f) ? juce::String((int)hf) + " Hz" : juce::String(hf / 1000.0f, 1) + " kHz";
        juce::String lStr = (lf < 1000.0f) ? juce::String((int)lf) + " Hz" : juce::String(lf / 1000.0f, 1) + " kHz";
        g.setFont(juce::Font(9.0f));
        g.setColour(juce::Colour(mst::theme::textLow));
        g.drawText("HPF " + hStr + juce::String::fromUTF8(" \u00B7 LPF ") + lStr,
                   0, 8, (int)bounds.getWidth() - 14, 14, juce::Justification::topRight);
    }
}

void EqPanel::resized() {
    auto bounds = getLocalBounds();

    auto curveArea = bounds.reduced(10);
    curveArea.removeFromTop(30); // header
    auto stripArea = curveArea.removeFromBottom(70).reduced(0, 5);
    curveArea.removeFromBottom(10); // gap

    curveDisplay->setBounds(curveArea);

    // Design's call: HPF (band 0) and LPF (band 5) live as edge handles on the
    // curve canvas only, not in the band strip. The remaining 4 peak/shelf bands
    // get the full horizontal space — each cell ~187px instead of ~125px.
    bandCells[0]->setVisible(false);
    bandCells[5]->setVisible(false);

    const int visibleBands = 4;
    int stripW = stripArea.getWidth() / visibleBands;
    for (int i = 1; i <= 4; ++i) {
        bandCells[i]->setBounds(stripArea.removeFromLeft(stripW).reduced(2));
    }
}

void EqPanel::refreshSpectrum() {
    curveDisplay->refreshSpectrum();
}
