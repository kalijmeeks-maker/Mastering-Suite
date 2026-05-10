#include "LUFSDisplay.h"
#include "Colors.h"

using namespace MasteringSuiteColors;

LUFSDisplay::LUFSDisplay(EbuR128Meter& meterRef) : meter(meterRef)
{
    startTimer(100);
}

LUFSDisplay::~LUFSDisplay()
{
    stopTimer();
}

void LUFSDisplay::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(ColorBackground));

    auto bounds = getLocalBounds().toFloat();
    g.setFont(juce::FontOptions(18.0f));
    g.setColour(juce::Colour(ColorLUFSAccent));

    auto thirdWidth = bounds.getWidth() / 3.0f;

    auto formatLUFS = [](double value) -> juce::String {
        if (value < -200.0)
            return "-inf";
        return juce::String(value, 1);
    };

    // Momentary (left third)
    g.drawFittedText("M: " + formatLUFS(momentary) + " LU",
                     juce::Rectangle<float>(bounds.getX(), bounds.getY(), thirdWidth, bounds.getHeight()).toNearestInt(),
                     juce::Justification::centred, 1);

    // Short-term (middle third)
    g.drawFittedText("S: " + formatLUFS(shortTerm) + " LU",
                     juce::Rectangle<float>(bounds.getX() + thirdWidth, bounds.getY(), thirdWidth, bounds.getHeight()).toNearestInt(),
                     juce::Justification::centred, 1);

    // Integrated (right third)
    g.drawFittedText("I: " + formatLUFS(integrated) + " LU",
                     juce::Rectangle<float>(bounds.getX() + thirdWidth * 2, bounds.getY(), thirdWidth, bounds.getHeight()).toNearestInt(),
                     juce::Justification::centred, 1);
}

void LUFSDisplay::resized()
{
}

void LUFSDisplay::timerCallback()
{
    auto newMomentary = meter.getMomentaryLufs();
    auto newShortTerm = meter.getShortTermLufs();
    auto newIntegrated = meter.getIntegratedLufs();

    if (newMomentary != momentary || newShortTerm != shortTerm || newIntegrated != integrated)
    {
        momentary = newMomentary;
        shortTerm = newShortTerm;
        integrated = newIntegrated;
        repaint();
    }
}
