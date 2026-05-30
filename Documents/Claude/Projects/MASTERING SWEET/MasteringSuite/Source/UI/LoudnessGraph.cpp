#include "LoudnessGraph.h"
#include "../PluginProcessor.h"

// ── v1.0.4 Loudness Restack — shared range constants ──
// History bars + hero zone share a single -48 → 0 dB axis. The mint dashed
// target line at -14 LUFS is the visual contract that says
// "bars touching the line ⇒ hero number reads -14".
static constexpr float kBottomDb   = -48.0f;
static constexpr float kTopDb      =   0.0f;
static constexpr float kRangeDb    = kTopDb - kBottomDb;
static constexpr float kTargetLufs = -14.0f;

// ── Layout constants (DEFAULT density unless overridden by APVTS) ──
namespace {
    constexpr float kLeftPad     = 28.0f;
    constexpr float kRightPad    = 16.0f;
    constexpr float kTitleRowH   = 28.0f;
    constexpr float kYAxisColW   = 28.0f;

    // L/R meter strip is 84 px wide:  32 (L) + 10 (gap) + 32 (R) + 10 (labels)
    constexpr float kLRMeterW       = 32.0f;
    constexpr float kLRMeterGap     = 10.0f;
    constexpr float kLRLabelsW      = 10.0f;
    constexpr float kLRStripW       = kLRMeterW + kLRMeterGap + kLRMeterW + kLRLabelsW;

    // Chip dimensions (top-right corner — SHORT, MOM, RANGE, TP)
    constexpr float kChipW = 78.0f, kChipH = 20.0f, kChipGap = 4.0f;
}

LoudnessGraph::LoudnessGraph(MasteringSuiteProcessor& proc) : processor(proc) {}

