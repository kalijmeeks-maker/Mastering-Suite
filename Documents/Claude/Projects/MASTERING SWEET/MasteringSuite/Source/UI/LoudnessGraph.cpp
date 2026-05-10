#include "LoudnessGraph.h"
#include "Colors.h"

using namespace MasteringSuiteColors;

LoudnessGraph::LoudnessGraph(EbuR128Meter& meterRef) : meter(meterRef)
{
    buffer.fill(-200.0);
    startTimer(100);
}

LoudnessGraph::~LoudnessGraph()
{
    stopTimer();
}

void LoudnessGraph::paint(juce::Graphics& g)
{
    // Background
    g.fillAll(juce::Colour(ColorBackground));

    // Draw grid
    drawGrid(g);

    // Draw curve
    drawCurve(g);

    // Draw target line
    drawTargetLine(g);
}

void LoudnessGraph::resized()
{
}

void LoudnessGraph::timerCallback()
{
    auto integrated = meter.getIntegratedLufs();

    if (sampleCount < BUFFER_SIZE)
        sampleCount++;

    buffer[writeIndex] = integrated < -200.0 ? -200.0 : integrated;
    writeIndex = (writeIndex + 1) % BUFFER_SIZE;

    repaint();
}

void LoudnessGraph::drawGrid(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    auto yRange = MAX_LUFS - MIN_LUFS;

    g.setColour(juce::Colour(ColorGrid));
    g.setOpacity(0.3f);

    // Horizontal grid lines every 5 dB
    for (double lufs = MIN_LUFS; lufs <= MAX_LUFS; lufs += 5.0)
    {
        auto normalised = (lufs - MIN_LUFS) / yRange;
        auto y = bounds.getBottom() - (normalised * bounds.getHeight());

        g.drawLine(bounds.getX(), y, bounds.getRight(), y, 0.5f);
    }

    // Vertical grid lines (6 dividers = 0s, 15s, 30s, 45s, 60s)
    for (int i = 0; i <= 4; ++i)
    {
        auto x = bounds.getX() + (bounds.getWidth() / 4.0f) * i;
        g.drawLine(x, bounds.getY(), x, bounds.getBottom(), 0.5f);
    }
}

void LoudnessGraph::drawCurve(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    auto yRange = MAX_LUFS - MIN_LUFS;

    g.setColour(juce::Colour(ColorLUFSAccent));

    if (sampleCount < 2)
        return;

    juce::Path curve;

    for (int i = 0; i < sampleCount; ++i)
    {
        auto idx = (writeIndex + i) % BUFFER_SIZE;
        auto value = buffer[idx];

        if (value < -200.0)
            continue;

        auto normalised = (value - MIN_LUFS) / yRange;
        normalised = juce::jlimit(0.0, 1.0, normalised);

        auto x = bounds.getX() + (bounds.getWidth() / static_cast<float>(BUFFER_SIZE)) * i;
        auto y = bounds.getBottom() - (normalised * bounds.getHeight());

        if (i == 0)
            curve.startNewSubPath(x, y);
        else
            curve.lineTo(x, y);
    }

    g.strokePath(curve, juce::PathStrokeType(2.0f, juce::PathStrokeType::curved));
}

void LoudnessGraph::drawTargetLine(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    auto yRange = MAX_LUFS - MIN_LUFS;
    auto normalised = (TARGET_LUFS - MIN_LUFS) / yRange;
    auto y = bounds.getBottom() - (normalised * bounds.getHeight());

    g.setColour(juce::Colour(ColorGainReduction).withAlpha(0.6f));

    g.drawLine(bounds.getX(), y, bounds.getRight(), y, 1.5f);
}
