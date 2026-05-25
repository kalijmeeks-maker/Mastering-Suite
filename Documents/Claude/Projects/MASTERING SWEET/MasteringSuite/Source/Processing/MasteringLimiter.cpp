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
    
    // Default ballistics
    attackCoeff = std::exp (-1.0 / (0.1 * 0.001 * sr)); // 0.1ms attack
    releaseCoeff = std::exp (-1.0 / (releaseTimeMs * 0.001 * sr));

    isInitialized = false;

    if (useTruePeak && numChannels > 0) {
        // Create 4x oversampler (factor=2 means 2^2 = 4) with polyphase IIR filter for true-peak detection
        oversampler = std::make_unique<juce::dsp::Oversampling<float>>(numChannels, 2, juce::dsp::Oversampling<float>::filterHalfBandPolyphaseIIR);

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
        float reduction = thresholdDbfs - peakDb;
        if (reduction < currentGainReductionDb)
            currentGainReductionDb = attackCoeff * currentGainReductionDb + (1.0f - attackCoeff) * reduction;
        else
            currentGainReductionDb = releaseCoeff * currentGainReductionDb + (1.0f - releaseCoeff) * reduction;
    } else {
        currentGainReductionDb = releaseCoeff * currentGainReductionDb;
    }

    gainReductionDb.store (currentGainReductionDb);
    
    // Apply gain + honor ceiling
    float gain = std::pow (10.0f, (currentGainReductionDb + makeupGainDb) / 20.0f);
    float ceilGain = std::pow (10.0f, ceilingDbfs / 20.0f);
    
    for (int ch = 0; ch < numCh; ++ch) {
        auto* ptr = const_cast<float*>(oversampledBlock.getChannelPointer(ch));
        for (int i = 0; i < numSamples; ++i) {
            ptr[i] *= gain;
            if (std::abs(ptr[i]) > ceilGain) ptr[i] = (ptr[i] > 0) ? ceilGain : -ceilGain;
        }
    }

    oversampler->processSamplesDown (block);
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
        float reduction = thresholdDbfs - peakDb;
        if (reduction < currentGainReductionDb)
            currentGainReductionDb = attackCoeff * currentGainReductionDb + (1.0f - attackCoeff) * reduction;
        else
            currentGainReductionDb = releaseCoeff * currentGainReductionDb + (1.0f - releaseCoeff) * reduction;
    } else {
        currentGainReductionDb = releaseCoeff * currentGainReductionDb;
    }

    gainReductionDb.store (currentGainReductionDb);
    float gain = std::pow (10.0f, (currentGainReductionDb + makeupGainDb) / 20.0f);
    float ceilGain = std::pow (10.0f, ceilingDbfs / 20.0f);

    for (int ch = 0; ch < numCh; ++ch) {
        float* ptr = buffer.getWritePointer(ch);
        for (int i = 0; i < numSamples; ++i) {
            ptr[i] *= gain;
            if (std::abs(ptr[i]) > ceilGain) ptr[i] = (ptr[i] > 0) ? ceilGain : -ceilGain;
        }
    }
}

void MasteringLimiter::setThreshold (float dbfs) {
    thresholdDbfs = dbfs;
}

void MasteringLimiter::setRelease (float timeMs) {
    releaseTimeMs = timeMs;
    if (sr > 0) releaseCoeff = std::exp (-1.0 / (releaseTimeMs * 0.001 * sr));
}

void MasteringLimiter::setCeiling (float dbfs) {
    ceilingDbfs = dbfs;
}

void MasteringLimiter::setMakeupGain (float dbfs) {
    makeupGainDb = dbfs;
}
