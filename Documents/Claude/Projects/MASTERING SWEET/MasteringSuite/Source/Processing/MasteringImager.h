#pragma once
#include <JuceHeader.h>
#include <atomic>

class MasteringImager {
public:
    MasteringImager();
    ~MasteringImager() = default;

    void prepare (double sampleRate, int numChannels);
    void processBlock (juce::AudioBuffer<float>& buffer);
    
    void setWidth (float width0to2);
    void setPan (float panNeg1to1);

private:
    float width = 1.0f;
    float pan = 0.0f;
    
    double sr = 48000.0;
    int numChannels = 2;
};
