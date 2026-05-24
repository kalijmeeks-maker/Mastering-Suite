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

    // Header bar with logo
    g.setColour(Theme::Color::toColour(Theme::Color::BG_1));
    auto headerBounds = bounds.removeFromTop(50);
    g.fillRect(headerBounds);

    // SWEET logo
    g.setColour(Theme::Color::toColour(Theme::Color::ACCENT));
    g.setFont(Theme::Font::make(24.0f, Theme::Font::WEIGHT_BOLD));
    g.drawText("SWEET", headerBounds.withLeft(14), juce::Justification::centredLeft);

    // Module title and controls indicator
    g.setColour(Theme::Color::toColour(Theme::Color::TEXT_BRIGHT));
    g.setFont(Theme::Font::make(12.0f, Theme::Font::WEIGHT_BOLD));
    g.drawText("6-BAND EQUALIZER", headerBounds.withLeft(100), juce::Justification::centredLeft);

    // Top divider line
    g.setColour(Theme::Color::toColour(Theme::Color::LINE));
    g.drawHorizontalLine((int)headerBounds.getBottom() - 1, 0, getWidth());
}

void MasteringSuiteEditor::resized()
{
    auto bounds = getLocalBounds();
    bounds.removeFromTop(50); // Header

    // Frequency response curve (45% of remaining space)
    auto curveBounds = bounds.removeFromTop((int)(bounds.getHeight() * 0.45f));
    freqCurve->setBounds(curveBounds.reduced(12, 10));

    // Control grid (50% of remaining space)
    auto controlBounds = bounds.removeFromTop((int)(getHeight() * 0.45f)).reduced(12, 10);
    int bandWidth = controlBounds.getWidth() / 6;
    int knobSize = 48;
    int labelHeight = 16;
    int spacing = 6;

    for (int i = 0; i < 6; ++i) {
        auto bandX = controlBounds.getX() + i * bandWidth;
        auto bandBounds = juce::Rectangle<int>(bandX, controlBounds.getY(), bandWidth, controlBounds.getHeight());
        bandBounds = bandBounds.reduced(spacing);

        int y = bandBounds.getY();

        // Type selector at top
        bandTypeBoxes[i]->setBounds(bandBounds.withHeight(labelHeight).withY(y));
        y += labelHeight + spacing;

        // Frequency knob
        auto knobX = bandBounds.getCentreX() - knobSize / 2;
        bandFreqKnobs[i]->setBounds(knobX, y, knobSize, knobSize);
        y += knobSize + spacing;

        // Gain knob
        bandGainKnobs[i]->setBounds(knobX, y, knobSize, knobSize);
        y += knobSize + spacing;

        // Q knob
        bandQKnobs[i]->setBounds(knobX, y, knobSize, knobSize);
    }

    // Bottom status bar (5% of remaining space)
    auto statusBounds = bounds.removeFromBottom((int)(getHeight() * 0.08f));
    lufsLabel.setBounds(statusBounds.withLeft(12).withWidth(150).reduced(0, 2));
    grLabel.setBounds(statusBounds.withRight(getWidth() - 12).withLeft(getWidth() - 162).reduced(0, 2));
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
