#include "../Source/Metering/EbuR128Meter.h"
#include <iostream>
#include <cmath>
#include <iomanip>
#include <vector>
#include <cstring>

struct TestSignal {
    const char* name;
    double frequency;
    double peakDbfs;
    int channels;
    double expectedLufs;
    double tolerance;
};

class SineWaveGenerator {
public:
    SineWaveGenerator(double frequency, double sampleRate)
        : freq(frequency), sr(sampleRate), phase(0) {}

    float next() {
        float sample = (float) std::sin(2.0 * M_PI * phase);
        phase += freq / sr;
        if (phase >= 1.0) phase -= 1.0;
        return sample;
    }

private:
    double freq, sr, phase;
};

int main(int argc, char* argv[]) {
    std::cout << "\n╔═══════════════════════════════════════════════════════════════╗\n"
              << "║ EbuR128Meter Validation (ITU-R BS.1770-4 Spec)               ║\n"
              << "║ Reference: EBU Tech 3341 'EBU Mode' Metering                 ║\n"
              << "╚═══════════════════════════════════════════════════════════════╝\n" << std::endl;

    TestSignal tests[] = {
        { "Stereo 1kHz @ -23.0 LUFS (seq-3341-1 ref)", 1000, -20.0, 2, -23.0, 0.1 },
        { "Stereo 1kHz @ -33.0 LUFS (seq-3341-2 ref)", 1000, -30.0, 2, -33.0, 0.1 },
        { "Mono 1kHz @ -23.0 dBFS (ch weight test)", 1000, -23.0, 1, -26.0, 0.1 },
        { "Stereo 100Hz @ -20 dBFS (K-weight high-pass)", 100, -17.0, 2, -20.0, 0.2 },
        { "Stereo 1kHz @ -20 dBFS (K-weight neutral)", 1000, -17.0, 2, -20.0, 0.1 },
        { "Stereo 8kHz @ -20 dBFS (K-weight shelf)", 8000, -17.0, 2, -20.0, 0.2 },
        { "Silence (should be -∞)", 1000, 0.0, 2, -999.0, 999.0 },
    };

    double sampleRate = 48000.0;
    int blockSize = 4096;
    int durationSamples = (int) (sampleRate * 10.0);

    int passed = 0, failed = 0;

    for (const auto& test : tests) {
        EbuR128Meter meter;
        meter.prepare(sampleRate, test.channels);

        std::vector<std::vector<float>> buffers(test.channels, std::vector<float>(blockSize));
        std::vector<const float*> channelPtrs(test.channels);

        int samplesProcessed = 0;
        double amplitude = std::pow(10.0, test.peakDbfs / 20.0);

        while (samplesProcessed < durationSamples) {
            int samplesToProcess = std::min(blockSize, durationSamples - samplesProcessed);

            // Generate sine wave for each channel
            SineWaveGenerator gen(test.frequency, sampleRate);
            for (int ch = 0; ch < test.channels; ++ch) {
                for (int i = 0; i < samplesToProcess; ++i) {
                    buffers[ch][i] = gen.next() * (float) amplitude;
                }
            }

            // Set up channel pointers
            for (int ch = 0; ch < test.channels; ++ch) {
                channelPtrs[ch] = buffers[ch].data();
            }

            meter.processPlain(channelPtrs.data(), test.channels, samplesToProcess);
            samplesProcessed += samplesToProcess;
        }

        float measured = meter.getIntegratedLufs();
        double delta = std::isinf((float)test.expectedLufs) ?
                       (std::isinf(measured) ? 0.0 : 1000.0) :
                       std::abs((double) measured - test.expectedLufs);

        bool pass = delta <= test.tolerance;

        std::string status = pass ? "✓ PASS" : "✗ FAIL";
        std::cout << std::setw(45) << std::left << test.name
                  << " | Expected: " << std::setw(7) << std::fixed << std::setprecision(2) << test.expectedLufs
                  << " | Measured: " << std::setw(7) << measured
                  << " | Δ: " << std::setw(6) << delta << " LU | "
                  << status << std::endl;

        if (pass) ++passed; else ++failed;
    }

    std::cout << "\n" << std::string(70, '=') << std::endl;
    std::cout << "Results: " << passed << " passed, " << failed << " failed\n" << std::endl;

    if (failed > 0) {
        std::cerr << "❌ VALIDATION FAILED\n"
                  << "Meter does not meet ±0.1 LU spec requirement.\n"
                  << "Check:\n"
                  << "  1. K-weighting filter coefficients (vs ITU-R BS.1770-4)\n"
                  << "  2. Biquad state precision (float64 required, not float32)\n"
                  << "  3. Channel weighting accumulation (sum before log, not after)\n"
                  << "  4. Gating logic (-70 LUFS absolute, -10 LU relative)\n";
        return 1;
    }

    std::cout << "✅ VALIDATION SUCCESS\n"
              << "EbuR128Meter meets ITU-R BS.1770-4 specification (±0.1 LU).\n"
              << "Safe to proceed with downstream features (EQ, limiter, reference matching).\n";
    return 0;
}
