#pragma once
#include <JuceHeader.h>
#include "Metering/EbuR128Meter.h"
#include "Processing/MasteringEQ.h"
#include "Processing/MasteringLimiter.h"
#include "Processing/MasteringCompressor.h"

class MasteringSuiteProcessor : public juce::AudioProcessor {
public:
    MasteringSuiteProcessor();
    ~MasteringSuiteProcessor() override = default;

    void prepareToPlay (double sr, int bs) override;
    void releaseResources() override {}
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "MasteringSuite"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    double getTailLengthSeconds() const override { return 0.5; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return "Default"; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock&) override;
    void setStateInformation (const void*, int) override;

    // Professional parameter management via APVTS
    juce::AudioProcessorValueTreeState& getAPVTS() { return apvts; }

    EbuR128Meter& getMeter() { return meter; }
    MasteringEQ& getEQ() { return eq; }
    MasteringLimiter& getLimiter() { return limiter; }
    MasteringCompressor& getCompressor() { return compressor; }

private:
    // Create parameter layout for APVTS
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    // Professional state management
    juce::UndoManager undoManager;
    juce::AudioProcessorValueTreeState apvts;

    // DSP Modules
    EbuR128Meter meter;
    MasteringEQ eq;
    MasteringCompressor compressor;
    MasteringLimiter limiter;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MasteringSuiteProcessor)
};
