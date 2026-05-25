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

    // Goniometer scaling mode (Design's spec: AUTO RMS vs FIXED -20 dBFS)
    params.push_back(std::make_unique<juce::AudioParameterChoice>(juce::ParameterID("gonioScale", 1),
        "Goniometer Scale", juce::StringArray("Auto", "Fixed -20dB"), 0));

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
    int idx = gonioIndex.load(std::memory_order_relaxed);
    gonioBufferL[idx] = l;
    gonioBufferR[idx] = r;
    gonioIndex.store((idx + 1) % goniometerSize, std::memory_order_relaxed);
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

// ────────────────────────── A/B banks ──────────────────────────
void MasteringSuiteProcessor::captureCurrentToBank(int bank) {
    if (bank < 0 || bank > 1) return;
    banks[bank].clear();
    for (auto* param : getParameters()) {
        if (auto* p = dynamic_cast<juce::RangedAudioParameter*>(param)) {
            if (auto* v = apvts.getRawParameterValue(p->paramID))
                banks[bank][p->paramID] = *v;
        }
    }
}

void MasteringSuiteProcessor::applyBankToParams(int bank) {
    if (bank < 0 || bank > 1 || banks[bank].empty()) return;
    for (auto& kv : banks[bank]) {
        if (auto* p = apvts.getParameter(kv.first))
            p->setValueNotifyingHost(p->getNormalisableRange().convertTo0to1(kv.second));
    }
}

void MasteringSuiteProcessor::switchToBank(int bank) {
    if (bank == activeBank || bank < 0 || bank > 1) return;
    captureCurrentToBank(activeBank);  // freeze whatever the user just did
    activeBank = bank;
    applyBankToParams(bank);
    postStatusMessage(juce::String("Switched to bank ") + (bank == 0 ? "A" : "B"));
}

void MasteringSuiteProcessor::copyActiveBankToOther() {
    captureCurrentToBank(activeBank);
    int other = 1 - activeBank;
    banks[other] = banks[activeBank];
    postStatusMessage(juce::String(activeBank == 0 ? "A" : "B") + " → "
                      + (other == 0 ? "A" : "B"));
}

// ────────────────────────── Factory presets ──────────────────────────
// Five hand-crafted starting points. Each preset writes a handful of
// parameters via setValueNotifyingHost, then loadPreset stores the index
// so the HeaderBar can show the active name.

juce::StringArray MasteringSuiteProcessor::getPresetNames() {
    return { "DEFAULT", "BRIGHT", "WARM", "PUNCHY", "WIDE" };
}

namespace {
    void setF(juce::AudioProcessorValueTreeState& apvts, const juce::String& id, float v) {
        if (auto* p = apvts.getParameter(id))
            p->setValueNotifyingHost(p->getNormalisableRange().convertTo0to1(v));
    }
    void setI(juce::AudioProcessorValueTreeState& apvts, const juce::String& id, int v) {
        setF(apvts, id, (float)v);
    }
}

// Returns true if any current parameter value differs from what we snapshot'd
// in the last loadPreset() call. Used by the header to render the "•" bullet.
bool MasteringSuiteProcessor::isPresetModified() const {
    if (presetSnapshot.empty()) return false;
    for (auto& kv : presetSnapshot) {
        if (auto* p = apvts.getRawParameterValue(kv.first)) {
            if (std::abs(*p - kv.second) > 1e-4f) return true;
        }
    }
    return false;
}

void MasteringSuiteProcessor::postStatusMessage(const juce::String& msg) {
    const juce::ScopedLock sl(statusLock);
    currentStatus = msg;
    statusPostedAtMs = juce::Time::getMillisecondCounterHiRes();
}

juce::String MasteringSuiteProcessor::getStatusMessage() {
    const juce::ScopedLock sl(statusLock);
    // 2s toast lifetime per Design's spec.
    if (juce::Time::getMillisecondCounterHiRes() - statusPostedAtMs > 2000.0)
        return {};
    return currentStatus;
}

