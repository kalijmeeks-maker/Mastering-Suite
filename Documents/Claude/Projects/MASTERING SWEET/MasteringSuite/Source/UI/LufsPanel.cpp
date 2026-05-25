#include "LufsPanel.h"
#include "../PluginProcessor.h"

LufsPanel::LufsPanel(MasteringSuiteProcessor& proc) : processor(proc) {}

void LufsPanel::paint(juce::Graphics& g) {
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
    g.drawText(juce::String::fromUTF8("LOUDNESS \u00B7 LUFS"), 14, 8, (int)bounds.getWidth() - 28, 14, juce::Justification::topLeft);

    // Giant LUFS number
    float integratedLufs = processor.getMeter().getIntegratedLufs();
    juce::String intLufsStr = (integratedLufs > -70.0f) ? juce::String(integratedLufs, 1) : "-inf";

    g.setFont(juce::Font(48.0f).boldened());
    g.setColour(juce::Colour(mst::theme::tabEq)); // Cyan
    g.drawText(intLufsStr, 14, 40, 150, 60, juce::Justification::centredLeft);
    
    g.setFont(juce::Font(9.0f).boldened());
    g.setColour(juce::Colour(mst::theme::textLow));
    g.drawText("INTEGRATED", 16, 95, 100, 12, juce::Justification::topLeft);

    // Dual L+R 13-segment Color-Graded Bar Meter (v2 design)
    auto meterArea = bounds.removeFromRight(96).reduced(8, 40);
    g.setColour(juce::Colour(mst::theme::panelInner));
    g.fillRoundedRectangle(meterArea, 4.0f);

    const int segments = 13;
    const float chW = (meterArea.getWidth() - 4 - 4) * 0.5f;  // 4px outer pad, 4px gap between cols
    const float segGap = 1.5f;
    const float segH = (meterArea.getHeight() - 4) / segments;

    auto drawColumn = [&](float colX, float dbLevel) {
        float normalized = juce::jlimit(0.0f, 1.0f, (dbLevel + 60.0f) / 60.0f);
        int litSegs = (int)(normalized * segments);
        for (int i = 0; i < segments; ++i) {
            float y = meterArea.getBottom() - 2 - (i + 1) * segH;
            // v2 color grade top→bottom: red → orange → yellow → lime → mint → dark mint
            juce::Colour c = (i >= 11) ? juce::Colour(0xFFFF3A4D)     // top 2: red
                           : (i >= 9)  ? juce::Colour(0xFFFF8A3A)     // 2: orange
                           : (i == 8)  ? juce::Colour(0xFFFFD54A)     // 1: yellow
                           : (i == 7)  ? juce::Colour(0xFFB9FF3A)     // 1: lime
                           : (i >= 4)  ? juce::Colour(0xFF3AFFB0)     // 3: mint
                                       : juce::Colour(0xFF2A8A6A);    // bottom: dark mint
            g.setColour(i < litSegs ? c : c.withAlpha(0.10f));
            g.fillRect(colX, y, chW, segH - segGap);
        }
    };

    const float colL = meterArea.getX() + 2;
    const float colR = colL + chW + 4;
    drawColumn(colL, processor.getChannelLevelL());
    drawColumn(colR, processor.getChannelLevelR());

    // Per-column letter labels
    g.setFont(juce::Font(7.0f).boldened());
    g.setColour(juce::Colour(mst::theme::textLow));
    g.drawText("L", (int)colL, (int)meterArea.getBottom(), (int)chW, 10, juce::Justification::centred);
    g.drawText("R", (int)colR, (int)meterArea.getBottom(), (int)chW, 10, juce::Justification::centred);

    // Shared scale labels along the right edge
    g.setFont(juce::Font(7.0f));
    for (int db : {0, -6, -12, -18, -24, -36, -48}) {
        float y = meterArea.getBottom() - ((db + 60.0f) / 60.0f) * meterArea.getHeight();
        g.drawText(juce::String(db), meterArea.getRight() + 3, (int)y - 4, 15, 8, juce::Justification::centredLeft);
    }

    // Readout Column (Anchored to top-right of center area)
    auto readoutArea = bounds.removeFromRight(140).reduced(0, 45);
    int rowH = 14;

    auto drawRow = [&](juce::String label, float val, int row, juce::String unit) {
        juce::String valStr = (val > -70.0f) ? juce::String(val, 1) : "-inf";
        float y = readoutArea.getY() + row * rowH;
        g.setFont(juce::Font(9.0f));
        g.setColour(juce::Colour(mst::theme::textLow));
        g.drawText(label, readoutArea.getX(), (int)y, 65, rowH, juce::Justification::centredLeft);
        
        g.setColour(juce::Colour(mst::theme::textHigh));
        g.drawText(valStr, readoutArea.getX() + 65, (int)y, 40, rowH, juce::Justification::centredRight);
        
        g.setColour(juce::Colour(mst::theme::textLow).withAlpha(0.4f));
        g.drawText(unit, readoutArea.getX() + 110, (int)y, 30, rowH, juce::Justification::centredLeft);
    };

    float shortTerm = processor.getMeter().getShortTermLufs();
    float momentary = processor.getMeter().getMomentaryLufs();
    float range = processor.getMeter().getLoudnessRange();
    float truePeak = processor.getMeter().getTruePeakDbfs();

    drawRow("SHORT", shortTerm, 0, "LUFS");
    drawRow("MOMENTARY", momentary, 1, "LUFS");
    drawRow("RANGE", range, 2, "LU");
    drawRow("TRUE PEAK", truePeak, 3, "dBTP");
}

void LufsPanel::resized() {}

void LufsPanel::refresh() {
    repaint();
}
