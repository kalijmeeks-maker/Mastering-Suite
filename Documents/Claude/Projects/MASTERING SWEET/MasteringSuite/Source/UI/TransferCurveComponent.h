#pragma once
#include <JuceHeader.h>
#include "Theme.h"

class TransferCurveComponent : public juce::Component {
public:
    enum class Mode { Comp, Exp, Up };
    
    TransferCurveComponent();
    ~TransferCurveComponent() override = default;

    void paint(juce::Graphics& g) override;
    void setParameters(float threshold, float ratio, float knee, Mode mode);

private:
    float thresholdDb = -18.0f;
    float ratioDb = 2.0f;
    float kneeDb = 6.0f;
    Mode compressionMode = Mode::Comp;
    juce::Path curvePath;
    
    float computeGain(float inputDb) const;
    void rebuildCurve();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TransferCurveComponent)
};
