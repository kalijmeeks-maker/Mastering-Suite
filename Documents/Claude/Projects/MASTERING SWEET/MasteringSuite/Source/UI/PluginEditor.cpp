#include "PluginEditor.h"
#include "../PluginProcessor.h"

MasteringSuiteEditor::MasteringSuiteEditor(MasteringSuiteProcessor& proc)
    : AudioProcessorEditor(proc), processor(proc)
{
    setResizable(false, false);

    freqCurve = std::make_unique<FrequencyResponseCurve>(processor.getEQ());
    addAndMakeVisible(*freqCurve);

    const char* bandNames[] = {"eq0", "eq1", "eq2", "eq3", "eq4", "eq5"};
    const char* bandLabels[] = {"HP", "LS", "BELL1", "BELL2", "HS", "LP"};

    for (int i = 0; i < 6; ++i) {
        // Frequency knob
        bandFreqKnobs[i] = std::make_unique<RotaryKnob>();
        juce::String freqId = juce::String(bandNames[i]) + "Freq";
        freqAttachments[i] = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            processor.getAPVTS(), freqId.toStdString(), *bandFreqKnobs[i]
        );
        addAndMakeVisible(*bandFreqKnobs[i]);

        // Gain knob
        bandGainKnobs[i] = std::make_unique<RotaryKnob>();
        juce::String gainId = juce::String(bandNames[i]) + "Gain";
        gainAttachments[i] = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            processor.getAPVTS(), gainId.toStdString(), *bandGainKnobs[i]
        );
        addAndMakeVisible(*bandGainKnobs[i]);

        // Q knob
        bandQKnobs[i] = std::make_unique<RotaryKnob>();
        juce::String qId = juce::String(bandNames[i]) + "Q";
        qAttachments[i] = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            processor.getAPVTS(), qId.toStdString(), *bandQKnobs[i]
        );
        addAndMakeVisible(*bandQKnobs[i]);

        // Type box
        bandTypeBoxes[i] = std::make_unique<juce::ComboBox>();
        bandTypeBoxes[i]->addItem("Bypass", 1);
        bandTypeBoxes[i]->addItem("H-Shelf", 2);
        bandTypeBoxes[i]->addItem("L-Shelf", 3);
        bandTypeBoxes[i]->addItem("Peaking", 4);
        juce::String typeId = juce::String(bandNames[i]) + "Type";
        bandTypeBoxes[i]->setSelectedItemIndex((int)*processor.getAPVTS().getRawParameterValue(typeId.toStdString().c_str()), juce::dontSendNotification);
        addAndMakeVisible(*bandTypeBoxes[i]);
    }

    lufsLabel.setText("LUFS: -∞", juce::dontSendNotification);
    lufsLabel.setFont(Theme::Font::make(22.0f, Theme::Font::WEIGHT_BOLD));
    lufsLabel.setColour(juce::Label::textColourId, Theme::Color::toColour(Theme::Color::ACCENT));
    addAndMakeVisible(lufsLabel);

    grLabel.setText("GR: 0 dB", juce::dontSendNotification);
    grLabel.setFont(Theme::Font::make(16.0f, Theme::Font::WEIGHT_BOLD));
    grLabel.setColour(juce::Label::textColourId, Theme::Color::toColour(Theme::Color::CYAN));
    addAndMakeVisible(grLabel);

    startTimer(33);

    // Defer size setting to after editor initialization
    juce::MessageManager::callAsync([this]() {
        this->setSize(1000, 700);
    });
}

MasteringSuiteEditor::~MasteringSuiteEditor()
{
    stopTimer();
}

void MasteringSuiteEditor::paint(juce::Graphics& g)
{
    g.fillAll(Theme::Color::toColour(Theme::Color::BG_0));

    auto bounds = getLocalBounds().toFloat();

    // Header
    g.setColour(Theme::Color::toColour(Theme::Color::BG_1));
    auto headerBounds = bounds.removeFromTop(44);
    g.fillRect(headerBounds);
    g.setColour(Theme::Color::toColour(Theme::Color::TEXT_BRIGHT));
    g.setFont(Theme::Font::make(18.0f, Theme::Font::WEIGHT_BOLD));
    g.drawText("SWEET MASTERING", headerBounds.reduced(14, 0), juce::Justification::centredLeft);

    // Module title
    g.setColour(Theme::Color::toColour(Theme::Color::TEXT_BRIGHT));
    g.setFont(Theme::Font::make(13.0f, Theme::Font::WEIGHT_BOLD));
    auto titleBounds = bounds.removeFromTop(44);
    g.drawText("EQUALIZER", titleBounds.reduced(14, 0), juce::Justification::centredLeft);
}

void MasteringSuiteEditor::resized()
{
    auto bounds = getLocalBounds();
    bounds.removeFromTop(44); // Header
    bounds.removeFromTop(44); // Title

    // Frequency response curve (top 60% of remaining space)
    auto curveBounds = bounds.removeFromTop(bounds.getHeight() * 0.60f);
    freqCurve->setBounds(curveBounds.reduced(14));

    // Control deck (bottom 40%)
    auto controlBounds = bounds.reduced(14);
    int bandWidth = controlBounds.getWidth() / 6;
    int knobSize = Theme::Layout::KNOB_SIZE_SMALL;
    int spacing = 8;

    for (int i = 0; i < 6; ++i) {
        auto bandBounds = controlBounds.withLeft(controlBounds.getX() + i * bandWidth)
                                       .withRight(controlBounds.getX() + (i + 1) * bandWidth);
        bandBounds = bandBounds.reduced(spacing);

        int y = bandBounds.getY();
        bandTypeBoxes[i]->setBounds(bandBounds.withHeight(22).translated(0, y));
        y += 30;

        bandFreqKnobs[i]->setBounds(bandBounds.withLeft(bandBounds.getCentreX() - knobSize/2)
                                               .withTop(y)
                                               .withWidth(knobSize)
                                               .withHeight(knobSize));
        y += knobSize + spacing;

        bandGainKnobs[i]->setBounds(bandBounds.withLeft(bandBounds.getCentreX() - knobSize/2)
                                               .withTop(y)
                                               .withWidth(knobSize)
                                               .withHeight(knobSize));
        y += knobSize + spacing;

        bandQKnobs[i]->setBounds(bandBounds.withLeft(bandBounds.getCentreX() - knobSize/2)
                                            .withTop(y)
                                            .withWidth(knobSize)
                                            .withHeight(knobSize));
    }

    lufsLabel.setBounds(getLocalBounds().getWidth() - 180, 10, 160, 24);
    grLabel.setBounds(getLocalBounds().getWidth() - 180, 40, 160, 20);
}

void MasteringSuiteEditor::timerCallback()
{
    auto& meter = processor.getMeter();
    lufsIntegrated = meter.getIntegratedLufs();
    grCurrent = processor.getLimiter().getCurrentGainReduction();

    juce::String lufsText;
    if (std::isfinite(lufsIntegrated))
        lufsText = juce::String(lufsIntegrated, 1) + " LUFS";
    else
        lufsText = "-∞ LUFS";
    lufsLabel.setText(lufsText, juce::dontSendNotification);

    grLabel.setText(juce::String(grCurrent, 1) + " dB", juce::dontSendNotification);

    freqCurve->repaint();
}
