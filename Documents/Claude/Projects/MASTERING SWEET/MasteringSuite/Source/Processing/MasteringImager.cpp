#include "MasteringImager.h"
#include <cmath>

MasteringImager::MasteringImager() = default;

void MasteringImager::prepare (double sampleRate, int inNumChannels) {
    sr = sampleRate;
    numChannels = inNumChannels;
}

void MasteringImager::processBlock (juce::AudioBuffer<float>& buffer) {
    if (numChannels < 2) return;

    int numSamples = buffer.getNumSamples();
    float* left = buffer.getWritePointer(0);
    float* right = buffer.getWritePointer(1);

    for (int i = 0; i < numSamples; ++i) {
        float l = left[i];
        float r = right[i];

        // Mid-Side processing for width
        float mid = (l + r) * 0.5f;
        float side = (l - r) * 0.5f;

        side *= width;

        l = mid + side;
        r = mid - side;

        // Panning (Constant Power)
        float angle = juce::jmap(pan, -1.0f, 1.0f, 0.0f, juce::MathConstants<float>::halfPi);
        float gainL = std::cos(angle);
        float gainR = std::sin(angle);

        left[i] = l * gainL * std::sqrt(2.0f);
        right[i] = r * gainR * std::sqrt(2.0f);
    }
}

void MasteringImager::setWidth (float w) {
    width = w;
}

void MasteringImager::setPan (float p) {
    pan = p;
}