void LoudnessGraph::paint(juce::Graphics& g) {
    auto bounds = getLocalBounds().toFloat();

    // ── Background + border (same chrome as the prior panels) ──
    {
        juce::ColourGradient bg(
            juce::Colour(mst::theme::panelTop),   0.0f, 0.0f,
            juce::Colour(mst::theme::panelInner), 0.0f, bounds.getHeight(),
            false);
        g.setGradientFill(bg);
        g.fillRoundedRectangle(bounds, 8.0f);

        g.setColour(juce::Colour(mst::theme::border).withAlpha(0.3f));
        g.drawRoundedRectangle(bounds, 8.0f, 1.0f);
    }

    // ── Density mode (APVTS choice: 0 COMPACT, 1 DEFAULT, 2 EXPANDED) ──
    int historyZoneH = 90, heroZoneH = 90, heroFontPx = 78;
    if (auto* p = processor.getAPVTS().getRawParameterValue("density")) {
        const int density = (int)*p;
        if      (density == 0) { historyZoneH =  60; heroZoneH =  60; heroFontPx = 56; }
        else if (density == 2) { historyZoneH = 120; heroZoneH = 120; heroFontPx = 96; }
        // density == 1 falls through to DEFAULT defaults
    }

    // ── Live data sampled once per paint ──
    const double nowMs       = juce::Time::getMillisecondCounterHiRes();
    const float  integratedLufs = processor.getMeter().getIntegratedLufs();
    const float  shortTerm   = processor.getMeter().getShortTermLufs();
    const float  momentary   = processor.getMeter().getMomentaryLufs();
    const float  range       = processor.getMeter().getLoudnessRange();
    const float  truePeak    = processor.getMeter().getTruePeakDbfs();

    // ════════════════════════════════════════════════════════════════════
    //  TITLE ROW  — top 28 px, "LOUDNESS HISTORY" left + 4 chips right
    // ════════════════════════════════════════════════════════════════════
    auto titleRow = bounds.removeFromTop(kTitleRowH);
    {
        g.setFont(juce::Font(11.0f).boldened());
        g.setColour(juce::Colour(mst::theme::textHigh));
        g.drawText("LOUDNESS HISTORY",
                   (int)(titleRow.getX() + kLeftPad), (int)titleRow.getY(),
                   200, (int)titleRow.getHeight(),
                   juce::Justification::centredLeft);

        // Chip row (top-right). 4 chips, each 78 × 20, 4 px gap. Anchored
        // immediately left of the L/R strip column.
        const float chipsRight = bounds.getRight() - kRightPad - kLRStripW - 6.0f;
        const float chipsY     = titleRow.getY() + (titleRow.getHeight() - kChipH) * 0.5f;

        struct Chip { const char* label; float value; const char* unit; };
        Chip chips[] = {
            { "SHORT", shortTerm, "LUFS" },
            { "MOM",   momentary, "LUFS" },
            { "RANGE", range,     "LU"   },
            { "TP",    truePeak,  "dBTP" }
        };
        for (int i = 3; i >= 0; --i) {
            float cx = chipsRight - (4 - i) * (kChipW + kChipGap) + kChipGap;
            juce::Rectangle<float> chipR(cx, chipsY, kChipW, kChipH);

            g.setColour(juce::Colour(mst::theme::panelInner));
            g.fillRoundedRectangle(chipR, 3.0f);
            g.setColour(juce::Colour(mst::theme::border).withAlpha(0.35f));
            g.drawRoundedRectangle(chipR, 3.0f, 0.8f);

            g.setFont(juce::Font(juce::FontOptions("SF Mono", 9.5f, juce::Font::plain)));
            g.setColour(juce::Colour(mst::theme::textLow));
            g.drawText(chips[i].label,
                       chipR.toNearestInt().withTrimmedLeft(6).withWidth(34),
                       juce::Justification::centredLeft);

            const juce::String valStr = (chips[i].value > -70.0f)
                ? juce::String(chips[i].value, 1)
                : juce::String("-inf");
            g.setColour(juce::Colour(mst::theme::textHigh));
            g.drawText(valStr,
                       chipR.toNearestInt().withTrimmedRight(4).withTrimmedLeft(40),
                       juce::Justification::centredRight);
        }
    }

    // ════════════════════════════════════════════════════════════════════
    //  L/R STRIP  — right side, 32+10+32+10 = 84 px wide
    //  (per v1.0.4 lock: preserve the existing 5-colour ramp; do not touch)
    // ════════════════════════════════════════════════════════════════════
    bounds.removeFromRight(kRightPad);    // panel right padding
    auto lrArea = bounds.removeFromRight(kLRStripW);
    {
        auto bodyArea = lrArea.reduced(0.0f, 10.0f);
        const float lX = bodyArea.getX();
        const float rX = lX + kLRMeterW + kLRMeterGap;
        const float labelsX = rX + kLRMeterW;
        const float topY = bodyArea.getY();
        const float botY = bodyArea.getBottom();
        const float colH = botY - topY;

        // Frame each column
        g.setColour(juce::Colour(mst::theme::panelInner));
        g.fillRoundedRectangle(lX, topY, kLRMeterW, colH, 3.0f);
        g.fillRoundedRectangle(rX, topY, kLRMeterW, colH, 3.0f);
        g.setColour(juce::Colour(mst::theme::border).withAlpha(0.4f));
        g.drawRoundedRectangle(lX, topY, kLRMeterW, colH, 3.0f, 1.0f);
        g.drawRoundedRectangle(rX, topY, kLRMeterW, colH, 3.0f, 1.0f);

        // Peak-hold update — hold 3 s, then decay 30 dB / 2 s linear.
        auto updatePeakHold = [&](float& hold, double& holdTime, float currentDb) {
            if (currentDb > hold) {
                hold     = currentDb;
                holdTime = nowMs;
            } else {
                const double age = nowMs - holdTime;
                if (age > 3000.0) {
                    const float drop = (float)((age - 3000.0) / 2000.0) * 30.0f;
                    hold     = juce::jmax(currentDb, hold - drop);
                    holdTime = nowMs;
                }
            }
        };
        updatePeakHold(peakHoldL, peakHoldTimeL, processor.getChannelLevelL());
        updatePeakHold(peakHoldR, peakHoldTimeR, processor.getChannelLevelR());

        const int   segments = 13;
        const float segGap   = 1.5f;
        const float segH     = (colH - 4.0f) / segments;

        auto drawColumn = [&](float colX, float dbLevel, float peakDb) {
            const float norm = juce::jlimit(0.0f, 1.0f, (dbLevel + 60.0f) / 60.0f);
            const int   lit  = (int)(norm * segments);
            for (int i = 0; i < segments; ++i) {
                const float y = botY - 2.0f - (i + 1) * segH;
                // 5-colour ramp (preserved from shipped LufsPanel — peak meters,
                // separate metric from the -48→0 LUFS axis used by history).
                const juce::Colour c =
                      (i >= 11) ? juce::Colour(0xFFFF3A4D)
                    : (i >= 9)  ? juce::Colour(0xFFFF8A3A)
                    : (i == 8)  ? juce::Colour(0xFFFFD54A)
                    : (i == 7)  ? juce::Colour(0xFFB9FF3A)
                    : (i >= 4)  ? juce::Colour(0xFF3AFFB0)
                                : juce::Colour(0xFF2A8A6A);
                g.setColour(i < lit ? c : c.withAlpha(0.10f));
                g.fillRect(colX + 3.0f, y, kLRMeterW - 6.0f, segH - segGap);
            }
            if (peakDb > -60.0f) {
                const float peakNorm = juce::jlimit(0.0f, 1.0f, (peakDb + 60.0f) / 60.0f);
                const float py = botY - 2.0f - peakNorm * (segH * segments);
                g.setColour(juce::Colours::white.withAlpha(0.9f));
                g.fillRect(colX + 3.0f, py, kLRMeterW - 6.0f, 1.0f);
            }
        };
        drawColumn(lX, processor.getChannelLevelL(), peakHoldL);
        drawColumn(rX, processor.getChannelLevelR(), peakHoldR);

        // L/R letter labels under each column
        g.setFont(juce::Font(7.0f).boldened());
        g.setColour(juce::Colour(mst::theme::textLow));
        g.drawText("L", (int)lX, (int)botY,     (int)kLRMeterW, 10, juce::Justification::centred);
        g.drawText("R", (int)rX, (int)botY,     (int)kLRMeterW, 10, juce::Justification::centred);

        // Shared dB axis labels (0/-6/-12/-18/-24/-36/-48) in the 10-px
        // labels strip on the right of R. Peak-meter scale — intentionally
        // different from the -48→0 LUFS axis the bars use.
        g.setFont(juce::Font(7.0f));
        for (int db : {0, -6, -12, -18, -24, -36, -48}) {
            const float y = botY - ((db + 60.0f) / 60.0f) * colH;
            g.drawText(juce::String(db), (int)labelsX + 1, (int)y - 4,
                       (int)kLRLabelsW - 1, 8, juce::Justification::centredLeft);
        }
    }

    // ════════════════════════════════════════════════════════════════════
    //  Y-AXIS DB LABELS COLUMN  — between history bars and L/R strip
    //  Same range/labels as the v1.0.1 fix (0/-8/-16/-24/-32/-40).
    // ════════════════════════════════════════════════════════════════════
    auto yAxisCol = bounds.removeFromRight(kYAxisColW);

    // ════════════════════════════════════════════════════════════════════
    //  HISTORY ZONE  — magenta bars with bright top edge.
    //  History (top) + Hero (below) stack inside the remaining bounds.
    //  Target line is a single dashed line at y = historyZoneBottom,
    //  spanning both zones.
    // ════════════════════════════════════════════════════════════════════
    const float zonesLeft   = bounds.getX() + kLeftPad;
    const float zonesRight  = bounds.getRight();        // already trimmed by L/R + Y-axis
    const float zonesWidth  = zonesRight - zonesLeft;
    const float zonesTop    = bounds.getY();
    const float historyTop  = zonesTop;
    const float historyBot  = historyTop + historyZoneH;
    const float heroTop     = historyBot;

    auto barsArea  = juce::Rectangle<float>(zonesLeft, historyTop, zonesWidth, (float)historyZoneH);
    auto heroArea  = juce::Rectangle<float>(zonesLeft, heroTop,    zonesWidth, (float)heroZoneH);

    // History bars
    {
        const float barWidth  = 15.0f;   // v1.0.4 lock
        const float barStride = 18.0f;   // = 15 + 3 gap
        const int   N         = juce::jmin((int)history.size(), maxSamples);
        const float barsRight = barsArea.getRight();

        for (int i = 0; i < N; ++i) {
            const float v = history[(size_t)(history.size() - N + i)];
            float normalized = (v - kBottomDb) / kRangeDb;
            normalized = juce::jlimit(0.03f, 1.0f, normalized);   // 3 % floor

            // Right-aligned: newest sample at the right edge.
            const float barX = barsRight - (N - i) * barStride;
            if (barX < barsArea.getX()) continue;
            const float barH = barsArea.getHeight() * normalized;
            const float barY = barsArea.getBottom() - barH;

            // Body — magenta gradient (existing palette)
            juce::ColourGradient bg(
                juce::Colour(0xFFFF5CD1).withAlpha(0.88f), barX, barY,
                juce::Colour(0xFF4A2080).withAlpha(0.20f), barX, barsArea.getBottom(),
                false);
            g.setGradientFill(bg);
            g.fillRect(barX, barY, barWidth, barH);

            // 1.5 px bright top edge (#ffd0f0 α 0.6) — cross-pollinates the
            // §2 edge-lit visual language onto history. v1.0.4 spec.
            g.setColour(juce::Colour(0xFFFFD0F0).withAlpha(0.6f));
            g.fillRect(barX, barY, barWidth, 1.5f);
        }
    }

    // Y-axis labels next to the bars (same set as v1.0.1)
    {
        g.setFont(juce::Font(8.0f));
        g.setColour(juce::Colour(mst::theme::textLow));
        for (int lufs : {0, -8, -16, -24, -32, -40}) {
            const float yNorm = ((float)lufs - kBottomDb) / kRangeDb;
            const float y     = barsArea.getBottom() - (barsArea.getHeight() * yNorm);
            g.drawText(juce::String(lufs),
                       (int)yAxisCol.getX() + 2, (int)y - 5,
                       (int)yAxisCol.getWidth() - 4, 10,
                       juce::Justification::centredRight);
        }
    }

    // ════════════════════════════════════════════════════════════════════
    //  HERO ZONE  — large cyan integrated LUFS readout
    //  Helvetica Neue, weight 200 (extra-light), per v1.0.4 spec.
    // ════════════════════════════════════════════════════════════════════
    {
        const juce::String heroStr = (integratedLufs > -70.0f)
            ? juce::String(integratedLufs, 1)
            : juce::String("-inf");

        // "Thin" maps to W200 on macOS Helvetica Neue. JUCE falls back to
        // system regular weight if the requested face isn't available.
        juce::Font heroFont = juce::Font(juce::FontOptions("Helvetica Neue",
                                                           (float)heroFontPx,
                                                           juce::Font::plain))
                                  .withTypefaceStyle("Thin");
        g.setFont(heroFont);
        g.setColour(juce::Colour(mst::theme::tabEq));   // cyan = "present voice"
        g.drawText(heroStr,
                   (int)heroArea.getX(), (int)heroArea.getY() + 4,
                   (int)heroArea.getWidth() - 80, heroFontPx + 10,
                   juce::Justification::topLeft);

        // Measure the hero glyph width so the "LUFS" suffix sits flush
        // regardless of the value (-inf vs -14.2 vs -7.3).
        const float heroNumberW = heroFont.getStringWidthFloat(heroStr);

        g.setFont(juce::Font(juce::FontOptions("SF Mono", 14.0f, juce::Font::plain)));
        g.setColour(juce::Colour(mst::theme::textLow));
        g.drawText("LUFS",
                   (int)(heroArea.getX() + heroNumberW + 8), (int)heroArea.getY() + 16,
                   80, 18, juce::Justification::topLeft);

        // INTEGRATED caption below the number
        g.setFont(juce::Font(juce::FontOptions("SF Mono", 9.0f, juce::Font::plain)));
        g.setColour(juce::Colour(mst::theme::textLow).withAlpha(0.7f));
        g.drawText("INTEGRATED",
                   (int)heroArea.getX(), (int)heroArea.getY() + heroFontPx + 6,
                   140, 12, juce::Justification::topLeft);
    }

    // ════════════════════════════════════════════════════════════════════
    //  -14 TARGET LINE  — single dashed line spanning both zones.
    //  Drawn LAST so it lives on top of bars and stays visible across the
    //  entire history + hero stack (the visual contract for "at target").
    // ════════════════════════════════════════════════════════════════════
    {
        const float yTarget = barsArea.getBottom() -
                              barsArea.getHeight() * ((kTargetLufs - kBottomDb) / kRangeDb);
        const float lineL = zonesLeft;
        const float lineR = zonesRight - 14.0f;   // ~10 px left of L/R strip

        g.setColour(juce::Colour(mst::theme::tabLim).withAlpha(0.65f));
        for (float x = lineL; x < lineR; x += 10.0f) {
            const float xEnd = juce::jmin(x + 6.0f, lineR);
            g.drawLine(x, yTarget, xEnd, yTarget, 1.0f);
        }

        // "-14" label sits to the right of the line, in mint, no chip.
        g.setFont(juce::Font(juce::FontOptions("SF Mono", 9.0f, juce::Font::plain)));
        g.setColour(juce::Colour(mst::theme::tabLim).withAlpha(0.85f));
        g.drawText("-14",
                   (int)yAxisCol.getX() + 2, (int)yTarget - 6,
                   (int)yAxisCol.getWidth() - 4, 12,
                   juce::Justification::centredRight);
    }
}

void LoudnessGraph::resized() {}

void LoudnessGraph::pushSample(float lufsValue) {
    history.push_back(lufsValue);
    if ((int)history.size() > maxSamples) {
        history.pop_front();
    }
    repaint();
}
