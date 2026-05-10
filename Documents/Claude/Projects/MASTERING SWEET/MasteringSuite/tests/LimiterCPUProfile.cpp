#include <iostream>
#include <chrono>
#include <cmath>
#include <vector>
#include <iomanip>

class CPUProfiler {
private:
    std::chrono::high_resolution_clock::time_point start;
    std::chrono::high_resolution_clock::time_point end;

public:
    void begin() {
        start = std::chrono::high_resolution_clock::now();
    }

    void stop() {
        end = std::chrono::high_resolution_clock::now();
    }

    double getElapsedMicroseconds() const {
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        return static_cast<double>(duration.count());
    }

    double getElapsedMilliseconds() const {
        return getElapsedMicroseconds() / 1000.0;
    }
};

class SimpleLimiter {
public:
    void processBlock(float* buffer, int numSamples) {
        for (int i = 0; i < numSamples; ++i) {
            float level = std::abs(buffer[i]);
            if (level > threshold) {
                float gainDb = threshold - (20.0f * std::log10(level));
                float linearGain = std::pow(10.0f, gainDb / 20.0f);
                buffer[i] *= linearGain;
            }
        }
    }

private:
    float threshold = -6.0f;
};

class OversampledLimiter {
public:
    void processBlock(float* buffer, int numSamples) {
        // Simulate 4x oversampling overhead
        std::vector<float> upsampled(numSamples * 4);

        // Upsample: simple linear interpolation
        for (int i = 0; i < numSamples; ++i) {
            upsampled[i * 4] = buffer[i];
            if (i < numSamples - 1) {
                for (int j = 1; j < 4; ++j) {
                    float t = static_cast<float>(j) / 4.0f;
                    upsampled[i * 4 + j] = buffer[i] * (1.0f - t) + buffer[i + 1] * t;
                }
            }
        }

        // Process at 4x rate
        for (int i = 0; i < numSamples * 4; ++i) {
            float level = std::abs(upsampled[i]);
            if (level > threshold) {
                float gainDb = threshold - (20.0f * std::log10(level));
                float linearGain = std::pow(10.0f, gainDb / 20.0f);
                upsampled[i] *= linearGain;
            }
        }

        // Downsample: simple decimation
        for (int i = 0; i < numSamples; ++i) {
            buffer[i] = upsampled[i * 4];
        }
    }

private:
    float threshold = -6.0f;
};

int main() {
    std::cout << "\n╔═══════════════════════════════════════════════╗\n";
    std::cout << "║ Limiter CPU Profile - True-Peak vs Simple     ║\n";
    std::cout << "╚═══════════════════════════════════════════════╝\n\n";

    // Test parameters
    const int BLOCK_SIZE = 512;
    const int NUM_BLOCKS = 1000;
    const int SAMPLE_RATE = 48000;

    std::cout << "Test Configuration:\n";
    std::cout << "  Block size: " << BLOCK_SIZE << " samples\n";
    std::cout << "  Num blocks: " << NUM_BLOCKS << "\n";
    std::cout << "  Total samples: " << (BLOCK_SIZE * NUM_BLOCKS) << "\n";
    std::cout << "  Duration: " << (BLOCK_SIZE * NUM_BLOCKS / static_cast<float>(SAMPLE_RATE)) << " seconds\n\n";

    // Generate test signal (1kHz sine at -3dBFS to trigger limiting)
    std::vector<float> testSignal(BLOCK_SIZE);
    float amplitude = std::pow(10.0f, -3.0f / 20.0f);
    for (int i = 0; i < BLOCK_SIZE; ++i) {
        double phase = 2.0 * M_PI * 1000.0 * i / SAMPLE_RATE;
        testSignal[i] = amplitude * std::sin(phase);
    }

    // Test 1: Simple limiter (no oversampling)
    std::cout << "Test 1: Simple Limiter (No Oversampling)\n";

    SimpleLimiter simpleLimiter;
    std::vector<float> buffer1(BLOCK_SIZE);

    CPUProfiler profiler1;
    profiler1.begin();

    for (int block = 0; block < NUM_BLOCKS; ++block) {
        std::copy(testSignal.begin(), testSignal.end(), buffer1.begin());
        simpleLimiter.processBlock(buffer1.data(), BLOCK_SIZE);
    }

    profiler1.stop();

    double time1Ms = profiler1.getElapsedMilliseconds();
    double time1PerBlock = time1Ms / NUM_BLOCKS;
    double cpuPercentage1 = (time1PerBlock / (BLOCK_SIZE / static_cast<float>(SAMPLE_RATE) * 1000.0)) * 100.0;

    std::cout << std::fixed << std::setprecision(3);
    std::cout << "  Total time: " << time1Ms << " ms\n";
    std::cout << "  Per block: " << time1PerBlock << " ms\n";
    std::cout << "  CPU usage: " << cpuPercentage1 << "% of real-time\n\n";

    // Test 2: Oversampled limiter (4x)
    std::cout << "Test 2: Oversampled Limiter (4x True-Peak)\n";

    OversampledLimiter oversampledLimiter;
    std::vector<float> buffer2(BLOCK_SIZE);

    CPUProfiler profiler2;
    profiler2.begin();

    for (int block = 0; block < NUM_BLOCKS; ++block) {
        std::copy(testSignal.begin(), testSignal.end(), buffer2.begin());
        oversampledLimiter.processBlock(buffer2.data(), BLOCK_SIZE);
    }

    profiler2.stop();

    double time2Ms = profiler2.getElapsedMilliseconds();
    double time2PerBlock = time2Ms / NUM_BLOCKS;
    double cpuPercentage2 = (time2PerBlock / (BLOCK_SIZE / static_cast<float>(SAMPLE_RATE) * 1000.0)) * 100.0;

    std::cout << "  Total time: " << time2Ms << " ms\n";
    std::cout << "  Per block: " << time2PerBlock << " ms\n";
    std::cout << "  CPU usage: " << cpuPercentage2 << "% of real-time\n\n";

    // Overhead analysis
    std::cout << "Overhead Analysis:\n";
    double overhead = time2Ms - time1Ms;
    double overheadPercent = (overhead / time1Ms) * 100.0;
    double overheadFactor = time2Ms / time1Ms;

    std::cout << "  Absolute overhead: " << overhead << " ms\n";
    std::cout << "  Overhead %: " << overheadPercent << "%\n";
    std::cout << "  Speed factor: " << overheadFactor << "x slower\n\n";

    // CPU headroom analysis
    std::cout << "CPU Headroom (single core, assuming 100% real-time budget):\n";
    std::cout << "  Simple limiter headroom: " << (100.0 - cpuPercentage1) << "%\n";
    std::cout << "  True-peak limiter headroom: " << (100.0 - cpuPercentage2) << "%\n";
    std::cout << "  Safe margin for true-peak: " << (cpuPercentage2 < 25.0 ? "Yes ✓" : "Check ⚠") << "\n\n";

    std::cout << "═══════════════════════════════════════════════\n";
    std::cout << "Conclusion:\n";
    std::cout << "  4x oversampling adds " << overheadPercent << "% CPU overhead\n";
    std::cout << "  True-peak limiter uses " << cpuPercentage2 << "% of real-time budget\n";
    std::cout << "  Status: " << (cpuPercentage2 < 50.0 ? "✓ PASS - Acceptable CPU usage" : "✗ FAIL - High CPU usage") << "\n\n";

    return 0;
}
