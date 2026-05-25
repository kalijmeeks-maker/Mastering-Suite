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
    void mouseWheelMove(const juce::MouseEvent& e, const juce::MouseWheelDetails& wheel) override;

private:
    MasteringSuiteProcessor& processor;
    
    juce::Path curvePath;
    bool pathDirty = true;
    
    std::vector<Handle> handles;
    int draggingHandleIndex = -1;
    
    std::vector<float> spectrumBuffer;
    
    void updatePath();
    float freqToX(float freq);
    float xToFreq(float x);
    float gainToY(float gainDb);
    float yToGain(float y);
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EqCurveDisplay)
};
