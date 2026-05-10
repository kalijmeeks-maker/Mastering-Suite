#pragma once
#include <JuceHeader.h>
#include <atomic>

class MasteringCompressor {
public:
    enum class Mode { Comp, Exp, Up };

    MasteringCompressor();
    ~MasteringCompressor() = default;

    void prepare (double sampleRate, int numChannels);
    void processBlock (juce::AudioBuffer<float>& buffer);
    void reset();

    void setMode (Mode m);
    void setThreshold (float dBFS);
    void setRatio (float ratio);
    void setKnee (float dB);
    void setAttack (float ms);
    void setRelease (float ms);
    void setMakeup (float dB);
    void setMix (float zeroToOne);

    float getCurrentGainReduction() const { return gainReductionDb.load(); }

private:
    float computeGain (float inputDb) const;

    Mode mode = Mode::Comp;
    float thresholdDb = -18.0f;
    float ratio = 2.0f;
    float kneeDb = 6.0f;
    float attackMs = 14.0f;
    float releaseMs = 120.0f;
    float makeupDb = 0.0f;
    float mix = 1.0f;

    double sr = 48000.0;
    int numChannels = 2;

    float attackCoeff = 0.0f;
    float releaseCoeff = 0.0f;
    float envDb = 0.0f;

    std::atomic<float> gainReductionDb { 0.0f };
};
