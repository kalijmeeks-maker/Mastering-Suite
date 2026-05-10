#include "EbuR128Meter_TestInterface.h"
#include <cmath>
#include <algorithm>

namespace {
    const double PI = 3.14159265358979323846;

    void designKPre (EbuR128Meter_TestInterface::Biquad& f, double fs) {
        const double f0 = 1681.974450955533, G = 3.999843853973347, Q = 0.7071752369554196;
        double K = std::tan (PI * f0 / fs);
        double Vh = std::pow (10.0, G / 20.0), Vb = std::pow (Vh, 0.4996667741545416);
        double a0_ = 1 + K / Q + K * K;
        f.b0 = (Vh + Vb * K / Q + K * K) / a0_;
        f.b1 = 2 * (K * K - Vh) / a0_;
        f.b2 = (Vh - Vb * K / Q + K * K) / a0_;
        f.a1 = 2 * (K * K - 1) / a0_;
        f.a2 = (1 - K / Q + K * K) / a0_;
    }

    void designKRlb (EbuR128Meter_TestInterface::Biquad& f, double fs) {
        const double f0 = 38.13547087602444, Q = 0.5003270373238773;
        double K = std::tan (PI * f0 / fs);
        double a0_ = 1 + K / Q + K * K;
        f.b0 = 1 / a0_;
        f.b1 = -2 / a0_;
        f.b2 = 1 / a0_;
        f.a1 = 2 * (K * K - 1) / a0_;
        f.a2 = (1 - K / Q + K * K) / a0_;
    }
}

void EbuR128Meter_TestInterface::prepare (double sampleRate, int numChannels) {
    sr = sampleRate;
    channels = numChannels;
    kPre.assign (numChannels, {});
    kRlb.assign (numChannels, {});
    for (auto& f : kPre) designKPre (f, sr);
    for (auto& f : kRlb) designKRlb (f, sr);
    blockSamples400 = (int) std::round (0.400 * sr);
    blockSamples3s  = (int) std::round (3.000 * sr);
    reset();
}

void EbuR128Meter_TestInterface::reset() {
    momentaryBlocks.clear();
    integratedBlocks.clear();
    accum = 0.0;
    accumN = 0;
}

void EbuR128Meter_TestInterface::processPlain (const float* const* channelPtrs, int numChannels, int numSamples) {
    const int ch = std::min (numChannels, channels);

    // Accumulate weighted sum of squared samples per channel
    for (int c = 0; c < ch; ++c) {
        const float* x = channelPtrs[c];
        const double w = (c < 2) ? 1.0 : 1.41;
        for (int i = 0; i < numSamples; ++i) {
            double s = kRlb[c].tick (kPre[c].tick ((double) x[i]));
            accum += w * s * s;
        }
    }
    accumN += numSamples;

    const int hop = (int) std::round (0.100 * sr);
    while (accumN >= hop) {
        double meanSq = accum / accumN;
        accum = 0.0;
        accumN = 0;
        momentaryBlocks.push_back (meanSq);
        if ((int) momentaryBlocks.size() > 4) momentaryBlocks.pop_front();
        integratedBlocks.push_back (meanSq);

        auto lufs = [] (double m) { return m > 0 ? -0.691 + 10.0 * std::log10 (m) : -INFINITY; };

        double mAvg = 0;
        for (auto v : momentaryBlocks) mAvg += v;
        mAvg /= momentaryBlocks.size();
        momentaryLufs.store ((float) lufs (mAvg));

        size_t st = (size_t) std::min<int> ((int) integratedBlocks.size(), 30);
        double sAvg = 0;
        for (size_t i = integratedBlocks.size() - st; i < integratedBlocks.size(); ++i) sAvg += integratedBlocks[i];
        sAvg /= st;
        shortTermLufs.store ((float) lufs (sAvg));

        std::vector<double> kept;
        kept.reserve (integratedBlocks.size());
        for (auto v : integratedBlocks) {
            if (lufs (v) > -70.0) kept.push_back (v);
        }
        if (!kept.empty()) {
            double mean = 0;
            for (auto v : kept) mean += v;
            mean /= kept.size();
            double rel = lufs (mean) - 10.0;
            double mean2 = 0;
            int n2 = 0;
            for (auto v : kept) {
                if (lufs (v) > rel) { mean2 += v; ++n2; }
            }
            if (n2 > 0) integratedLufs.store ((float) lufs (mean2 / n2));
        }
    }
}
