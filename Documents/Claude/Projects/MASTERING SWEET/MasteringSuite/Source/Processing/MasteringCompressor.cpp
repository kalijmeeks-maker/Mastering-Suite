#include "MasteringCompressor.h"
#include <cmath>
#include <algorithm>

MasteringCompressor::MasteringCompressor() = default;

void MasteringCompressor::prepare (double sampleRate, int inNumChannels) {
    sr = sampleRate;
    numChannels = inNumChannels;

    // Compute attack/release coefficients from time constants
    // coeff = exp(-1.0 / (time_seconds * sr))
    attackCoeff = std::exp (-1.0 / (attackMs * 0.001 * sr));
    releaseCoeff = std::exp (-1.0 / (releaseMs * 0.001 * sr));
    envDb = 0.0f;
}

void MasteringCompressor::reset() {
    envDb = 0.0f;
    gainReductionDb.store (0.0f);
}

void MasteringCompressor::processBlock (juce::AudioBuffer<float>& buffer) {
    int numSamples = buffer.getNumSamples();

    // Sample-by-sample processing
    for (int i = 0; i < numSamples; ++i) {
        // Find input peak across all channels
        float peakLevel = 0.0f;
        for (int ch = 0; ch < numChannels; ++ch) {
            peakLevel = std::max (peakLevel, std::abs (buffer.getReadPointer(ch)[i]));
        }

        // Convert to dB
        float inputDb = (peakLevel > 0.0f) ? 20.0f * std::log10 (peakLevel) : -200.0f;

        // Envelope follower (peak detection)
        float coeff = (inputDb > envDb) ? attackCoeff : releaseCoeff;
        envDb = coeff * envDb + (1.0f - coeff) * inputDb;

        // Gain computer
        float gainDb = computeGain (envDb);

        // Store for meter readback
        gainReductionDb.store (gainDb);

        // Apply gain to all channels
        float gainLinear = std::pow (10.0f, (gainDb + makeupDb) / 20.0f);
        float dryLinear = std::pow (10.0f, makeupDb / 20.0f);
        float wetLinear = gainLinear;

        // Mix dry/wet
        float finalGain = dryLinear * (1.0f - mix) + wetLinear * mix;

        for (int ch = 0; ch < numChannels; ++ch)
            buffer.getWritePointer(ch)[i] *= finalGain;
    }
}

float MasteringCompressor::computeGain (float inputDb) const {
    // Standard soft-knee gain computer
    // Default: downward compressor behavior

    float gainDb = 0.0f;

    if (mode == Mode::Comp) {
        // Compressor: reduce gain above threshold
        float kneeStart = thresholdDb - kneeDb / 2.0f;
        float kneeEnd = thresholdDb + kneeDb / 2.0f;

        if (inputDb < kneeStart) {
            gainDb = 0.0f;
        } else if (inputDb < kneeEnd) {
            // Soft knee: quadratic interpolation in knee region
            float t = (inputDb - kneeStart) / kneeDb;
            float slope = (ratio - 1.0f) / ratio;
            gainDb = -slope * kneeDb / 2.0f * t * t;
        } else {
            // Hard knee: linear compression above threshold
            gainDb = (thresholdDb - inputDb) / ratio + (inputDb - thresholdDb);
            gainDb = thresholdDb - (inputDb - thresholdDb) / ratio - inputDb;
            gainDb = (thresholdDb + (inputDb - thresholdDb) / ratio) - inputDb;
        }
    } else if (mode == Mode::Exp) {
        // Expander: increase attenuation below threshold
        float kneeStart = thresholdDb - kneeDb / 2.0f;
        float kneeEnd = thresholdDb + kneeDb / 2.0f;

        if (inputDb > kneeEnd) {
            gainDb = 0.0f;
        } else if (inputDb > kneeStart) {
            // Soft knee
            float t = (kneeEnd - inputDb) / kneeDb;
            float slope = (ratio - 1.0f) / ratio;
            gainDb = -slope * kneeDb / 2.0f * t * t;
        } else {
            // Hard knee: expand below threshold
            gainDb = (inputDb - thresholdDb) * ratio - (inputDb - thresholdDb);
        }
    } else if (mode == Mode::Up) {
        // Upward compression: boost signals below threshold toward it
        float kneeStart = thresholdDb - kneeDb / 2.0f;
        float kneeEnd = thresholdDb + kneeDb / 2.0f;

        if (inputDb > kneeEnd) {
            gainDb = 0.0f;
        } else if (inputDb > kneeStart) {
            // Soft knee
            float t = (kneeEnd - inputDb) / kneeDb;
            float slope = (ratio - 1.0f) / ratio;
            gainDb = slope * kneeDb / 2.0f * t * t;
        } else {
            // Hard knee: boost below threshold
            gainDb = (thresholdDb - inputDb) * (ratio - 1.0f) / ratio;
        }
    }

    return gainDb;
}

void MasteringCompressor::setMode (Mode m) {
    mode = m;
}

void MasteringCompressor::setThreshold (float dBFS) {
    thresholdDb = dBFS;
}

void MasteringCompressor::setRatio (float r) {
    ratio = r;
}

void MasteringCompressor::setKnee (float dB) {
    kneeDb = dB;
}

void MasteringCompressor::setAttack (float ms) {
    attackMs = ms;
    if (sr > 0.0)
        attackCoeff = std::exp (-1.0 / (attackMs * 0.001 * sr));
}

void MasteringCompressor::setRelease (float ms) {
    releaseMs = ms;
    if (sr > 0.0)
        releaseCoeff = std::exp (-1.0 / (releaseMs * 0.001 * sr));
}

void MasteringCompressor::setMakeup (float dB) {
    makeupDb = dB;
}

void MasteringCompressor::setMix (float zeroToOne) {
    mix = std::clamp (zeroToOne, 0.0f, 1.0f);
}
