#include "HeaderBar.h"
#include "../PluginProcessor.h"
#include "PillButton.cpp"
#include "SegmentedControl.cpp"

HeaderBar::HeaderBar(MasteringSuiteProcessor& proc) : processor(proc) {
    // A/B Toggle — magenta fill on active segment only. Now wired to real state banks.
    abToggle.setVariant(PillButton::Variant::Filled);
    abToggle.setAccentColor(juce::Colour(mst::theme::cEqFreq));  // Magenta
    abToggle.addButton("A");
    abToggle.addButton("B");
    abToggle.setSelectedIndex(processor.getActiveBank());
    abToggle.onSelectionChanged = [this](int idx) {
        processor.switchToBank(idx);
    };
    addAndMakeVisible(abToggle);

    // ↔ button: copies active bank into the inactive one ("I like this, let me iterate from here").
    copyABButton.setButtonText(juce::String::fromUTF8("↔"));  // ↔
    copyABButton.setVariant(PillButton::Variant::Outlined);
    copyABButton.setAccentColor(juce::Colour(mst::theme::cEqFreq));
    copyABButton.setTooltip("Copy active bank to the other (A↔B)");
    copyABButton.onClick = [this] { processor.copyActiveBankToOther(); };
    addAndMakeVisible(copyABButton);

    // Preset Button — neutral pill, opens a PopupMenu on click.
    // Menu layout (v1.1-1):
    //   FACTORY: DEFAULT / BRIGHT / WARM / PUNCHY / WIDE     (checked = active)
    //   ─────
    //   USER: <alphabetical>                                  (checked = active)
    //   ─────
    //   + NEW…                                                (always enabled)
    //   Save changes to <current>                             (enabled iff user preset loaded)
    //   Reset to preset                                       (enabled iff snapshot exists)
    presetButton.setButtonText("PRESET: " + currentDisplayName());
    addAndMakeVisible(presetButton);
    presetButton.onClick = [this] {
        juce::PopupMenu menu;
        auto factoryNames = MasteringSuiteProcessor::getPresetNames();
        int activeFactory = processor.getCurrentPreset();
        juce::String activeUser = processor.getCurrentUserPresetName();
        auto userNames = processor.listUserPresets();

        for (int i = 0; i < factoryNames.size(); ++i) {
            menu.addItem(i + 1, factoryNames[i], true,
                         (activeUser.isEmpty() && i == activeFactory));
        }

        if (! userNames.isEmpty()) {
            menu.addSeparator();
            for (int i = 0; i < userNames.size(); ++i) {
                menu.addItem(100 + i, userNames[i], true, userNames[i] == activeUser);
            }
        }

        menu.addSeparator();
        menu.addItem(900, "+ NEW...");
        const bool canSave = activeUser.isNotEmpty();
        juce::String saveLabel = canSave ? juce::String("Save changes to ") + activeUser
                                         : juce::String("Save changes (read-only factory preset)");
        menu.addItem(901, saveLabel, canSave);
        menu.addItem(902, "Reset to preset", processor.isPresetModified());

        menu.showMenuAsync(juce::PopupMenu::Options().withTargetComponent(&presetButton),
            [this, factoryNames, userNames](int result) {
                if (result == 0) return;
                if (result >= 1 && result <= factoryNames.size()) {
                    processor.loadPreset(result - 1);
                } else if (result >= 100 && result < 100 + userNames.size()) {
                    processor.loadUserPreset(userNames[result - 100]);
                } else if (result == 900) {
                    showSavePresetDialog();
                } else if (result == 901) {
                    processor.saveChangesToCurrentPreset();
                } else if (result == 902) {
                    processor.resetToCurrentSnapshot();
                }
                presetButton.setButtonText("PRESET: " + currentDisplayName());
                presetButton.setToggleState(false, juce::dontSendNotification);
            });
    };

    // Bypass Button — amber outlined border + amber LED dot when active
    bypassButton.setButtonText("BYPASS");
    bypassButton.setVariant(PillButton::Variant::Outlined);
    bypassButton.setAccentColor(juce::Colour(0xFFFF9500));  // Amber
    bypassButton.setShowLed(true);
    addAndMakeVisible(bypassButton);
    bypassAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        processor.getAPVTS(), "bypass", bypassButton);

    // Oversample Button — cyan outlined border + LED dot when ON (i.e., not 1x)
    oversampleButton.setVariant(PillButton::Variant::Outlined);
    oversampleButton.setAccentColor(juce::Colour(0xFF6CD6FF));  // Cyan
    oversampleButton.setShowLed(true);
    addAndMakeVisible(oversampleButton);

    auto refreshOversampleLabel = [this] {
        if (auto* param = dynamic_cast<juce::AudioParameterChoice*>(processor.getAPVTS().getParameter("oversample"))) {
            oversampleButton.setButtonText(param->choices[param->getIndex()].toUpperCase() + " OVERSAMPLE");
            // ON state = any setting other than 1x (index 0)
            oversampleButton.setToggleState(param->getIndex() > 0, juce::dontSendNotification);
        }
    };

    oversampleButton.onClick = [this, refreshOversampleLabel] {
        if (auto* param = dynamic_cast<juce::AudioParameterChoice*>(processor.getAPVTS().getParameter("oversample"))) {
            int nextIndex = (param->getIndex() + 1) % param->choices.size();
            param->setValueNotifyingHost(param->getNormalisableRange().convertTo0to1((float)nextIndex));
            refreshOversampleLabel();
        }
    };

    refreshOversampleLabel();
    renderBrandMark();
}

