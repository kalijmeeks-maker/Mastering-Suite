#pragma once
#include <JuceHeader.h>
#include "RotaryKnob.h"
#include "SegmentedControl.h"
#include "TransferCurveComponent.h"
#include "CompressorGRMeter.h"
#include "PillButton.h"

class MasteringCompressor;

class DynamicsModule : public juce::Component, private juce::Timer {
public:
    DynamicsModule(MasteringCompressor& comp, juce::AudioProcessorValueTreeState& apvts);
    ~DynamicsModule() override;

    void paint(juce::Graphics& g) override;
    void resized() override;
    void timerCallback() override;

private:
    MasteringCompressor& compressor;
    juce::AudioProcessorValueTreeState& apvts;
    
    std::unique_ptr<SegmentedControl> modeControl;
    std::unique_ptr<RotaryKnob> thresholdKnob, ratioKnob, kneeKnob;
    std::unique_ptr<RotaryKnob> attackKnob, releaseKnob, makeupKnob, mixKnob;
    std::unique_ptr<TransferCurveComponent> transferCurve;
    std::unique_ptr<CompressorGRMeter> grMeter;
    std::vector<std::unique_ptr<PillButton>> toggleButtons;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DynamicsModule)
};
