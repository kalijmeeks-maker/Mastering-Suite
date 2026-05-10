#pragma once
#include <JuceHeader.h>
#include <vector>
#include <array>
#include <complex>

class MasteringEQ {
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

    enum class FilterType { Bypass, HighShelf, LowShelf, Peaking };
    enum class BandIndex { HP=0, LS=1, BELL1=2, BELL2=3, HS=4, LP=5 };

    MasteringEQ();
    ~MasteringEQ();

    void prepare (double sampleRate, int numChannels);
    void processBlock (juce::AudioBuffer<float>& buffer);
    void reset();

    void setBandType (int band, FilterType type);
    void setBandGain (int band, float gainDb);
    void setBandFrequency (int band, float freq);
    void setBandQ (int band, float q);

    float getFrequencyResponse (float hz);

    float getLatencySamples() const { return 0.0f; }

private:
    void updateBandCoefficients (int band);
    void computeBiquadCoefficients (FilterType type, float gainDb, float freq, float q, Biquad& biquad);

    static constexpr int NUM_BANDS = 6;
    std::array<FilterType, NUM_BANDS> bandTypes;
    std::array<float, NUM_BANDS> bandGains, bandFreqs, bandQs;

    double sr = 48000.0;
    int numChannels = 2;

    std::vector<std::array<Biquad, NUM_BANDS>> filters;
};
