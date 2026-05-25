#pragma once
#include <JuceHeader.h>
#include "Metering/EbuR128Meter.h"
#include "Processing/MasteringEQ.h"
#include "Processing/MasteringLimiter.h"
#include "Processing/MasteringCompressor.h"
#include "Processing/MasteringImager.h"

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
    MasteringImager& getImager() { return imager; }

    float getCPUUsage() { return 0.012f; } // Placeholder

    float getInputPeak() const { return inputPeak.load(); }
    float getOutputPeak() const { return outputPeak.load(); }
    float getCorrelation() const { return correlation.load(); }
    bool getTruePeakActive() const { return truePeakActive.load(); }
    // Per-channel post-DSP level for the dual L+R meter (dBFS, decaying).
    float getChannelLevelL() const { return channelLevelL.load(); }
    float getChannelLevelR() const { return channelLevelR.load(); }

    // Goniometer data
    static constexpr int goniometerSize = 1024;
    void getGoniometerSamples(float* left, float* right) {
        const juce::ScopedLock sl(gonioLock);
        std::copy(gonioBufferL.begin(), gonioBufferL.end(), left);
        std::copy(gonioBufferR.begin(), gonioBufferR.end(), right);
    }

    // Spectrum Analyzer data access
    static constexpr int fftSize = 2048;
    void getNextFFTBlock(float* dest) {
        const juce::ScopedLock sl(fftLock);
        std::copy(scopeData.begin(), scopeData.end(), dest);
    }

private:
    // Spectrum Analyzer DSP
    juce::dsp::FFT fft { 11 }; // 2^11 = 2048
    juce::dsp::WindowingFunction<float> window { fftSize + 1, juce::dsp::WindowingFunction<float>::hann };
    std::array<float, fftSize> fifo;
    std::array<float, fftSize * 2> fftData;
    std::array<float, fftSize / 2> scopeData;
    int fifoIndex = 0;
    juce::CriticalSection fftLock;

    std::atomic<float> inputPeak { -100.0f };
    std::atomic<float> outputPeak { -100.0f };
    std::atomic<float> correlation { 1.0f };       // [-1..+1] L/R correlation, smoothed
    std::atomic<bool>  truePeakActive { false };   // limiter currently catching peaks
    std::atomic<float> channelLevelL { -100.0f };  // dBFS, post-DSP, slow-decay
    std::atomic<float> channelLevelR { -100.0f };

    // Goniometer DSP
    std::array<float, goniometerSize> gonioBufferL, gonioBufferR;
    int gonioIndex = 0;
    juce::CriticalSection gonioLock;

    void pushSampleIntoFifo(float sample);
    void pushGonioSamples(float l, float r);

    // Create parameter layout for APVTS
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    // Professional state management
    juce::UndoManager undoManager;
    juce::AudioProcessorValueTreeState apvts;

    // DSP Modules
    EbuR128Meter meter;
    MasteringEQ eq;
    MasteringCompressor compressor;
    MasteringImager imager;
    MasteringLimiter limiter;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MasteringSuiteProcessor)
};
