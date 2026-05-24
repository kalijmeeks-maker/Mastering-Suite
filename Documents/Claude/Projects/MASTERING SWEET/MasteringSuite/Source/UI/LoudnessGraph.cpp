#include "LoudnessGraph.h"
#include "../PluginProcessor.h"

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
    g.drawText(juce::String(juce::CharPointer_UTF8(u8"LOUDNESS HISTORY · 30 SEC")), 14, 8, bounds.getWidth() - 28, 14, juce::Justification::topLeft);

    g.setFont(juce::Font(8.0f));
    g.setColour(juce::Colour(mst::theme::textLow));
    g.drawText("SHORT-TERM LUFS", 14, 20, bounds.getWidth() - 28, 10, juce::Justification::topLeft);

    // Graph area with gradient bars
    auto graphArea = bounds.reduced(14, 40);
    const int barWidth = juce::jmax(1, (int)(graphArea.getWidth() / (float)maxSamples));

    for (size_t i = 0; i < history.size(); ++i) {
        float normalized = (history[i] + 23.0f) / 30.0f; // -23 to 7 LUFS
        normalized = juce::jlimit(0.0f, 1.0f, normalized);

        // Magenta to violet gradient per bar
        auto barX = graphArea.getX() + (i * barWidth);
        juce::ColourGradient barGrad(
            juce::Colour(0xFFFF00FF), barX, graphArea.getY(),
            juce::Colour(0xFFBB00FF), barX, graphArea.getBottom(),
            false);
        g.setGradientFill(barGrad);

        float barHeight = graphArea.getHeight() * normalized;
        g.fillRect(barX, graphArea.getBottom() - barHeight, (float)barWidth - 1, barHeight);
    }

    // Reference line at -14 LUFS (mint green, dashed)
    const float refNorm = (-14.0f + 23.0f) / 30.0f;
    const float refY = graphArea.getBottom() - (graphArea.getHeight() * refNorm);
    g.setColour(juce::Colour(mst::theme::cLimMakeup).withAlpha(0.6f));
    for (float x = graphArea.getX(); x < graphArea.getRight(); x += 4.0f) {
        g.drawLine(x, refY, x + 2.0f, refY, 1.0f);
    }

    // Y-axis labels (-6 to -26 LUFS)
    g.setFont(juce::Font(7.0f));
    g.setColour(juce::Colour(mst::theme::textLow));
    for (int lufs = -6; lufs >= -26; lufs -= 4) {
        float yNorm = (-lufs + 23.0f) / 30.0f;
        float y = graphArea.getBottom() - (graphArea.getHeight() * yNorm);
        g.drawText(juce::String(lufs), bounds.getX(), (int)(y - 4), 12, 8, juce::Justification::centredRight);
    }

    // X-axis labels (-30s to now)
    g.setFont(juce::Font(7.0f));
    g.setColour(juce::Colour(mst::theme::textLow));
    g.drawText("-30s", bounds.getX() + 14, (int)(graphArea.getBottom() + 2), 30, 10, juce::Justification::topLeft);
    g.drawText("now", bounds.getRight() - 44, (int)(graphArea.getBottom() + 2), 30, 10, juce::Justification::topRight);
}

void LoudnessGraph::resized() {}

void LoudnessGraph::pushSample(float lufsValue) {
    history.push_back(lufsValue);
    if (history.size() > maxSamples) {
        history.pop_front();
    }
    repaint();
}
