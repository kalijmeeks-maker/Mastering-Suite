#include "LoudnessGraph.h"
#include "../PluginProcessor.h"

// Visible LUFS range for bar heights, Y-axis labels, and the target reference
// line. Read by three sites in paint() — keep them in sync via these constants.
// -48 dB picks up real noise-floor / pre-amp material without slamming the
// floor clamp; the streaming -14 target lands at ~70.8% bar height.
static constexpr float kBottomDb = -48.0f;
static constexpr float kTopDb    = 0.0f;
static constexpr float kRangeDb  = kTopDb - kBottomDb;
static constexpr float kTargetLufs = -14.0f;

LoudnessGraph::LoudnessGraph(MasteringSuiteProcessor& proc) : processor(proc) {}

void LoudnessGraph::paint(juce::Graphics& g) {
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
    g.drawText(juce::String::fromUTF8("LOUDNESS HISTORY \u00B7 30 SEC"), 14, 8, (int)bounds.getWidth() - 28, 14, juce::Justification::topLeft);

    // Legend (top-right): SHORT swatch + TARGET-14 swatch
    {
        auto legend = juce::Rectangle<float>(bounds.getRight() - 170.0f, 10.0f, 156.0f, 14.0f);
        g.setColour(juce::Colour(0xFFFF5CD1));
        g.fillRect(legend.getX(), legend.getCentreY() - 1.0f, 8.0f, 2.0f);
        g.setColour(juce::Colour(mst::theme::textLow));
        g.setFont(juce::Font(8.0f));
        g.drawText("SHORT", legend.getX() + 12.0f, legend.getY(), 50.0f, 14.0f, juce::Justification::centredLeft);
        g.setColour(juce::Colour(mst::theme::tabLim));
        g.fillRect(legend.getX() + 70.0f, legend.getCentreY() - 1.0f, 8.0f, 2.0f);
        g.setColour(juce::Colour(mst::theme::textLow));
        g.drawText("TARGET -14", legend.getX() + 82.0f, legend.getY(), 75.0f, 14.0f, juce::Justification::centredLeft);
    }

    // Graph area
    auto graphArea = bounds.reduced(40, 40).withX(35); // Room for labels on left
    const float barWidth = graphArea.getWidth() / (float)maxSamples;

    for (size_t i = 0; i < history.size(); ++i) {
        float normalized = (history[i] - kBottomDb) / kRangeDb;
        normalized = juce::jlimit(0.03f, 1.0f, normalized); // 3% min-height clamp

        auto barX = graphArea.getX() + (i * barWidth);
        juce::ColourGradient barGrad(
            juce::Colour(0xFFFF00FF), barX, graphArea.getY(),
            juce::Colour(0xFFBB00FF), barX, graphArea.getBottom(),
            false);
        g.setGradientFill(barGrad);

        float barHeight = graphArea.getHeight() * normalized;
        g.fillRect(barX, graphArea.getBottom() - barHeight, barWidth - 0.5f, barHeight);
    }

    // Reference line at streaming target (mint green, dashed). v1.1 #3 makes
    // kTargetLufs configurable; the only change there is the constant value.
    const float refY = graphArea.getBottom() - (graphArea.getHeight() * ((kTargetLufs - kBottomDb) / kRangeDb));
    g.setColour(juce::Colour(mst::theme::tabLim).withAlpha(0.4f));
    for (float x = graphArea.getX(); x < graphArea.getRight(); x += 4.0f) {
        g.drawLine(x, refY, x + 2.0f, refY, 1.0f);
    }

    // Y-axis labels in a proper column down the left side
    g.setFont(juce::Font(8.0f));
    g.setColour(juce::Colour(mst::theme::textLow));
    for (int lufs : {0, -8, -16, -24, -32, -40}) {
        float yNorm = ((float)lufs - kBottomDb) / kRangeDb;
        float y = graphArea.getBottom() - (graphArea.getHeight() * yNorm);
        g.drawText(juce::String(lufs), 5, (int)(y - 5), 25, 10, juce::Justification::centredRight);
    }

    // X-axis time labels along the bottom (v2 spec).
    g.setFont(juce::Font(8.0f));
    g.setColour(juce::Colour(mst::theme::textLow));
    const char* xLabels[] = { "-30s", "-24s", "-18s", "-12s", "-6s", "now" };
    const int nLabels = 6;
    for (int i = 0; i < nLabels; ++i) {
        float t = (float)i / (float)(nLabels - 1);
        float x = graphArea.getX() + t * graphArea.getWidth();
        auto j = (i == 0) ? juce::Justification::centredLeft
               : (i == nLabels - 1) ? juce::Justification::centredRight
               : juce::Justification::centred;
        g.drawText(xLabels[i], (int)x - 20, (int)graphArea.getBottom() + 3, 40, 10, j);
    }
}

void LoudnessGraph::resized() {}

void LoudnessGraph::pushSample(float lufsValue) {
    history.push_back(lufsValue);
    if (history.size() > maxSamples) {
        history.pop_front();
    }
    repaint();
}
