#pragma once
#include <JuceHeader.h>
#include <atomic>
#include <vector>

class MasteringLimiter {
public:
    enum class Style { Transparent, Punchy, Warm };

    MasteringLimiter();
    ~MasteringLimiter();

    void prepare (double sampleRate, int numChannels, int samplesPerBlock = 512);
    void processBlock (juce::AudioBuffer<float>& buffer);
    void reset();

    void setThreshold (float dbfs);
    void setRelease (float timeMs);
    void setCeiling (float dbfs);
    void setMakeupGain (float dbfs);
    void setEnableTruePeak (bool enable) { useTruePeak = enable; }
    void setStyle (Style s) { style = s; }

    float getLatencySamples() const { return truePeakLatency; }
    float getCurrentGainReduction() const { return gainReductionDb.load(); }

private:
    void processSampleWithTruePeak (juce::AudioBuffer<float>& buffer);
    void processSampleSimple (juce::AudioBuffer<float>& buffer);

    double sr = 48000.0;
    int numChannels = 2;
    bool isInitialized = false;

    Style style = Style::Transparent;
    float thresholdDbfs = 0.0f;
    float releaseTimeMs = 100.0f;
    float ceilingDbfs = -0.1f;
    float makeupGainDb = 0.0f;

    std::atomic<float> gainReductionDb { 0.0f };
    float currentGainReductionDb = 0.0f;
    double releaseCoeff = 0.995;
    double attackCoeff = 0.0;

    bool useTruePeak = true;
    std::unique_ptr<juce::dsp::Oversampling<float>> oversampler;
    float truePeakLatency = 0.0f;
};