void MasteringSuiteProcessor::loadPreset(int index) {
    currentPreset = juce::jlimit(0, (int)getPresetNames().size() - 1, index);

    // Reset EQ bands to default first (HP/LowShelf/Peak/Peak/HighShelf/LP).
    const int defaultTypes[6] = { 2, 4, 1, 1, 5, 3 };
    const float defaultFreqs[6] = { 30, 100, 500, 2500, 8000, 18000 };
    for (int i = 0; i < 6; ++i) {
        juce::String pre = "eq" + juce::String(i);
        setI(apvts, pre + "Type", defaultTypes[i]);
        setF(apvts, pre + "Freq", defaultFreqs[i]);
        setF(apvts, pre + "Gain", 0.0f);
        setF(apvts, pre + "Q",    0.707f);
    }
    // Dynamics defaults
    setI(apvts, "dynMode",      0);
    setF(apvts, "dynThreshold", -18.0f);
    setF(apvts, "dynRatio",     2.0f);
    setF(apvts, "dynKnee",      6.0f);
    setF(apvts, "dynAttack",    14.0f);
    setF(apvts, "dynRelease",   120.0f);
    setF(apvts, "dynMakeup",    0.0f);
    setF(apvts, "dynMix",       100.0f);
    // Imager / Limiter defaults
    setF(apvts, "imgWidth",     1.0f);
    setF(apvts, "imgPan",       0.0f);
    setF(apvts, "limThreshold", -6.0f);
    setF(apvts, "limCeiling",   -0.3f);
    setF(apvts, "limRelease",   120.0f);
    setF(apvts, "limMakeup",    0.0f);

    switch (currentPreset) {
        case 1: // BRIGHT — gentle high-shelf lift, a touch of presence boost, modest limiter.
            setF(apvts, "eq3Gain",    1.5f);  setF(apvts, "eq3Freq", 3500.0f); setF(apvts, "eq3Q", 1.0f);
            setF(apvts, "eq4Gain",    2.5f);  setF(apvts, "eq4Freq", 9000.0f); setF(apvts, "eq4Q", 0.7f);
            setF(apvts, "limThreshold", -8.0f);
            break;
        case 2: // WARM — low-shelf push, slight 1-2kHz dip for body, gentler limiting.
            setF(apvts, "eq1Gain",    2.0f);  setF(apvts, "eq1Freq", 120.0f);  setF(apvts, "eq1Q", 0.8f);
            setF(apvts, "eq3Gain",   -1.5f);  setF(apvts, "eq3Freq", 2000.0f); setF(apvts, "eq3Q", 1.2f);
            setF(apvts, "eq4Gain",   -1.0f);  setF(apvts, "eq4Freq", 12000.0f);
            setF(apvts, "limRelease", 180.0f);
            break;
        case 3: // PUNCHY — slow-attack compression for transient bite, slight low-mid scoop, hot limiter.
            setF(apvts, "dynThreshold", -14.0f);
            setF(apvts, "dynRatio",     3.0f);
            setF(apvts, "dynAttack",    28.0f);
            setF(apvts, "dynRelease",   80.0f);
            setF(apvts, "dynMakeup",    2.5f);
            setF(apvts, "eq2Gain",     -1.0f); setF(apvts, "eq2Freq", 350.0f); setF(apvts, "eq2Q", 1.2f);
            setF(apvts, "limThreshold", -10.0f);
            setF(apvts, "limMakeup",    1.5f);
            break;
        case 4: // WIDE — stereo width bump, gentle high lift, otherwise neutral.
            setF(apvts, "imgWidth",   1.45f);
            setF(apvts, "eq4Gain",    1.0f);  setF(apvts, "eq4Freq", 10000.0f);
            break;
        default: // DEFAULT — leave at neutral.
            break;
    }

    // Snapshot final state so isPresetModified() can detect any future change.
    presetSnapshot.clear();
    auto allParams = apvts.state.getChildWithName("PARAM");  // not used — iterate via params layout
    for (auto* param : getParameters()) {
        if (auto* p = dynamic_cast<juce::RangedAudioParameter*>(param)) {
            if (auto* v = apvts.getRawParameterValue(p->paramID))
                presetSnapshot[p->paramID] = *v;
        }
    }
    currentUserPresetName.clear();  // loading a factory preset clears the user-preset binding
    postStatusMessage("Preset loaded: " + getPresetNames()[currentPreset]);
}

