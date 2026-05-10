#include "MasteringEQ.h"
#include <cmath>

MasteringEQ::MasteringEQ() {
    bandTypes.fill(FilterType::Bypass);
    bandGains.fill(0.0f);
    bandFreqs.fill(1000.0f);
    bandQs.fill(0.707f);

    bandTypes[static_cast<int>(BandIndex::HP)] = FilterType::HighShelf;
    bandFreqs[static_cast<int>(BandIndex::HP)] = 32.0f;

    bandTypes[static_cast<int>(BandIndex::LS)] = FilterType::LowShelf;
    bandFreqs[static_cast<int>(BandIndex::LS)] = 120.0f;
    bandQs[static_cast<int>(BandIndex::LS)] = 0.9f;

    bandTypes[static_cast<int>(BandIndex::BELL1)] = FilterType::Peaking;
    bandFreqs[static_cast<int>(BandIndex::BELL1)] = 480.0f;
    bandQs[static_cast<int>(BandIndex::BELL1)] = 1.4f;

    bandTypes[static_cast<int>(BandIndex::BELL2)] = FilterType::Peaking;
    bandFreqs[static_cast<int>(BandIndex::BELL2)] = 2400.0f;
    bandQs[static_cast<int>(BandIndex::BELL2)] = 1.2f;

    bandTypes[static_cast<int>(BandIndex::HS)] = FilterType::HighShelf;
    bandFreqs[static_cast<int>(BandIndex::HS)] = 8200.0f;
    bandQs[static_cast<int>(BandIndex::HS)] = 0.8f;

    bandTypes[static_cast<int>(BandIndex::LP)] = FilterType::HighShelf;
    bandFreqs[static_cast<int>(BandIndex::LP)] = 18000.0f;
}

MasteringEQ::~MasteringEQ() = default;

void MasteringEQ::prepare (double sampleRate, int numChannels) {
    sr = sampleRate;
    this->numChannels = numChannels;
    filters.clear();
    filters.resize(numChannels);
    for (int i = 0; i < NUM_BANDS; ++i) {
        updateBandCoefficients(i);
    }
}

void MasteringEQ::reset() {
    for (auto& channelBands : filters) {
        for (auto& biquad : channelBands) {
            biquad.z1 = 0.0;
            biquad.z2 = 0.0;
        }
    }
}

void MasteringEQ::processBlock (juce::AudioBuffer<float>& buffer) {
    int numSamples = buffer.getNumSamples();
    for (int ch = 0; ch < juce::jmin((int)filters.size(), buffer.getNumChannels()); ++ch) {
        float* data = buffer.getWritePointer(ch);
        for (int i = 0; i < numSamples; ++i) {
            double sample = (double)data[i];
            for (int band = 0; band < NUM_BANDS; ++band) {
                sample = filters[ch][band].tick(sample);
            }
            data[i] = (float)sample;
        }
    }
}

void MasteringEQ::setBandType (int band, FilterType type) {
    if (band >= 0 && band < NUM_BANDS && type != bandTypes[band]) {
        bandTypes[band] = type;
        updateBandCoefficients(band);
    }
}

void MasteringEQ::setBandGain (int band, float gainDb) {
    if (band >= 0 && band < NUM_BANDS && gainDb != bandGains[band]) {
        bandGains[band] = gainDb;
        updateBandCoefficients(band);
    }
}

void MasteringEQ::setBandFrequency (int band, float freq) {
    if (band >= 0 && band < NUM_BANDS && freq != bandFreqs[band]) {
        bandFreqs[band] = freq;
        updateBandCoefficients(band);
    }
}

void MasteringEQ::setBandQ (int band, float q) {
    if (band >= 0 && band < NUM_BANDS && q != bandQs[band]) {
        bandQs[band] = q;
        updateBandCoefficients(band);
    }
}

void MasteringEQ::updateBandCoefficients(int band) {
    if (band < 0 || band >= NUM_BANDS) return;

    Biquad tempBiquad;
    computeBiquadCoefficients(bandTypes[band], bandGains[band], bandFreqs[band], bandQs[band], tempBiquad);

    for (auto& channelBands : filters) {
        channelBands[band] = tempBiquad;
    }
}

void MasteringEQ::computeBiquadCoefficients(FilterType type, float gainDb, float freq, float q, Biquad& biquad) {
    const double pi = 3.14159265358979323846;

    if (type == FilterType::Bypass) {
        biquad.b0 = 1.0; biquad.b1 = 0.0; biquad.b2 = 0.0;
        biquad.a1 = 0.0; biquad.a2 = 0.0;
        return;
    }

    double w0 = 2.0 * pi * freq / sr;
    double alpha = std::sin(w0) / (2.0 * q);
    double A = std::pow(10.0, gainDb / 40.0);

    double b0, b1, b2, a0, a1, a2;

    if (type == FilterType::HighShelf) {
        b0 = A * ((A + 1) + (A - 1) * std::cos(w0) + 2 * std::sqrt(A) * alpha);
        b1 = -2 * A * ((A - 1) + (A + 1) * std::cos(w0));
        b2 = A * ((A + 1) + (A - 1) * std::cos(w0) - 2 * std::sqrt(A) * alpha);
        a0 = (A + 1) - (A - 1) * std::cos(w0) + 2 * std::sqrt(A) * alpha;
        a1 = 2 * ((A - 1) - (A + 1) * std::cos(w0));
        a2 = (A + 1) - (A - 1) * std::cos(w0) - 2 * std::sqrt(A) * alpha;
    } else if (type == FilterType::LowShelf) {
        b0 = A * ((A + 1) - (A - 1) * std::cos(w0) + 2 * std::sqrt(A) * alpha);
        b1 = 2 * A * ((A - 1) - (A + 1) * std::cos(w0));
        b2 = A * ((A + 1) - (A - 1) * std::cos(w0) - 2 * std::sqrt(A) * alpha);
        a0 = (A + 1) + (A - 1) * std::cos(w0) + 2 * std::sqrt(A) * alpha;
        a1 = -2 * ((A - 1) + (A + 1) * std::cos(w0));
        a2 = (A + 1) + (A - 1) * std::cos(w0) - 2 * std::sqrt(A) * alpha;
    } else {
        b0 = 1.0 + alpha * A;
        b1 = -2.0 * std::cos(w0);
        b2 = 1.0 - alpha * A;
        a0 = 1.0 + alpha / A;
        a1 = -2.0 * std::cos(w0);
        a2 = 1.0 - alpha / A;
    }

    b0 /= a0; b1 /= a0; b2 /= a0;
    a1 /= a0; a2 /= a0;

    biquad.b0 = b0; biquad.b1 = b1; biquad.b2 = b2;
    biquad.a1 = a1; biquad.a2 = a2;
}

float MasteringEQ::getFrequencyResponse(float hz) {
    using complex = std::complex<double>;
    const double pi = 3.14159265358979323846;

    double w = 2.0 * pi * hz / sr;
    complex H(1.0, 0.0);

    for (int band = 0; band < NUM_BANDS; ++band) {
        if (bandTypes[band] == FilterType::Bypass) continue;

        Biquad dummy;
        computeBiquadCoefficients(bandTypes[band], bandGains[band], bandFreqs[band], bandQs[band], dummy);

        complex z = complex(std::cos(w), std::sin(w));
        complex num = complex(dummy.b0) + complex(dummy.b1) * z + complex(dummy.b2) * z * z;
        complex denom = complex(1.0) + complex(dummy.a1) * z + complex(dummy.a2) * z * z;
        H *= num / denom;
    }

    return (float)std::abs(H);
}
