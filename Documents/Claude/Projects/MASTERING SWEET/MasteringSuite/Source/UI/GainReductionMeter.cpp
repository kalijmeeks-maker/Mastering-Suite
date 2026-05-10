#include "GainReductionMeter.h"
#include "Colors.h"

using namespace MasteringSuiteColors;

GainReductionMeter::GainReductionMeter(MasteringLimiter& limiterRef) : limiter(limiterRef)
{
    startTimer(33);
}

GainReductionMeter::~GainReductionMeter()
{
    stopTimer();
}

void GainReductionMeter::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    g.fillAll(juce::Colour(ColorBackground));

    auto segmentHeight = bounds.getHeight() * 0.85f;
    auto spacingHeight = bounds.getHeight() * 0.15f;
    auto y = bounds.getY() + spacingHeight / 2.0f;

    for (int i = 0; i < NUM_SEGMENTS; ++i)
    {
        auto level = static_cast<double>(i) / NUM_SEGMENTS;
        auto isActive = currentGainReduction > level;
        auto colour = isActive ? getSegmentColour(currentGainReduction) : juce::Colour(ColorBorder).withAlpha(0.3f);

        auto x = bounds.getX() + (i * (SEGMENT_WIDTH + 1));

        g.setColour(colour);
        g.fillRect(x, y, static_cast<float>(SEGMENT_WIDTH), segmentHeight);
    }

    // Draw peak hold indicator
    if (peakHold > 0.0)
    {
        int peakSegment = static_cast<int>((peakHold * NUM_SEGMENTS));
        peakSegment = juce::jlimit(0, NUM_SEGMENTS - 1, peakSegment);

        auto x = bounds.getX() + (peakSegment * (SEGMENT_WIDTH + 1));
        g.setColour(juce::Colour(ColorClipDanger).withAlpha(0.7f));
        g.drawRect(x, y, static_cast<float>(SEGMENT_WIDTH), segmentHeight, 1.5f);
    }
}

void GainReductionMeter::resized()
{
}

void GainReductionMeter::timerCallback()
{
    auto newGainReduction = -limiter.getCurrentGainReduction() / 20.0;
    newGainReduction = juce::jlimit(0.0, 1.0, newGainReduction);

    if (newGainReduction != currentGainReduction)
    {
        currentGainReduction = newGainReduction;

        if (newGainReduction > peakHold)
        {
            peakHold = newGainReduction;
            peakHoldCounter = PEAK_HOLD_TIME;
        }

        repaint();
    }

    // Decay peak hold
    if (peakHoldCounter > 0)
    {
        peakHoldCounter--;
        if (peakHoldCounter == 0)
            peakHold = 0.0;
    }
}

juce::Colour GainReductionMeter::getSegmentColour(double gainReduction)
{
    if (gainReduction < 0.3)
        return juce::Colour(ColorGainReduction);
    else if (gainReduction < 0.6)
        return juce::Colour(ColorPeakWarning);
    else
        return juce::Colour(ColorClipDanger);
}
