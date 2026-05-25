#include "SegmentedControl.h"

SegmentedControl::SegmentedControl()
{
}

void SegmentedControl::paint(juce::Graphics& g)
{
}

void SegmentedControl::resized()
{
    auto bounds = getLocalBounds();
    int buttonWidth = bounds.getWidth() / static_cast<int>(buttons.size());

    for (size_t i = 0; i < buttons.size(); ++i) {
        auto btnBounds = bounds.withX(bounds.getX() + static_cast<int>(i) * buttonWidth)
                                .withWidth(buttonWidth)
                                .reduced(2);
        buttons[i]->setBounds(btnBounds);
        buttons[i]->setSegmentGroup(i == 0, i == buttons.size() - 1);
    }
}

void SegmentedControl::addButton(const juce::String& label)
{
    auto btn = std::make_unique<PillButton>();
    btn->setButtonText(label);
    btn->setRadioGroupId(12345);
    btn->setAccentColor(buttonAccent);
    btn->setVariant(buttonVariant);
    addAndMakeVisible(*btn);

    btn->onClick = [this, idx = static_cast<int>(buttons.size())] {
        setSelectedIndex(idx);
    };

    buttons.push_back(std::move(btn));
}

void SegmentedControl::setSelectedIndex(int index)
{
    index = juce::jlimit(0, juce::jmax(0, (int)buttons.size() - 1), index);
    if (index == selectedIndex) return;  // breaks param<->UI feedback loop

    selectedIndex = index;
    for (int i = 0; i < (int)buttons.size(); ++i) {
        buttons[i]->setToggleState(i == index, juce::dontSendNotification);
    }
    if (onSelectionChanged) onSelectionChanged(index);
    repaint();
}

void SegmentedControl::connectParameter(juce::AudioProcessorValueTreeState& apvts, const juce::String& paramID)
{
    if (buttons.empty()) return;

    auto* param = dynamic_cast<juce::AudioParameterChoice*>(apvts.getParameter(paramID));
    if (!param) return;

    setSelectedIndex(param->getIndex());

    onSelectionChanged = [param](int idx) {
        param->setValueNotifyingHost(
            param->getNormalisableRange().convertTo0to1((float)idx));
    };

    paramAttachment = std::make_unique<juce::ParameterAttachment>(
        *param,
        [this](float newValue) { setSelectedIndex((int)newValue); });
}

void SegmentedControl::setAccentColor(juce::Colour c)
{
    buttonAccent = c;
    for (auto& b : buttons) b->setAccentColor(c);
}

void SegmentedControl::setVariant(PillButton::Variant v)
{
    buttonVariant = v;
    for (auto& b : buttons) b->setVariant(v);
}
