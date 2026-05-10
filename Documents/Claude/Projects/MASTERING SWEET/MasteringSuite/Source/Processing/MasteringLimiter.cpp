#include "MasteringLimiter.h"
#include <cmath>
#include <algorithm>

MasteringLimiter::MasteringLimiter() {
    useTruePeak = true;  // 4x oversampling for true-peak detection (essential for mastering)
}
MasteringLimiter::~MasteringLimiter() = default;

void MasteringLimiter::prepare (double sampleRate, int numChannels, int samplesPerBlock) {
    sr = sampleRate;
    this->numChannels = numChannels;
    releaseCoeff = std::exp (-2.0 * 3.14159265358979323846 * releaseTimeMs / 1000.0 / sr);

    isInitialized = false;

    if (useTruePeak) {
        // Create 4x oversampler with polyphase IIR filter for true-peak detection
        oversampler = std::make_unique<juce::dsp::Oversampling<float>>(numChannels, 4, juce::dsp::Oversampling<float>::filterHalfBandPolyphaseIIR);

        // Initialize with actual block size plus safety margin
        int initBlockSize = juce::jmax(512, samplesPerBlock * 4);
        oversampler->initProcessing(initBlockSize);

        truePeakLatency = oversampler->getLatencyInSamples();
        isInitialized = true;
    }
}

void MasteringLimiter::reset() {
    currentGainReductionDb = 0.0f;
    gainReductionDb.store (0.0f);
    isInitialized = false;
    if (oversampler)
        oversampler->reset();
}

void MasteringLimiter::processBlock (juce::AudioBuffer<float>& buffer) {
    if (useTruePeak && oversampler && isInitialized)
        processSampleWithTruePeak (buffer);
    else
        processSampleSimple (buffer);
}

void MasteringLimiter::processSampleWithTruePeak (juce::AudioBuffer<float>& buffer) {
    auto block = juce::dsp::AudioBlock<float>(buffer);
    auto oversampledBlock = oversampler->processSamplesUp (block);

    int numSamples = (int)oversampledBlock.getNumSamples();
    int numCh = (int)oversampledBlock.getNumChannels();

    float peakDb = -std::numeric_limits<float>::infinity();
    for (int ch = 0; ch < numCh; ++ch) {
        const auto* samples = oversampledBlock.getChannelPointer(ch);
        for (int i = 0; i < numSamples; ++i) {
            float absVal = std::abs (samples[i]);
            if (absVal > 0.0f) {
                float db = 20.0f * std::log10 (absVal);
                peakDb = std::max (peakDb, db);
            }
        }
    }

    if (peakDb > thresholdDbfs) {
        currentGainReductionDb = -(peakDb - thresholdDbfs);
    } else {
        currentGainReductionDb = 0.0f;
    }

    gainReductionDb.store (currentGainReductionDb);
    float gain = std::pow (10.0f, (currentGainReductionDb + makeupGainDb) / 20.0f);

    for (int ch = 0; ch < numCh; ++ch) {
        auto* ptr = const_cast<float*>(oversampledBlock.getChannelPointer(ch));
        for (int i = 0; i < numSamples; ++i)
            ptr[i] *= gain;
    }

    oversampler->processSamplesDown (block);
    currentGainReductionDb = releaseCoeff * currentGainReductionDb;
}

void MasteringLimiter::processSampleSimple (juce::AudioBuffer<float>& buffer) {
    int numSamples = buffer.getNumSamples();
    int numCh = buffer.getNumChannels();

    float peakDb = -std::numeric_limits<float>::infinity();
    for (int ch = 0; ch < numCh; ++ch) {
        const float* samples = buffer.getReadPointer (ch);
        for (int i = 0; i < numSamples; ++i) {
            float absVal = std::abs (samples[i]);
            if (absVal > 0.0f) {
                float db = 20.0f * std::log10 (absVal);
                peakDb = std::max (peakDb, db);
            }
        }
    }

    if (peakDb > thresholdDbfs) {
        currentGainReductionDb = -(peakDb - thresholdDbfs);
    } else {
        currentGainReductionDb = 0.0f;
    }

    gainReductionDb.store (currentGainReductionDb);
    float gain = std::pow (10.0f, (currentGainReductionDb + makeupGainDb) / 20.0f);

    for (int ch = 0; ch < numCh; ++ch) {
        buffer.applyGain (ch, 0, numSamples, gain);
    }

    currentGainReductionDb = releaseCoeff * currentGainReductionDb;
}

void MasteringLimiter::setThreshold (float dbfs) {
    thresholdDbfs = dbfs;
}

void MasteringLimiter::setRelease (float timeMs) {
    releaseTimeMs = timeMs;
    releaseCoeff = std::exp (-2.0 * 3.14159265358979323846 * releaseTimeMs / 1000.0 / sr);
}

void MasteringLimiter::setMakeupGain (float dbfs) {
    makeupGainDb = dbfs;
}
