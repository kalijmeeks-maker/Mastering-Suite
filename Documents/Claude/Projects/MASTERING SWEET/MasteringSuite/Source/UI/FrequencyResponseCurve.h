#pragma once
#include <JuceHeader.h>
#include "../Processing/MasteringEQ.h"
#include "Theme.h"

class FrequencyResponseCurve : public juce::Component
{
public:
    explicit FrequencyResponseCurve(MasteringEQ& eq);

    void paint(juce::Graphics& g) override;
    void resized() override;
    void mouseDown(const juce::MouseEvent& event) override;
    void mouseDrag(const juce::MouseEvent& event) override;
    void mouseUp(const juce::MouseEvent& event) override;

private:
    void updateCurve();
    int getHoveredBandIndex(juce::Point<int> pos);
    float screenXToFrequency(float screenX);
    float frequencyToScreenX(float freq);
    float magnitudeToScreenY(float magDb);
    float screenYToMagnitude(float screenY);

    MasteringEQ& eq;
    std::vector<juce::Point<float>> curvePoints;
    std::vector<juce::Point<float>> bandControlPoints;
    int draggingBandIndex = -1;

    static constexpr int CONTROL_POINT_RADIUS = 6;
    static constexpr int NUM_CURVE_POINTS = 200;
};
