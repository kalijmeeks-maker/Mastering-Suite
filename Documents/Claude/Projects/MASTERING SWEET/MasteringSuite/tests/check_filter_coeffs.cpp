#include "../Source/Metering/EbuR128Meter_TestInterface.h"
#include <iostream>
#include <iomanip>

int main() {
    EbuR128Meter_TestInterface meter;
    meter.prepare(48000.0, 2);

    std::cout << "Meter prepared at 48kHz, 2 channels\n";
    std::cout << "Check if filters were initialized by processing silence and measuring output:\n\n";

    // Process 1 second of silence
    std::vector<float> silence(48000, 0.0f);
    const float* ptrs[2] = {silence.data(), silence.data()};

    meter.processPlain(ptrs, 2, 48000);

    std::cout << "Integrated LUFS after 1s silence: " << meter.getIntegratedLufs() << "\n";

    // Process 1 second of 1kHz @ -20dBFS sine
    std::vector<float> sine(48000);
    float amplitude = 0.1f; // -20 dBFS
    for (int i = 0; i < 48000; ++i) {
        sine[i] = amplitude * sinf(2.0f * 3.14159f * 1000.0f * i / 48000.0f);
    }

    ptrs[0] = sine.data();
    ptrs[1] = sine.data();

    meter.processPlain(ptrs, 2, 48000);

    std::cout << "Integrated LUFS after sine: " << meter.getIntegratedLufs() << "\n";

    return 0;
}
