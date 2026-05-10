#pragma once
#include <JuceHeader.h>
#include <deque>
#include <atomic>

class EbuR128Meter {
public:
    struct Biquad {
        double b0=1, b1=0, b2=0, a1=0, a2=0, z1=0, z2=0;
        double tick (double x) {
            double factorForB0 = x - a1 * z1 - a2 * z2;
            double y = b0 * factorForB0 + b1 * z1 + b2 * z2;
            z2 = z1;
            z1 = factorForB0;
            return y;
        }
    };

public:
    void prepare (double sampleRate, int numChannels);
    void process (const juce::AudioBuffer<float>& buffer);
    void processPlain (const float* const* channelPtrs, int numChannels, int numSamples);
    void reset();

    float getIntegratedLufs() const  { return integratedLufs.load(); }
    float getShortTermLufs() const   { return shortTermLufs.load(); }
    float getMomentaryLufs() const   { return momentaryLufs.load(); }
    float getLoudnessRange() const   { return loudnessRange.load(); }
    float getTruePeakDbfs() const    { return truePeakDbfs.load(); }

private:
    std::vector<Biquad> kPre, kRlb;
    double sr = 48000.0;
    int channels = 2;

    int blockSamples400 = 0, blockSamples3s = 0;
    std::deque<double> momentaryBlocks;
    std::deque<double> integratedBlocks;
    double accum = 0.0;
    int accumN = 0;

    juce::dsp::Oversampling<float> oversampler { 2, 2,
        juce::dsp::Oversampling<float>::filterHalfBandPolyphaseIIR };

    std::atomic<float> integratedLufs { -std::numeric_limits<float>::infinity() };
    std::atomic<float> shortTermLufs  { -std::numeric_limits<float>::infinity() };
    std::atomic<float> momentaryLufs  { -std::numeric_limits<float>::infinity() };
    std::atomic<float> loudnessRange  { 0.0f };
    std::atomic<float> truePeakDbfs   { -std::numeric_limits<float>::infinity() };
};
