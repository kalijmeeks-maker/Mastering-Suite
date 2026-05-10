#include "PluginProcessor.h"
#include "UI/PluginEditor.h"

juce::AudioProcessorValueTreeState::ParameterLayout MasteringSuiteProcessor::createParameterLayout() {
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    // EQ Parameters (6 bands: HP, LS, Bell1, Bell2, HS, LP)
    const char* bandNames[] = {"eq0", "eq1", "eq2", "eq3", "eq4", "eq5"};
    const char* bandLabels[] = {"HP", "LS", "BELL1", "BELL2", "HS", "LP"};
    const int bandDefTypes[] = {1, 2, 3, 3, 1, 1}; // HighShelf, LowShelf, Peaking, Peaking, HighShelf, HighShelf
    const float bandDefFreqs[] = {32.0f, 120.0f, 480.0f, 2400.0f, 8200.0f, 18000.0f};
    const float bandDefQs[] = {0.70f, 0.90f, 1.40f, 1.20f, 0.80f, 0.70f};

    for (int i = 0; i < 6; ++i) {
        // Band Type
        juce::String typeId = juce::String(bandNames[i]) + "Type";
        juce::String typeLabel = juce::String(bandLabels[i]) + " Type";
        params.push_back(std::make_unique<juce::AudioParameterChoice>(
            juce::ParameterID(typeId, 1),
            typeLabel,
            juce::StringArray("Bypass", "High-Shelf", "Low-Shelf", "Peaking"),
            bandDefTypes[i]
        ));

        // Band Gain
        juce::String gainId = juce::String(bandNames[i]) + "Gain";
        juce::String gainLabel = juce::String(bandLabels[i]) + " Gain";
        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID(gainId, 1),
            gainLabel,
            juce::NormalisableRange<float>(-12.0f, 12.0f),
            0.0f
        ));

        // Band Frequency
        juce::String freqId = juce::String(bandNames[i]) + "Freq";
        juce::String freqLabel = juce::String(bandLabels[i]) + " Frequency";
        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID(freqId, 1),
            freqLabel,
            juce::NormalisableRange<float>(20.0f, 20000.0f, 0.0f, 0.25f),
            bandDefFreqs[i]
        ));

        // Band Q
        juce::String qId = juce::String(bandNames[i]) + "Q";
        juce::String qLabel = juce::String(bandLabels[i]) + " Q";
        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID(qId, 1),
            qLabel,
            juce::NormalisableRange<float>(0.1f, 10.0f),
            bandDefQs[i]
        ));
    }

    // Limiter Parameters
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("limThreshold", 1),
        "Limiter Threshold",
        juce::NormalisableRange<float>(-20.0f, 0.0f),
        0.0f
    ));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("limRelease", 1),
        "Limiter Release",
        juce::NormalisableRange<float>(10.0f, 1000.0f, 0.0f, 0.33f), // log scaling
        100.0f
    ));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("limMakeup", 1),
        "Limiter Makeup Gain",
        juce::NormalisableRange<float>(0.0f, 12.0f),
        0.0f
    ));

    // Dynamics (Compressor/Expander) Parameters
    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID("dynMode", 1), "Dynamics Mode",
        juce::StringArray("COMP", "EXP", "UP"), 0));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("dynThreshold", 1), "Dyn Threshold",
        juce::NormalisableRange<float>(-60.0f, 0.0f), -18.0f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("dynRatio", 1), "Dyn Ratio",
        juce::NormalisableRange<float>(1.0f, 20.0f, 0.0f, 0.35f), 2.0f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("dynKnee", 1), "Dyn Knee",
        juce::NormalisableRange<float>(0.0f, 20.0f), 6.0f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("dynAttack", 1), "Dyn Attack",
        juce::NormalisableRange<float>(0.1f, 300.0f, 0.0f, 0.35f), 14.0f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("dynRelease", 1), "Dyn Release",
        juce::NormalisableRange<float>(10.0f, 3000.0f, 0.0f, 0.35f), 120.0f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("dynMakeup", 1), "Dyn Makeup",
        juce::NormalisableRange<float>(0.0f, 24.0f), 0.0f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("dynMix", 1), "Dyn Mix",
        juce::NormalisableRange<float>(0.0f, 100.0f), 100.0f));

    // Global Gain Staging
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("inGain", 1), "Input Gain",
        juce::NormalisableRange<float>(-24.0f, 24.0f), 0.0f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("outGain", 1), "Output Gain",
        juce::NormalisableRange<float>(-24.0f, 24.0f), 0.0f));

    // Dynamics Advanced Parameters
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID("dynSidechainHPF", 1), "Sidechain HPF", false));

    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID("dynAutoRelease", 1), "Auto Release", false));

    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID("dynRMS", 1), "RMS Mode", true));

    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID("dynEnabled", 1), "Dynamics Enabled", true));

    return { params.begin(), params.end() };
}

MasteringSuiteProcessor::MasteringSuiteProcessor()
    : AudioProcessor (BusesProperties()
        .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
        .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, &undoManager, "PARAMETERS", createParameterLayout())
{
}

