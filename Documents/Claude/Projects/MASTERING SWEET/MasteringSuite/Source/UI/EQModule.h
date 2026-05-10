#pragma once
#include <JuceHeader.h>
#include "Theme.h"
#include "RotaryKnob.h"

class EQModule : public juce::Component
{
public:
    explicit EQModule(juce::AudioProcessorValueTreeState& apvts);
    ~EQModule() override = default;

    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    struct BandControl {
        std::unique_ptr<juce::ComboBox> typeBox;
        std::unique_ptr<RotaryKnob> freqKnob;
        std::unique_ptr<RotaryKnob> gainKnob;
        std::unique_ptr<RotaryKnob> qKnob;
        std::unique_ptr<juce::ComboBox::Listener> typeListener;
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> freqAttach;
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> gainAttach;
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> qAttach;
    };

    juce::AudioProcessorValueTreeState& apvts;
    std::array<BandControl, 6> bands;

    static constexpr int BAND_COUNT = 6;
    const char* bandNames[6] = {"eq0", "eq1", "eq2", "eq3", "eq4", "eq5"};
    const char* bandLabels[6] = {"HP", "LS", "BELL1", "BELL2", "HS", "LP"};
    const juce::uint32 bandColors[6] = {
        Theme::Color::TEXT,
        Theme::Color::CYAN,
        Theme::Color::ACCENT,
        Theme::Color::MAGENTA,
        Theme::Color::AMBER,
        Theme::Color::TEXT
    };
};
