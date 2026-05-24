#include "PluginProcessor.h"
#include "UI/PluginEditor.h"

juce::AudioProcessorValueTreeState::ParameterLayout MasteringSuiteProcessor::createParameterLayout() {
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    // EQ: Single peaking band (Gain/Freq/Q)
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("eq_gain", 1),
        "EQ Gain",
        juce::NormalisableRange<float>(-12.0f, 12.0f),
        0.0f
    ));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("eq_freq", 1),
        "EQ Freq",
        juce::NormalisableRange<float>(20.0f, 20000.0f, 0.0f, 0.25f),
        1000.0f
    ));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("eq_q", 1),
        "EQ Q",
        juce::NormalisableRange<float>(0.5f, 8.0f, 0.0f, 0.5f),
        1.0f
    ));

    // Limiter: Threshold/Release/Makeup
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("lim_thresh", 1),
        "Threshold",
        juce::NormalisableRange<float>(-40.0f, 0.0f),
        -6.0f
    ));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("lim_release", 1),
        "Release",
        juce::NormalisableRange<float>(10.0f, 500.0f, 0.0f, 0.5f),
        120.0f
    ));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("lim_makeup", 1),
        "Makeup",
        juce::NormalisableRange<float>(0.0f, 6.0f),
        0.0f
    ));

    // Global parameters
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID("bypass", 1),
        "Bypass",
        false
    ));

    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID("oversample", 1),
        "Oversample",
        juce::StringArray("1x", "2x", "4x"),
        2  // Default to 4x (index 2)
    ));

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
    return new PluginEditor (*this);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() {
    return new MasteringSuiteProcessor();
}