void MasteringSuiteProcessor::prepareToPlay (double sr, int bs) {
    eq.prepare (sr, getTotalNumInputChannels());
    compressor.prepare(sr, getTotalNumInputChannels());
    limiter.prepare(sr, getTotalNumInputChannels(), bs);
    meter.prepare (sr, getTotalNumInputChannels());

    // Apply initial parameter values for 6 EQ bands
    const char* bandNames[] = {"eq0", "eq1", "eq2", "eq3", "eq4", "eq5"};
    for (int i = 0; i < 6; ++i) {
        juce::String typeId = juce::String(bandNames[i]) + "Type";
        juce::String gainId = juce::String(bandNames[i]) + "Gain";
        juce::String freqId = juce::String(bandNames[i]) + "Freq";
        juce::String qId = juce::String(bandNames[i]) + "Q";

        eq.setBandType(i, (MasteringEQ::FilterType)(int)*apvts.getRawParameterValue(typeId.toStdString().c_str()));
        eq.setBandGain(i, *apvts.getRawParameterValue(gainId.toStdString().c_str()));
        eq.setBandFrequency(i, *apvts.getRawParameterValue(freqId.toStdString().c_str()));
        eq.setBandQ(i, *apvts.getRawParameterValue(qId.toStdString().c_str()));
    }

    compressor.setMode((MasteringCompressor::Mode)(int)*apvts.getRawParameterValue("dynMode"));
    compressor.setThreshold(*apvts.getRawParameterValue("dynThreshold"));
    compressor.setRatio(*apvts.getRawParameterValue("dynRatio"));
    compressor.setKnee(*apvts.getRawParameterValue("dynKnee"));
    compressor.setAttack(*apvts.getRawParameterValue("dynAttack"));
    compressor.setRelease(*apvts.getRawParameterValue("dynRelease"));
    compressor.setMakeup(*apvts.getRawParameterValue("dynMakeup"));
    compressor.setMix(*apvts.getRawParameterValue("dynMix") / 100.0f);

    limiter.setThreshold(*apvts.getRawParameterValue("limThreshold"));
    limiter.setRelease(*apvts.getRawParameterValue("limRelease"));
    limiter.setMakeupGain(*apvts.getRawParameterValue("limMakeup"));
}

void MasteringSuiteProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&) {
    juce::ScopedNoDenormals nd;

    // Update DSP parameters for 6 EQ bands in real-time
    const char* bandNames[] = {"eq0", "eq1", "eq2", "eq3", "eq4", "eq5"};
    for (int i = 0; i < 6; ++i) {
        juce::String typeId = juce::String(bandNames[i]) + "Type";
        juce::String gainId = juce::String(bandNames[i]) + "Gain";
        juce::String freqId = juce::String(bandNames[i]) + "Freq";
        juce::String qId = juce::String(bandNames[i]) + "Q";

        eq.setBandType(i, (MasteringEQ::FilterType)(int)*apvts.getRawParameterValue(typeId.toStdString().c_str()));
        eq.setBandGain(i, *apvts.getRawParameterValue(gainId.toStdString().c_str()));
        eq.setBandFrequency(i, *apvts.getRawParameterValue(freqId.toStdString().c_str()));
        eq.setBandQ(i, *apvts.getRawParameterValue(qId.toStdString().c_str()));
    }

    compressor.setMode((MasteringCompressor::Mode)(int)*apvts.getRawParameterValue("dynMode"));
    compressor.setThreshold(*apvts.getRawParameterValue("dynThreshold"));
    compressor.setRatio(*apvts.getRawParameterValue("dynRatio"));
    compressor.setKnee(*apvts.getRawParameterValue("dynKnee"));
    compressor.setAttack(*apvts.getRawParameterValue("dynAttack"));
    compressor.setRelease(*apvts.getRawParameterValue("dynRelease"));
    compressor.setMakeup(*apvts.getRawParameterValue("dynMakeup"));
    compressor.setMix(*apvts.getRawParameterValue("dynMix") / 100.0f);

    limiter.setThreshold(*apvts.getRawParameterValue("limThreshold"));
    limiter.setRelease(*apvts.getRawParameterValue("limRelease"));
    limiter.setMakeupGain(*apvts.getRawParameterValue("limMakeup"));

    // Apply input gain
    float inGainDb = *apvts.getRawParameterValue("inGain");
    float inGainLinear = std::pow(10.0f, inGainDb / 20.0f);
    buffer.applyGain(inGainLinear);

    // Process audio through DSP chain
    eq.processBlock (buffer);

    // Only process dynamics if enabled
    if (*apvts.getRawParameterValue("dynEnabled") > 0.5f)
        compressor.processBlock (buffer);

    limiter.processBlock (buffer);

    // Apply output gain
    float outGainDb = *apvts.getRawParameterValue("outGain");
    float outGainLinear = std::pow(10.0f, outGainDb / 20.0f);
    buffer.applyGain(outGainLinear);

    meter.process (buffer);
}

void MasteringSuiteProcessor::getStateInformation(juce::MemoryBlock& destData) {
    // Serialize APVTS state to XML
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());

    // Add meter state if needed (optional enhancement)
    // For now, just serialize parameters
    copyXmlToBinary(*xml, destData);
}

void MasteringSuiteProcessor::setStateInformation(const void* data, int sizeInBytes) {
    // Deserialize state from XML
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

    if (xmlState && xmlState->hasTagName(apvts.state.getType())) {
        apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
    }
}

juce::AudioProcessorEditor* MasteringSuiteProcessor::createEditor() {
    return new MasteringSuiteEditor (*this);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() {
    return new MasteringSuiteProcessor();
}
