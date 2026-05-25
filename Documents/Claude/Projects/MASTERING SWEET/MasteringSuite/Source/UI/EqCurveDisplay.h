#pragma once
#include <JuceHeader.h>
#include "Theme.h"

class MasteringSuiteProcessor;

class EqCurveDisplay : public juce::Component, public juce::AudioProcessorValueTreeState::Listener {
public:
    explicit EqCurveDisplay(MasteringSuiteProcessor& proc);
    ~EqCurveDisplay() override;

    void paint(juce::Graphics& g) override;
    void resized() override;
    
    void parameterChanged(const juce::String& parameterID, float newValue) override;
    void refreshSpectrum();

    struct Handle {
        int index;
        juce::Point<float> pos;
        juce::Colour color;
        bool isDragging = false;
    };

    void mouseDown(const juce::MouseEvent& e) override;
    void mouseDrag(const juce::MouseEvent& e) override;
    void mouseUp(const juce::MouseEvent& e) override;
    void mouseDoubleClick(const juce::MouseEvent& e) override;
    void mouseMove(const juce::MouseEvent& e) override;
    void mouseExit(const juce::MouseEvent& e) override;
    void mouseWheelMove(const juce::MouseEvent& e, const juce::MouseWheelDetails& wheel) override;

private:
    MasteringSuiteProcessor& processor;
    
    juce::Path curvePath;
    bool pathDirty = true;
    
    std::vector<Handle> handles;
    int draggingHandleIndex = -1;
    
    std::vector<float> spectrumBuffer;
    // Per-bin peak-hold (dB) and the time it was last set. Decays over 1.5s
    // after a 200ms hold so dots read as "ghost peaks" above the live bars.
    std::vector<float>  peakHoldDb;
    std::vector<double> peakHoldTimeMs;

    int hoveredHandleIndex = -1;   // v1.0.1-H3: matches knob hover behavior

    // v1.0.1-H1: called from mouseDrag so adjacent EqBandCells repaint
    // synchronously instead of waiting for the next async parameter callback.
    void repaintSiblingCells();
    
    void updatePath();
    float freqToX(float freq);
    float xToFreq(float x);
    float gainToY(float gainDb);
    float yToGain(float y);
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EqCurveDisplay)
};
