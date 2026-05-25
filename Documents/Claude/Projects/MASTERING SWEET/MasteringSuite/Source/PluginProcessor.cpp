#include "PluginProcessor.h"
#include "UI/PluginEditor.h"
#include "Processing/MasteringImager.cpp"

juce::AudioProcessorValueTreeState::ParameterLayout MasteringSuiteProcessor::createParameterLayout() {
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    // EQ parameters (6 bands: HPF, LowShelf, Peak1, Peak2, HighShelf, LPF)
    const char* bandNames[] = {"eq0", "eq1", "eq2", "eq3", "eq4", "eq5"};
    for (int i = 0; i < 6; ++i) {
        params.push_back(std::make_unique<juce::AudioParameterChoice>(juce::ParameterID(juce::String(bandNames[i]) + "Type", 1), "Band " + juce::String(i+1) + " Type", juce::StringArray("Bypass", "Peak", "HighPass", "LowPass", "LowShelf", "HighShelf"), (i == 0) ? 2 : (i == 5) ? 3 : 1));
        params.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID(juce::String(bandNames[i]) + "Gain", 1), "Band " + juce::String(i+1) + " Gain", -12.0f, 12.0f, 0.0f));
        params.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID(juce::String(bandNames[i]) + "Freq", 1), "Band " + juce::String(i+1) + " Freq", 20.0f, 20000.0f, (i == 0) ? 30.0f : (i == 1) ? 100.0f : (i == 2) ? 500.0f : (i == 3) ? 2500.0f : (i == 4) ? 8000.0f : 18000.0f));
        params.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID(juce::String(bandNames[i]) + "Q", 1), "Band " + juce::String(i+1) + " Q", 0.1f, 10.0f, 0.707f));
    }

    // Dynamics parameters
    params.push_back(std::make_unique<juce::AudioParameterBool>(juce::ParameterID("dynEnabled", 1), "Dynamics Enabled", true));
    params.push_back(std::make_unique<juce::AudioParameterChoice>(juce::ParameterID("dynMode", 1), "Dynamics Mode", juce::StringArray("Compressor", "Expander", "Upward"), 0));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("dynThreshold", 1), "Dynamics Threshold", -60.0f, 0.0f, -18.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("dynRatio", 1), "Dynamics Ratio", 1.0f, 20.0f, 2.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("dynKnee", 1), "Dynamics Knee", 0.0f, 24.0f, 6.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("dynAttack", 1), "Dynamics Attack", 0.1f, 100.0f, 14.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("dynRelease", 1), "Dynamics Release", 10.0f, 1000.0f, 120.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("dynMakeup", 1), "Dynamics Makeup", 0.0f, 24.0f, 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("dynMix", 1), "Dynamics Mix", 0.0f, 100.0f, 100.0f));

    // Limiter parameters
    params.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("limThreshold", 1), "Limiter Threshold", -40.0f, 0.0f, -6.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("limRelease", 1), "Limiter Release", 10.0f, 500.0f, 120.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("limCeiling", 1), "Limiter Ceiling", -6.0f, 0.0f, -0.1f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("limMakeup", 1), "Limiter Makeup", 0.0f, 6.0f, 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterChoice>(juce::ParameterID("limStyle", 1), "Limiter Style", juce::StringArray("Transparent", "Punchy", "Warm"), 0));
    params.push_back(std::make_unique<juce::AudioParameterBool>(juce::ParameterID("limTruePeak", 1), "True Peak", true));

    // Imager parameters
    params.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("imgWidth", 1), "Stereo Width", 0.0f, 2.0f, 1.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("imgPan", 1), "Panning", -1.0f, 1.0f, 0.0f));

    // Global parameters
    params.push_back(std::make_unique<juce::AudioParameterBool>(juce::ParameterID("bypass", 1), "Bypass", false));
    params.push_back(std::make_unique<juce::AudioParameterChoice>(juce::ParameterID("oversample", 1), "Oversample", juce::StringArray("1x", "2x", "4x"), 2));
    
    // Density Toggle (Non-automatable)
    auto densityParam = std::make_unique<juce::AudioParameterChoice>(juce::ParameterID("density", 1), "UI Density", juce::StringArray("Compact", "Default", "Expanded"), 1);
    params.push_back(std::move(densityParam));

    return { params.begin(), params.end() };
}

MasteringSuiteProcessor::MasteringSuiteProcessor()
    : apvts(*this, &undoManager, "PARAMETERS", createParameterLayout()) {}

