#pragma once
#include <JuceHeader.h>
#include "Theme.h"
#include "../PluginProcessor.h"

class TabBarComponent : public juce::Component {
public:
    struct Tab {
        juce::String name;
        juce::Colour accent;
    };

    TabBarComponent(MasteringSuiteProcessor& proc) : processor(proc) {
        tabs.push_back({ "EQ", juce::Colour(mst::theme::tabEq) });
        tabs.push_back({ "DYNAMICS", juce::Colour(mst::theme::tabDyn) });
        tabs.push_back({ "IMAGER", juce::Colour(mst::theme::tabImg) });
        tabs.push_back({ "LIMITER", juce::Colour(mst::theme::tabLim) });
    }

    void paint(juce::Graphics& g) override {
        auto bounds = getLocalBounds().toFloat();
        float tabWidth = bounds.getWidth() / (float)tabs.size();

        for (int i = 0; i < (int)tabs.size(); ++i) {
            auto tabRect = bounds.withX(i * tabWidth).withWidth(tabWidth);
            bool isSelected = (i == selectedIndex);

            if (isSelected) {
                // Subtle raised gradient on the active tab (v2: panelTop→bgBase)
                g.setGradientFill(juce::ColourGradient(
                    juce::Colour(0xFF14141D), 0, tabRect.getY(),
                    juce::Colour(0xFF11111A), 0, tabRect.getBottom(), false));
                g.fillRect(tabRect);
                // Accent underline cap (v2)
                g.setColour(tabs[i].accent);
                g.fillRect(tabRect.getX() + 14.0f, tabRect.getBottom() - 2.0f,
                           tabRect.getWidth() - 28.0f, 2.0f);
            }

            // 6px LED dot left of the label (v2 design — bright when active).
            const float dotR = 6.0f;
            const float dotY = tabRect.getCentreY() - dotR * 0.5f - 3.0f;
            const float dotX = tabRect.getX() + 14.0f;
            g.setColour(isSelected ? tabs[i].accent : juce::Colour(0xFF2A2A3A));
            g.fillEllipse(dotX, dotY, dotR, dotR);

            // Tab label
            g.setColour(isSelected ? juce::Colour(mst::theme::textHigh)
                                   : juce::Colour(mst::theme::textLow).withAlpha(0.6f));
            g.setFont(juce::Font(11.0f).boldened());
            auto labelArea = tabRect.withX(dotX + dotR + 8.0f).withWidth(tabRect.getRight() - (dotX + dotR + 8.0f) - 32.0f);
            g.drawText(tabs[i].name, labelArea.toNearestInt(), juce::Justification::centredLeft);

            // Activity indicator on the right edge of the tab.
            auto indicatorArea = tabRect.withX(tabRect.getRight() - 32.0f).withWidth(30.0f);
            drawIndicator(g, indicatorArea.reduced(2), i);
        }

        g.setColour(juce::Colour(mst::theme::border).withAlpha(0.4f));
        g.drawHorizontalLine((int)bounds.getBottom() - 1, 0, bounds.getWidth());
    }

    void drawIndicator(juce::Graphics& g, juce::Rectangle<float> area, int tabIdx) {
        if (tabIdx == 0) {
            // EQ: 24x8 cyan sparkline showing the live EQ response across log freq.
            const float sparkW = 24.0f, sparkH = 8.0f;
            auto s = juce::Rectangle<float>(area.getCentreX() - sparkW * 0.5f,
                                            area.getCentreY() - sparkH * 0.5f,
                                            sparkW, sparkH);
            juce::Path p;
            const int N = 24;
            float minLog = std::log10(20.0f), maxLog = std::log10(20000.0f);
            for (int i = 0; i < N; ++i) {
                float t = (float)i / (N - 1);
                float hz = std::pow(10.0f, minLog + t * (maxLog - minLog));
                float gainDb = juce::Decibels::gainToDecibels(
                    juce::jmax(0.000001f, processor.getEQ().getFrequencyResponse(hz)));
                // Map ±12 dB across the sparkline height.
                float y = s.getCentreY() - juce::jlimit(-1.0f, 1.0f, gainDb / 12.0f) * (sparkH * 0.5f);
                float x = s.getX() + t * sparkW;
                if (i == 0) p.startNewSubPath(x, y);
                else        p.lineTo(x, y);
            }
            g.setColour(juce::Colour(mst::theme::tabEq));
            g.strokePath(p, juce::PathStrokeType(1.2f));
        } else if (tabIdx == 1) {
            // DYNAMICS: 2x14 orange GR bar pinned to right edge.
            const float barW = 2.0f, barH = 14.0f;
            float gr = juce::jlimit(0.0f, 1.0f,
                                    std::abs(processor.getCompressor().getCurrentGainReduction()) / 20.0f);
            float x = area.getRight() - barW - 2.0f;
            float y = area.getBottom() - barH - 1.0f;
            g.setColour(juce::Colour(mst::theme::tabDyn).withAlpha(0.25f));
            g.fillRect(x, y, barW, barH);
            g.setColour(juce::Colour(mst::theme::tabDyn));
            g.fillRect(x, y + barH * (1.0f - gr), barW, barH * gr);
        } else if (tabIdx == 2) {
            // IMAGER: correlation readout text in violet.
            float c = processor.getCorrelation();
            juce::String txt = (c >= 0.0f ? "+" : "") + juce::String(c, 2);
            g.setColour(juce::Colour(mst::theme::tabImg));
            g.setFont(juce::Font(9.0f).boldened());
            g.drawText(txt, area, juce::Justification::centred);
        } else if (tabIdx == 3) {
            // LIMITER: 2x14 mint GR bar + 4x4 TP status dot.
            const float barW = 2.0f, barH = 14.0f;
            float gr = juce::jlimit(0.0f, 1.0f,
                                    std::abs(processor.getLimiter().getCurrentGainReduction()) / 20.0f);
            float barX = area.getRight() - barW - 8.0f;
            float barY = area.getBottom() - barH - 1.0f;
            auto mint = juce::Colour(mst::theme::tabLim);
            g.setColour(mint.withAlpha(0.25f));
            g.fillRect(barX, barY, barW, barH);
            g.setColour(mint);
            g.fillRect(barX, barY + barH * (1.0f - gr), barW, barH * gr);

            // 4x4 TP dot — bright mint when limiter is catching peaks, dim otherwise.
            bool tp = processor.getTruePeakActive();
            g.setColour(tp ? mint : mint.withAlpha(0.25f));
            g.fillEllipse(area.getRight() - 4.0f, area.getCentreY() - 2.0f, 4.0f, 4.0f);
        }
    }

    void mouseDown(const juce::MouseEvent& e) override {
        float tabWidth = getWidth() / (float)tabs.size();
        int newIndex = (int)(e.position.x / tabWidth);
        if (newIndex >= 0 && newIndex < (int)tabs.size()) {
            selectedIndex = newIndex;
            if (onSelectionChanged) onSelectionChanged(selectedIndex);
            repaint();
        }
    }

    void setSelectedIndex(int index) {
        selectedIndex = juce::jlimit(0, (int)tabs.size() - 1, index);
        if (onSelectionChanged) onSelectionChanged(selectedIndex);
        repaint();
    }
    
    int getSelectedIndex() const { return selectedIndex; }

    std::function<void(int)> onSelectionChanged;

private:
    std::vector<Tab> tabs;
    int selectedIndex = 0;
    MasteringSuiteProcessor& processor;
};