void HeaderBar::refresh() {
    // Append a dim cyan "•" when the current preset's parameter snapshot doesn't
    // match the live state. Reads as "dirty buffer" (asterisks read as "broken").
    juce::String base = "PRESET: " + currentDisplayName();
    juce::String desired = processor.isPresetModified() ? (base + juce::String::fromUTF8(" •")) : base;
    if (presetButton.getButtonText() != desired) {
        presetButton.setButtonText(desired);
        presetButton.repaint();
    }
}

juce::String HeaderBar::currentDisplayName() const {
    juce::String userName = processor.getCurrentUserPresetName();
    if (userName.isNotEmpty()) return userName;
    int idx = processor.getCurrentPreset();
    auto factory = MasteringSuiteProcessor::getPresetNames();
    return (idx >= 0 && idx < factory.size()) ? factory[idx] : juce::String("—");
}

void HeaderBar::showSavePresetDialog() {
    // AlertWindow with a single text editor — pre-fill with the current user
    // preset's name if one is loaded so engineers can quickly clone it.
    auto* alert = new juce::AlertWindow("Save Preset",
                                        "Name this preset:",
                                        juce::AlertWindow::NoIcon);
    juce::String prefill = processor.getCurrentUserPresetName();
    alert->addTextEditor("name", prefill, juce::String());
    alert->addButton("Save",   1, juce::KeyPress(juce::KeyPress::returnKey));
    alert->addButton("Cancel", 0, juce::KeyPress(juce::KeyPress::escapeKey));

    // Auto-focus the text editor so the user can start typing immediately
    // (and so automated tests / accessibility keystrokes actually land).
    if (auto* te = alert->getTextEditor("name")) {
        juce::MessageManager::callAsync([te] {
            te->grabKeyboardFocus();
            te->selectAll();
        });
    }

    alert->enterModalState(true,
        juce::ModalCallbackFunction::create([this, alert](int result) {
            if (result == 1) {
                auto name = alert->getTextEditorContents("name");
                if (name.trim().isNotEmpty())
                    processor.savePresetAs(name);
            }
            // alert auto-deletes due to deleteWhenDismissed=true below
        }),
        true);
}

