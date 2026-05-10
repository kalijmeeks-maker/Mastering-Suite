#include "DynamicsModule.h"

DynamicsModule::DynamicsModule(MasteringCompressor& comp, juce::AudioProcessorValueTreeState& apvts_)
    : compressor(comp), apvts(apvts_)
{
    modeControl = std::make_unique<SegmentedControl>();
    modeControl->addButton("COMP");
    modeControl->addButton("EXP");
    modeControl->addButton("UP");
    addAndMakeVisible(*modeControl);
    
    thresholdKnob = std::make_unique<RotaryKnob>();
    ratioKnob = std::make_unique<RotaryKnob>();
    kneeKnob = std::make_unique<RotaryKnob>();
    attackKnob = std::make_unique<RotaryKnob>();
    releaseKnob = std::make_unique<RotaryKnob>();
    makeupKnob = std::make_unique<RotaryKnob>();
    mixKnob = std::make_unique<RotaryKnob>();
    
    for (auto* knob : {thresholdKnob.get(), ratioKnob.get(), kneeKnob.get(), 
                       attackKnob.get(), releaseKnob.get(), makeupKnob.get(), mixKnob.get()}) {
        addAndMakeVisible(*knob);
    }
    
    transferCurve = std::make_unique<TransferCurveComponent>();
    addAndMakeVisible(*transferCurve);
    
    grMeter = std::make_unique<CompressorGRMeter>(compressor);
    addAndMakeVisible(*grMeter);
    
    startTimer(33);
}

DynamicsModule::~DynamicsModule()
{
    stopTimer();
}

void DynamicsModule::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    g.setColour(Theme::Color::toColour(Theme::Color::BG_0));
    g.fillRect(bounds);
}

void DynamicsModule::resized()
{
    auto bounds = getLocalBounds();
    
    auto leftPanel = bounds.removeFromLeft(bounds.getWidth() * 65 / 100);
    auto rightPanel = bounds;
    
    modeControl->setBounds(leftPanel.removeFromTop(40).reduced(10));
    
    auto knobRow1 = leftPanel.removeFromTop(80);
    int knobSize = 56;
    thresholdKnob->setBounds(knobRow1.removeFromLeft(knobSize + 12).reduced(2));
    ratioKnob->setBounds(knobRow1.removeFromLeft(knobSize + 12).reduced(2));
    kneeKnob->setBounds(knobRow1.removeFromLeft(knobSize + 12).reduced(2));
    
    auto knobRow2 = leftPanel.removeFromTop(80);
    attackKnob->setBounds(knobRow2.removeFromLeft(knobSize + 12).reduced(2));
    releaseKnob->setBounds(knobRow2.removeFromLeft(knobSize + 12).reduced(2));
    makeupKnob->setBounds(knobRow2.removeFromLeft(knobSize + 12).reduced(2));
    mixKnob->setBounds(knobRow2.removeFromLeft(knobSize + 12).reduced(2));
    
    grMeter->setBounds(leftPanel.removeFromBottom(40).reduced(10));
    
    transferCurve->setBounds(rightPanel.reduced(10));
}

void DynamicsModule::timerCallback()
{
    if (auto threshold = apvts.getRawParameterValue("dynThreshold")) {
        if (auto ratio = apvts.getRawParameterValue("dynRatio")) {
            if (auto knee = apvts.getRawParameterValue("dynKnee")) {
                transferCurve->setParameters(*threshold, *ratio, *knee, TransferCurveComponent::Mode::Comp);
            }
        }
    }
}
