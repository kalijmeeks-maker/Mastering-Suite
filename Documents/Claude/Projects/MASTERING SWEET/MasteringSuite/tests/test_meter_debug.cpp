#include "../Source/Metering/EbuR128Meter_TestInterface.h"
#include <iostream>
#include <cmath>
#include <iomanip>

int main() {
    EbuR128Meter_TestInterface meter;
    meter.prepare(48000.0, 2);

    std::cout << std::fixed << std::setprecision(6);

    // Test 1: Simple unweighted signal
    std::cout << "\n=== Test 1: Stereo 1kHz @ 0.1 amplitude (unweighted) ===\n";

    std::vector<float> sine(48000);
    float amplitude = 0.1f;  // This is 10^(-20/20) = 0.1
    const float PI = 3.14159265359f;

    for (int i = 0; i < 48000; ++i) {
        sine[i] = amplitude * std::sinf(2.0f * PI * 1000.0f * i / 48000.0f);
    }

    const float* ptrs[2] = {sine.data(), sine.data()};

    // Process 10 seconds
    for (int block = 0; block < 10; ++block) {
        meter.processPlain(ptrs, 2, 4800);  // 100ms blocks
    }

    float measured = meter.getIntegratedLufs();
    std::cout << "Amplitude: " << amplitude << "\n";
    std::cout << "Mean square should be: " << (amplitude * amplitude) * 2.0f << "\n";
    std::cout << "Expected LUFS (unweighted): " << (-0.691f + 10.0f * std::log10f(amplitude * amplitude)) << "\n";
    std::cout << "Actual LUFS: " << measured << "\n";

    // Test 2: Check if K-weighting is working
    std::cout << "\n=== Test 2: Check filter state ===\n";

    // Create a new meter
    EbuR128Meter_TestInterface meter2;
    meter2.prepare(48000.0, 2);

    // Feed DC component (which should be heavily attenuated by high-pass)
    std::vector<float> dc(48000, 0.1f);
    ptrs[0] = dc.data();
    ptrs[1] = dc.data();

    // Process 1 second of DC
    for (int block = 0; block < 10; ++block) {
        meter2.processPlain(ptrs, 2, 4800);
    }

    float dc_lufs = meter2.getIntegratedLufs();
    std::cout << "DC component (0.1) after high-pass filter: " << dc_lufs << " LUFS\n";
    std::cout << "(Should be very negative if high-pass is working)\n";

    return 0;
}
