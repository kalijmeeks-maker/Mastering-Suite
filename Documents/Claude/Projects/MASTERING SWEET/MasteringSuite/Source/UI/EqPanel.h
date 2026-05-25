#pragma once
#include <JuceHeader.h>
#include "Theme.h"
#include "NeonLookAndFeel.h"
#include "EqCurveDisplay.h"
#include "EqBandCell.h"

class MasteringSuiteProcessor;

class EqPanel : public juce::Component {
public:
    explicit EqPanel(MasteringSuiteProcessor& proc, NeonLookAndFeel& laf);
    ~EqPanel() override = default;

    void paint(juce::Graphics& g) override;
    void resized() override;
    void refreshSpectrum();

private:
    MasteringSuiteProcessor& processor;
    NeonLookAndFeel& lookAndFeel;

    std::unique_ptr<EqCurveDisplay> curveDisplay;
    std::vector<std::unique_ptr<EqBandCell>> bandCells;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EqPanel)
};

