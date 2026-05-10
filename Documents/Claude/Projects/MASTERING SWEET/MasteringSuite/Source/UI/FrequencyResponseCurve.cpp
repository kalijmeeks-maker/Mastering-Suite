#include "FrequencyResponseCurve.h"

FrequencyResponseCurve::FrequencyResponseCurve(MasteringEQ& eqRef) : eq(eqRef)
{
    curvePoints.reserve(NUM_CURVE_POINTS);
    bandControlPoints.resize(6);
    updateCurve();
}

void FrequencyResponseCurve::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    g.fillAll(Theme::Color::toColour(Theme::Color::BG_2));

    // Draw frequency grid (log scale)
    g.setColour(Theme::Color::toColour(Theme::Color::LINE_2).withAlpha(0.3f));
    for (int i = 0; i <= 4; ++i) {
        float normalised = i / 4.0f;
        float freq = 20.0f * std::pow(1000.0f, normalised);
        float x = bounds.getX() + (normalised * bounds.getWidth());
        g.drawLine(x, bounds.getY(), x, bounds.getBottom(), 0.5f);
    }

    // Draw gain grid
    for (int i = -12; i <= 12; i += 6) {
        float normalised = (i + 12.0f) / 24.0f;
        float y = bounds.getBottom() - (normalised * bounds.getHeight());
        g.drawLine(bounds.getX(), y, bounds.getRight(), y, 0.5f);
    }

    // Draw 0dB reference line
    {
        float y = bounds.getBottom() - (0.5f * bounds.getHeight());
        g.setColour(Theme::Color::toColour(Theme::Color::LINE).withAlpha(0.5f));
        g.drawLine(bounds.getX(), y, bounds.getRight(), y, 1.0f);
    }

    // Draw combined frequency response curve
    if (curvePoints.size() > 1) {
        g.setColour(Theme::Color::toColour(Theme::Color::ACCENT));
        juce::Path curve;
        for (size_t i = 0; i < curvePoints.size(); ++i) {
            auto pt = curvePoints[i];
            if (i == 0)
                curve.startNewSubPath(pt.x, pt.y);
            else
                curve.lineTo(pt.x, pt.y);
        }
        g.strokePath(curve, juce::PathStrokeType(2.5f, juce::PathStrokeType::curved));
    }

    // Draw band control points with colors
    const juce::uint32 bandColors[] = {
        Theme::Color::TEXT,      // HP: neutral
        Theme::Color::CYAN,      // LS: cyan
        Theme::Color::ACCENT,    // BELL1: green
        Theme::Color::MAGENTA,   // BELL2: magenta
        Theme::Color::AMBER,     // HS: amber
        Theme::Color::TEXT       // LP: neutral
    };

    for (int i = 0; i < 6; ++i) {
        g.setColour(Theme::Color::toColour(bandColors[i]));
        auto pt = bandControlPoints[i];
        g.fillEllipse(pt.x - CONTROL_POINT_RADIUS, pt.y - CONTROL_POINT_RADIUS,
                      CONTROL_POINT_RADIUS * 2, CONTROL_POINT_RADIUS * 2);

        g.setColour(Theme::Color::toColour(bandColors[i]).brighter(0.5f));
        g.drawEllipse(pt.x - CONTROL_POINT_RADIUS, pt.y - CONTROL_POINT_RADIUS,
                      CONTROL_POINT_RADIUS * 2, CONTROL_POINT_RADIUS * 2, 1.5f);
    }
}

void FrequencyResponseCurve::resized()
{
    updateCurve();
}

void FrequencyResponseCurve::updateCurve()
{
    auto bounds = getLocalBounds().toFloat();
    curvePoints.clear();

    for (int i = 0; i < NUM_CURVE_POINTS; ++i) {
        float normalised = i / (NUM_CURVE_POINTS - 1.0f);
        float freq = 20.0f * std::pow(1000.0f, normalised);
        float x = bounds.getX() + (normalised * bounds.getWidth());

        float magDb = 20.0f * std::log10(eq.getFrequencyResponse(freq) + 1e-6f);
        float y = magnitudeToScreenY(magDb);

        curvePoints.push_back({x, y});
    }

    // Update band control points
    for (int i = 0; i < 6; ++i) {
        float freq = 1000.0f;
        if (i == 0) freq = 32.0f;
        else if (i == 1) freq = 120.0f;
        else if (i == 2) freq = 480.0f;
        else if (i == 3) freq = 2400.0f;
        else if (i == 4) freq = 8200.0f;
        else if (i == 5) freq = 18000.0f;

        float x = frequencyToScreenX(freq);
        float magDb = 20.0f * std::log10(eq.getFrequencyResponse(freq) + 1e-6f);
        float y = magnitudeToScreenY(magDb);

        bandControlPoints[i] = {x, y};
    }
}

int FrequencyResponseCurve::getHoveredBandIndex(juce::Point<int> pos)
{
    for (int i = 0; i < 6; ++i) {
        auto pt = bandControlPoints[i];
        float dx = pos.x - pt.x;
        float dy = pos.y - pt.y;
        float dist = std::sqrt(dx * dx + dy * dy);
        if (dist <= CONTROL_POINT_RADIUS + 4) {
            return i;
        }
    }
    return -1;
}

float FrequencyResponseCurve::screenXToFrequency(float screenX)
{
    auto bounds = getLocalBounds().toFloat();
    float normalised = (screenX - bounds.getX()) / bounds.getWidth();
    normalised = juce::jlimit(0.0f, 1.0f, normalised);
    return 20.0f * std::pow(1000.0f, normalised);
}

float FrequencyResponseCurve::frequencyToScreenX(float freq)
{
    auto bounds = getLocalBounds().toFloat();
    float normalised = std::log10(freq / 20.0f) / std::log10(1000.0f);
    return bounds.getX() + (normalised * bounds.getWidth());
}

float FrequencyResponseCurve::magnitudeToScreenY(float magDb)
{
    auto bounds = getLocalBounds().toFloat();
    float normalised = (magDb + 12.0f) / 24.0f;
    normalised = juce::jlimit(0.0f, 1.0f, normalised);
    return bounds.getBottom() - (normalised * bounds.getHeight());
}

float FrequencyResponseCurve::screenYToMagnitude(float screenY)
{
    auto bounds = getLocalBounds().toFloat();
    float normalised = 1.0f - ((screenY - bounds.getY()) / bounds.getHeight());
    normalised = juce::jlimit(0.0f, 1.0f, normalised);
    return (normalised * 24.0f) - 12.0f;
}

void FrequencyResponseCurve::mouseDown(const juce::MouseEvent& event)
{
    draggingBandIndex = getHoveredBandIndex(event.getPosition());
}

void FrequencyResponseCurve::mouseDrag(const juce::MouseEvent& event)
{
    if (draggingBandIndex < 0 || draggingBandIndex >= 6) return;

    float freq = screenXToFrequency(event.getPosition().x);
    float magDb = screenYToMagnitude(event.getPosition().y);

    freq = juce::jlimit(20.0f, 20000.0f, freq);
    magDb = juce::jlimit(-12.0f, 12.0f, magDb);

    eq.setBandFrequency(draggingBandIndex, freq);
    eq.setBandGain(draggingBandIndex, magDb);

    repaint();
}

void FrequencyResponseCurve::mouseUp(const juce::MouseEvent& event)
{
    draggingBandIndex = -1;
}