// Builds a 56×56 retina image of the v2 brand mark: 4-stop radial gradient
// pink → mid-pink → purple → near-black, plus 1px white ring inset 1.5px at 80%.
// Rendered once and blitted on every paint to keep frame cost ~zero.
void HeaderBar::renderBrandMark() {
    const int px = 56;  // 2x of 28pt for retina sharpness
    brandMarkImage = juce::Image(juce::Image::ARGB, px, px, true);
    juce::Graphics g(brandMarkImage);
    auto r = juce::Rectangle<float>(0, 0, (float)px, (float)px);
    juce::Path squircle;
    squircle.addRoundedRectangle(r, 12.0f);  // 6pt corner radius x 2 for retina
    g.reduceClipRegion(squircle);

    juce::ColourGradient grad(
        juce::Colour(0xFFFF5CD1), r.getX() + r.getWidth() * 0.30f, r.getY() + r.getHeight() * 0.30f,
        juce::Colour(0xFF1A0A2A), r.getRight(), r.getBottom(),
        true);
    grad.addColour(0.55f, juce::Colour(0xFF6A1F9A));
    grad.addColour(0.85f, juce::Colour(0xFF2B1240));
    g.setGradientFill(grad);
    g.fillRect(r);

    // 1px inner ring at 80% white opacity, inset 3px (= 1.5pt at logical scale)
    g.setColour(juce::Colours::white.withAlpha(0.35f));
    auto ring = r.reduced(12.0f);  // matches inset 6px in v2 mockup x2
    g.drawEllipse(ring, 2.0f);     // 1pt logical line = 2px retina

    // Subtle border around the squircle
    g.setColour(juce::Colour(0xFF3A1F5A));
    g.drawRoundedRectangle(r.reduced(0.5f), 12.0f, 2.0f);
}

void HeaderBar::paint(juce::Graphics& g) {
    auto bounds = getLocalBounds().toFloat();

    // Background gradient
    juce::ColourGradient bg(
        juce::Colour(mst::theme::panelTop), 0.0f, 0.0f,
        juce::Colour(mst::theme::bgBase), 0.0f, bounds.getHeight(),
        false);
    g.setGradientFill(bg);
    g.fillAll();

    // Bottom border
    g.setColour(juce::Colour(mst::theme::border));
    g.drawHorizontalLine((int)bounds.getBottom() - 1, 0.0f, bounds.getWidth());

    // Brand mark: 28×28 cached radial-gradient squircle (v2 spec).
    auto markX = 14.0f;
    auto markY = (bounds.getHeight() - 28.0f) * 0.5f;
    auto markRect = juce::Rectangle<float>(markX, markY, 28.0f, 28.0f);
    if (brandMarkImage.isValid())
        g.drawImage(brandMarkImage, markRect, juce::RectanglePlacement::stretchToFit);

    // Wordmark "MASTERING SWEET"
    g.setFont(juce::Font(juce::FontOptions(14.0f)).boldened());
    g.setColour(juce::Colour(mst::theme::textHigh));
    g.drawText("MASTERING ", markX + 42.0f, 0, 150, (int)bounds.getHeight(), juce::Justification::centredLeft);

    g.setColour(juce::Colour(mst::theme::cEqFreq)); // Magenta for "SWEET"
    g.drawText("SWEET", markX + 152.0f, 0, 100, (int)bounds.getHeight(), juce::Justification::centredLeft);

    // Tag text "v1.0 · STEREO · 48 kHz"
    g.setFont(juce::Font(juce::FontOptions(9.0f)));
    g.setColour(juce::Colour(mst::theme::textLow));
    g.drawText(juce::String::fromUTF8("v1.0 \u00B7 STEREO \u00B7 48 kHz"), (int)markX + 250, 0, 150, (int)bounds.getHeight(), juce::Justification::centredLeft);
}

void HeaderBar::resized() {
    auto r = getLocalBounds().reduced(14, 0);
    // 120+8+80+8+140+16+72+4+28 = 476, plus 10px slack
    auto rightSide = r.removeFromRight(486);

    // Position from right to left
    oversampleButton.setBounds(rightSide.removeFromRight(120).reduced(0, 14));
    rightSide.removeFromRight(8);   // gap
    bypassButton.setBounds(rightSide.removeFromRight(80).reduced(0, 14));
    rightSide.removeFromRight(8);   // gap
    presetButton.setBounds(rightSide.removeFromRight(140).reduced(0, 14));
    rightSide.removeFromRight(16);  // larger gap before the A/B/copy cluster
    // A/B widened from 60 → 72 so each segment is a comfortable 36px wide and
    // the magenta fill on A doesn't visually overpower the outlined B.
    abToggle.setBounds(rightSide.removeFromRight(72).reduced(0, 14));
    rightSide.removeFromRight(4);
    copyABButton.setBounds(rightSide.removeFromRight(28).reduced(0, 14));
}
