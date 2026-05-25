#pragma once
#include <JuceHeader.h>
#include "Theme.h"
#include "../PluginProcessor.h"

// Compact EQ band readout. Visual is from "Mastering Sweet v2" mockup —
// colored left border, type pill in top-right, FREQ/GAIN/Q rendered as a
// 2-col key/value grid. Density param controls suffix visibility + padding.
class EqBandCell : public juce::Component, public juce::AudioProcessorValueTreeState::Listener {
public:
    EqBandCell(int bandIdx, juce::Colour c, MasteringSuiteProcessor& proc)
        : index(bandIdx), color(c), processor(proc) {

        auto& apvts = processor.getAPVTS();
        juce::String prefix = "eq" + juce::String(bandIdx);

        apvts.addParameterListener(prefix + "Type", this);
        apvts.addParameterListener(prefix + "Gain", this);
        apvts.addParameterListener(prefix + "Freq", this);
        apvts.addParameterListener(prefix + "Q", this);
        apvts.addParameterListener("density", this);
    }

    ~EqBandCell() override {
        auto& apvts = processor.getAPVTS();
        juce::String prefix = "eq" + juce::String(index);
        apvts.removeParameterListener(prefix + "Type", this);
        apvts.removeParameterListener(prefix + "Gain", this);
        apvts.removeParameterListener(prefix + "Freq", this);
        apvts.removeParameterListener(prefix + "Q", this);
        apvts.removeParameterListener("density", this);
    }

    void paint(juce::Graphics& g) override {
        auto bounds = getLocalBounds().toFloat();

        int density = (int)*processor.getAPVTS().getRawParameterValue("density");
        bool compact  = (density == 0);
        bool expanded = (density == 2);
        float pad     = compact ? 4.0f : (expanded ? 8.0f : 6.0f);

        // Card body
        g.setColour(juce::Colour(mst::theme::panelInner));
        g.fillRoundedRectangle(bounds, 4.0f);
        g.setColour(juce::Colour(0xFF1D1D29));
        g.drawRoundedRectangle(bounds, 4.0f, 1.0f);

        // Colored left border (v2: 2px accent down the left edge)
        g.setColour(color);
        g.fillRect(bounds.getX(), bounds.getY() + 2.0f, 2.0f, bounds.getHeight() - 4.0f);

        // Type label + on/off dot
        float typeVal = *processor.getAPVTS().getRawParameterValue("eq" + juce::String(index) + "Type");
        const char* typeStrs[] = { "BYPASS", "PEAK", "HPF", "LPF", "L-SH", "H-SH" };
        int ti = juce::jlimit(0, 5, (int)typeVal);
        bool bandOn = (ti != 0);

        g.setFont(juce::Font(8.0f).boldened());
        g.setColour(color);
        g.drawText(juce::String(index + 1), juce::Rectangle<float>(bounds.getX() + 6, bounds.getY() + pad, 16, 10).toNearestInt(),
                   juce::Justification::topLeft);

        g.setColour(juce::Colour(mst::theme::textLow));
        g.drawText(typeStrs[ti], juce::Rectangle<float>(bounds.getX() + 22, bounds.getY() + pad, bounds.getWidth() - 34, 10).toNearestInt(),
                   juce::Justification::topLeft);

        // On-state dot top-right
        float dotR = 4.0f;
        if (bandOn) {
            g.setColour(color);
        } else {
            g.setColour(juce::Colour(0xFF2A2A3A));
        }
        g.fillEllipse(bounds.getRight() - dotR - pad, bounds.getY() + pad + 1.0f, dotR, dotR);

        // Values — 2-col grid with k/v pairs
        float f      = *processor.getAPVTS().getRawParameterValue("eq" + juce::String(index) + "Freq");
        float gainDb = *processor.getAPVTS().getRawParameterValue("eq" + juce::String(index) + "Gain");
        float q      = *processor.getAPVTS().getRawParameterValue("eq" + juce::String(index) + "Q");

        const float rowY0 = bounds.getY() + pad + 14.0f;
        const float rowH  = compact ? 10.0f : 11.0f;
        const float kCol  = bounds.getX() + 6.0f;
        const float vCol  = bounds.getRight() - pad;
        const float vW    = vCol - kCol - 4.0f;

        auto drawRow = [&](int row, const char* k, const juce::String& v) {
            float y = rowY0 + row * rowH;
            g.setFont(juce::Font(8.0f));
            g.setColour(juce::Colour(mst::theme::textLow));
            g.drawText(k, juce::Rectangle<float>(kCol, y, 40, rowH).toNearestInt(),
                       juce::Justification::centredLeft);
            g.setFont(juce::Font(9.0f));
            // Gain row uses cyan for boost, soft-red for cut (v2 spec)
            g.setColour(juce::Colour(mst::theme::textHigh));
            g.drawText(v, juce::Rectangle<float>(kCol + 22, y, vW - 18, rowH).toNearestInt(),
                       juce::Justification::centredRight);
        };

        juce::String freqStr = (f < 1000.0f) ? juce::String((int)f) : juce::String(f / 1000.0f, 1) + "k";
        juce::String gainStr = juce::String(gainDb, 1);
        juce::String qStr    = juce::String(q, 2);

        if (compact) {
            // Suffixes removed in compact mode per spec.
            drawRow(0, "F",    freqStr);
            drawRow(1, "G",    gainStr);
            drawRow(2, "Q",    qStr);
        } else {
            drawRow(0, "FREQ", freqStr + " Hz");
            drawRow(1, "GAIN", gainStr + " dB");
            drawRow(2, "Q",    qStr);
            if (expanded) {
                // 1-line tooltip subtitle using parameter metadata (v2 expanded mode).
                auto* param = processor.getAPVTS().getParameter("eq" + juce::String(index) + "Type");
                juce::String hint = param != nullptr ? param->getName(64) : juce::String();
                g.setFont(juce::Font(7.0f));
                g.setColour(juce::Colour(mst::theme::textLow).withAlpha(0.75f));
                g.drawText(hint,
                           juce::Rectangle<float>(kCol, bounds.getBottom() - pad - 9.0f,
                                                  bounds.getWidth() - 2 * pad, 9.0f).toNearestInt(),
                           juce::Justification::centredLeft);
            }
        }
    }

    void parameterChanged(const juce::String&, float) override {
        juce::MessageManager::callAsync([safe = juce::Component::SafePointer<EqBandCell>(this)] {
            if (safe != nullptr) safe->repaint();
        });
    }

private:
    int index;
    juce::Colour color;
    MasteringSuiteProcessor& processor;
};
