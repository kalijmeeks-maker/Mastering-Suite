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
    addAndMakeVisible(*btn);

    btn->onClick = [this, idx = static_cast<int>(buttons.size())] {
        setSelectedIndex(idx);
        if (onSelectionChanged) onSelectionChanged(idx);
    };

    buttons.push_back(std::move(btn));
}

void SegmentedControl::setSelectedIndex(int index)
{
    if (index >= 0 && index < static_cast<int>(buttons.size())) {
        selectedIndex = index;
        buttons[index]->setToggleState(true, juce::NotificationType::sendNotification);
    }
}

void SegmentedControl::connectParameter(juce::AudioProcessorValueTreeState& apvts, const juce::String& paramID)
{
    if (buttons.empty()) return;

    auto param = dynamic_cast<juce::AudioParameterChoice*>(apvts.getParameter(paramID));
    if (!param) return;

    setSelectedIndex(param->getIndex());

    buttons[selectedIndex]->setToggleState(true, juce::NotificationType::dontSendNotification);
    buttons[selectedIndex]->onClick = [this, &apvts, paramID] {
        apvts.getParameter(paramID)->setValueNotifyingHost(selectedIndex / 100.0f);
    };
}