// ────────────────────────── User presets (v1.1-1) ──────────────────────────

juce::File MasteringSuiteProcessor::getUserPresetDir() {
    // ~/Library/Application Support/Mastering Sweet/Presets/
    // The /Library/Audio/Presets tree is root-owned on stock macOS installs,
    // so the canonical user-writable path is Application Support.
    auto d = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
                .getChildFile("Application Support/Mastering Sweet/Presets");
    if (!d.exists()) d.createDirectory();
    return d;
}

void MasteringSuiteProcessor::captureSnapshotFromCurrentParams() {
    presetSnapshot.clear();
    for (auto* param : getParameters()) {
        if (auto* p = dynamic_cast<juce::RangedAudioParameter*>(param)) {
            if (auto* v = apvts.getRawParameterValue(p->paramID))
                presetSnapshot[p->paramID] = *v;
        }
    }
}

static juce::String sanitizePresetName(juce::String n) {
    n = n.replaceCharacter('/', '_').replaceCharacter('\\', '_').replaceCharacter(':', '_');
    n = n.trim();
    if (n.length() > 64) n = n.substring(0, 64);
    return n;
}

juce::StringArray MasteringSuiteProcessor::listUserPresets() {
    juce::StringArray names;
    auto dir = getUserPresetDir();
    if (!dir.isDirectory()) return names;
    auto rootTag = apvts.state.getType().toString();
    for (auto& f : dir.findChildFiles(juce::File::findFiles, false, "*.xml")) {
        auto xml = juce::XmlDocument::parse(f);
        if (xml != nullptr && xml->hasTagName(rootTag)) {
            names.add(f.getFileNameWithoutExtension());
        } else {
            DBG("Skipping corrupt or non-matching preset: " << f.getFullPathName());
        }
    }
    names.sort(true);
    return names;
}

bool MasteringSuiteProcessor::savePresetAs(const juce::String& rawName) {
    auto name = sanitizePresetName(rawName);
    if (name.isEmpty()) return false;
    auto state = apvts.copyState();
    auto xml = state.createXml();
    if (xml == nullptr) return false;
    auto file = getUserPresetDir().getChildFile(name + ".xml");
    if (!xml->writeTo(file)) {
        DBG("Failed to write preset: " << file.getFullPathName());
        return false;
    }
    currentPreset = -1;
    currentUserPresetName = name;
    captureSnapshotFromCurrentParams();
    postStatusMessage("Preset saved: " + name);
    return true;
}

bool MasteringSuiteProcessor::saveChangesToCurrentPreset() {
    if (currentUserPresetName.isEmpty()) return false;  // factory presets are read-only
    return savePresetAs(currentUserPresetName);
}

bool MasteringSuiteProcessor::loadUserPreset(const juce::String& name) {
    auto file = getUserPresetDir().getChildFile(name + ".xml");
    if (!file.existsAsFile()) {
        DBG("User preset not found: " << file.getFullPathName());
        return false;
    }
    auto xml = juce::XmlDocument::parse(file);
    if (xml == nullptr || !xml->hasTagName(apvts.state.getType().toString())) {
        DBG("Corrupt user preset, refusing to load: " << file.getFullPathName());
        return false;
    }
    apvts.replaceState(juce::ValueTree::fromXml(*xml));
    currentPreset = -1;
    currentUserPresetName = name;
    captureSnapshotFromCurrentParams();
    postStatusMessage("Preset loaded: " + name);
    return true;
}

void MasteringSuiteProcessor::resetToCurrentSnapshot() {
    if (presetSnapshot.empty()) return;
    for (auto& kv : presetSnapshot) {
        if (auto* p = apvts.getParameter(kv.first))
            p->setValueNotifyingHost(p->getNormalisableRange().convertTo0to1(kv.second));
    }
    postStatusMessage("Reset to preset");
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() {
    return new MasteringSuiteProcessor();
}