void MasteringSuiteProcessor::prepareToPlay (double sr, int bs) {
    eq.prepare (sr, getTotalNumInputChannels());
    compressor.prepare(sr, getTotalNumInputChannels());
    imager.prepare(sr, getTotalNumInputChannels());
    limiter.prepare(sr, getTotalNumInputChannels(), bs);
    meter.prepare (sr, getTotalNumInputChannels());

    // Sync parameters
    const char* bandNames[] = {"eq0", "eq1", "eq2", "eq3", "eq4", "eq5"};
    for (int i = 0; i < 6; ++i) {
        if (auto* p = apvts.getRawParameterValue(juce::String(bandNames[i]) + "Type")) eq.setBandType(i, (MasteringEQ::FilterType)(int)*p);
        if (auto* p = apvts.getRawParameterValue(juce::String(bandNames[i]) + "Gain")) eq.setBandGain(i, *p);
        if (auto* p = apvts.getRawParameterValue(juce::String(bandNames[i]) + "Freq")) eq.setBandFrequency(i, *p);
        if (auto* p = apvts.getRawParameterValue(juce::String(bandNames[i]) + "Q"))    eq.setBandQ(i, *p);
    }

    if (auto* p = apvts.getRawParameterValue("dynMode"))      compressor.setMode((MasteringCompressor::Mode)(int)*p);
    if (auto* p = apvts.getRawParameterValue("dynThreshold")) compressor.setThreshold(*p);
    if (auto* p = apvts.getRawParameterValue("dynRatio"))     compressor.setRatio(*p);
    if (auto* p = apvts.getRawParameterValue("dynKnee"))      compressor.setKnee(*p);
    if (auto* p = apvts.getRawParameterValue("dynAttack"))    compressor.setAttack(*p);
    if (auto* p = apvts.getRawParameterValue("dynRelease"))   compressor.setRelease(*p);
    if (auto* p = apvts.getRawParameterValue("dynMakeup"))    compressor.setMakeup(*p);
    if (auto* p = apvts.getRawParameterValue("dynMix"))       compressor.setMix(*p / 100.0f);

    if (auto* p = apvts.getRawParameterValue("limThreshold")) limiter.setThreshold(*p);
    if (auto* p = apvts.getRawParameterValue("limRelease"))   limiter.setRelease(*p);
    if (auto* p = apvts.getRawParameterValue("limMakeup"))    limiter.setMakeupGain(*p);

    if (auto* p = apvts.getRawParameterValue("imgWidth")) imager.setWidth(*p);
    if (auto* p = apvts.getRawParameterValue("imgPan"))   imager.setPan(*p);
}

void MasteringSuiteProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&) {
    juce::ScopedNoDenormals nd;

    if (auto* bypassP = apvts.getRawParameterValue("bypass")) {
        if (*bypassP > 0.5f) return;
    }

    // Update parameters from APVTS
    const char* bandNames[] = {"eq0", "eq1", "eq2", "eq3", "eq4", "eq5"};
    for (int i = 0; i < 6; ++i) {
        if (auto* p = apvts.getRawParameterValue(juce::String(bandNames[i]) + "Type")) eq.setBandType(i, (MasteringEQ::FilterType)(int)*p);
        if (auto* p = apvts.getRawParameterValue(juce::String(bandNames[i]) + "Gain")) eq.setBandGain(i, *p);
        if (auto* p = apvts.getRawParameterValue(juce::String(bandNames[i]) + "Freq")) eq.setBandFrequency(i, *p);
        if (auto* p = apvts.getRawParameterValue(juce::String(bandNames[i]) + "Q"))    eq.setBandQ(i, *p);
    }

    if (auto* p = apvts.getRawParameterValue("dynMode"))      compressor.setMode((MasteringCompressor::Mode)(int)*p);
    if (auto* p = apvts.getRawParameterValue("dynThreshold")) compressor.setThreshold(*p);
    if (auto* p = apvts.getRawParameterValue("dynRatio"))     compressor.setRatio(*p);
    if (auto* p = apvts.getRawParameterValue("dynKnee"))      compressor.setKnee(*p);
    if (auto* p = apvts.getRawParameterValue("dynAttack"))    compressor.setAttack(*p);
    if (auto* p = apvts.getRawParameterValue("dynRelease"))   compressor.setRelease(*p);
    if (auto* p = apvts.getRawParameterValue("dynMakeup"))    compressor.setMakeup(*p);
    if (auto* p = apvts.getRawParameterValue("dynMix"))       compressor.setMix(*p / 100.0f);

    if (auto* p = apvts.getRawParameterValue("limThreshold")) limiter.setThreshold(*p);
    if (auto* p = apvts.getRawParameterValue("limRelease"))   limiter.setRelease(*p);
    if (auto* p = apvts.getRawParameterValue("limMakeup"))    limiter.setMakeupGain(*p);

    if (auto* p = apvts.getRawParameterValue("imgWidth")) imager.setWidth(*p);
    if (auto* p = apvts.getRawParameterValue("imgPan"))   imager.setPan(*p);

    // Track input peak
    float inPeakAbs = 0.0f;
    for (int ch = 0; ch < buffer.getNumChannels(); ++ch) {
        inPeakAbs = std::max(inPeakAbs, buffer.getMagnitude(ch, 0, buffer.getNumSamples()));
    }
    float inDb = juce::Decibels::gainToDecibels(inPeakAbs, -100.0f);
    if (inDb > inputPeak.load()) inputPeak.store(inDb);
    else inputPeak.store(inputPeak.load() * 0.999f + inDb * 0.001f); // Slow decay

    // Input gain
    if (auto* inGainP = apvts.getRawParameterValue("inGain")) {
        float inGainDb = *inGainP;
        buffer.applyGain(juce::Decibels::decibelsToGain(inGainDb));
    }

    // DSP Chain
    eq.processBlock(buffer);

    if (auto* dynEnabledP = apvts.getRawParameterValue("dynEnabled")) {
        if (*dynEnabledP > 0.5f)
            compressor.processBlock(buffer);
    }

    imager.processBlock(buffer);

    limiter.processBlock(buffer);

    // Track output peak — sum + per-channel for dual L+R meter.
    float outPeakAbs = 0.0f;
    float lAbs = 0.0f, rAbs = 0.0f;
    int nCh = buffer.getNumChannels();
    if (nCh > 0) lAbs = buffer.getMagnitude(0, 0, buffer.getNumSamples());
    rAbs = (nCh > 1) ? buffer.getMagnitude(1, 0, buffer.getNumSamples()) : lAbs;
    outPeakAbs = std::max(lAbs, rAbs);

    auto decay = [](std::atomic<float>& a, float dB) {
        float prev = a.load();
        a.store(dB > prev ? dB : prev * 0.999f + dB * 0.001f);
    };
    decay(outputPeak,    juce::Decibels::gainToDecibels(outPeakAbs, -100.0f));
    decay(channelLevelL, juce::Decibels::gainToDecibels(lAbs,       -100.0f));
    decay(channelLevelR, juce::Decibels::gainToDecibels(rAbs,       -100.0f));
    float outDb = outputPeak.load();

    // Output gain
    if (auto* outGainP = apvts.getRawParameterValue("outGain")) {
        float outGainDb = *outGainP;
        buffer.applyGain(juce::Decibels::decibelsToGain(outGainDb));
    }

    // Feed spectrum analyzer with output signal (sum of L/R)
    // Also compute L/R correlation and refresh true-peak activity flag.
    if (buffer.getNumChannels() > 0) {
        auto* left = buffer.getReadPointer(0);
        auto* right = (buffer.getNumChannels() > 1) ? buffer.getReadPointer(1) : left;
        double sumLR = 0.0, sumL2 = 0.0, sumR2 = 0.0;
        for (int i = 0; i < buffer.getNumSamples(); ++i) {
            pushSampleIntoFifo((left[i] + right[i]) * 0.5f);
            pushGonioSamples(left[i], right[i]);
            sumLR += left[i] * right[i];
            sumL2 += left[i] * left[i];
            sumR2 += right[i] * right[i];
        }
        double denom = std::sqrt(sumL2 * sumR2);
        float instant = (denom > 1e-9) ? (float)(sumLR / denom) : 1.0f;
        // Slow smoothing so the tab readout doesn't flicker
        float prev = correlation.load();
        correlation.store(prev * 0.9f + instant * 0.1f);
    }

    truePeakActive.store(limiter.getCurrentGainReduction() < -0.05f);

    meter.process(buffer);
}

void MasteringSuiteProcessor::pushGonioSamples(float l, float r) {
    if (gonioIndex == goniometerSize) {
        const juce::ScopedLock sl(gonioLock);
        gonioIndex = 0;
    }
    gonioBufferL[gonioIndex] = l;
    gonioBufferR[gonioIndex] = r;
    gonioIndex++;
}

void MasteringSuiteProcessor::pushSampleIntoFifo(float sample) {
    if (fifoIndex == fftSize) {
        std::fill(fftData.begin(), fftData.end(), 0.0f);
        std::copy(fifo.begin(), fifo.end(), fftData.begin());
        
        window.multiplyWithWindowingTable(fftData.data(), fftSize);
        fft.performFrequencyOnlyForwardTransform(fftData.data());

        const juce::ScopedLock sl(fftLock);
        
        // Ballistics constants (approx for 30Hz block processing)
        // 30ms attack -> coeff ~0.15
        // 200ms release -> coeff ~0.95
        const float attackCoeff = 0.15f;
        const float releaseCoeff = 0.95f;

        for (int i = 0; i < fftSize / 2; ++i) {
            float newVal = fftData[i];
            float oldVal = scopeData[i];

            if (newVal > oldVal)
                scopeData[i] = oldVal + attackCoeff * (newVal - oldVal);
            else
                scopeData[i] = oldVal + releaseCoeff * (newVal - oldVal);
        }

        fifoIndex = 0;
    }

    fifo[fifoIndex++] = sample;
}

void MasteringSuiteProcessor::getStateInformation(juce::MemoryBlock& destData) {
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void MasteringSuiteProcessor::setStateInformation(const void* data, int sizeInBytes) {
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    if (xmlState != nullptr && xmlState->hasTagName(apvts.state.getType())) {
        apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
    }
}

juce::AudioProcessorEditor* MasteringSuiteProcessor::createEditor() {
    return new PluginEditor (*this);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() {
    return new MasteringSuiteProcessor();
}
