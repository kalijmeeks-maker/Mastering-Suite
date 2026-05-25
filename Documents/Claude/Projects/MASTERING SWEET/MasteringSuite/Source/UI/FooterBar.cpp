#include "FooterBar.h"
#include "../PluginProcessor.h"

FooterBar::FooterBar(MasteringSuiteProcessor& proc) : processor(proc) {
    densityToggle = std::make_unique<SegmentedControl>();
    densityToggle->setVariant(PillButton::Variant::Filled);
    densityToggle->setAccentColor(juce::Colour(mst::theme::textHigh));
    densityToggle->addButton("COMPACT");
    densityToggle->addButton("DEFAULT");
    densityToggle->addButton("EXPANDED");
    addAndMakeVisible(*densityToggle);

    // Bind to APVTS density parameter (drives EqBandCell / panel layouts)
    densityToggle->connectParameter(processor.getAPVTS(), "density");
}

void FooterBar::paint(juce::Graphics& g) {
    auto bounds = getLocalBounds().toFloat();

    // Background
    g.setColour(juce::Colour(mst::theme::bgBase));
    g.fillRect(bounds);

    // Top border
    g.setColour(juce::Colour(mst::theme::border));
    g.drawHorizontalLine(0, 0.0f, bounds.getWidth());

    // Footer left: IN, OUT, SR, BUF readouts
    auto sr = (int)(processor.getSampleRate() / 1000.0);
    auto buf = processor.getBlockSize();

    float inPeak = processor.getInputPeak();
    float outPeak = processor.getOutputPeak();

    juce::String inStr = (inPeak > -70.0f) ? juce::String(inPeak, 1) : "-inf";
    juce::String outStr = (outPeak > -70.0f) ? juce::String(outPeak, 1) : "-inf";

    juce::String leftText = juce::String::fromUTF8("IN ") + inStr + " dB \u00B7 OUT " + outStr + " dB \u00B7 SR "
                         + juce::String(sr) + "K \u00B7 BUF " + juce::String(buf);

    g.setFont(juce::Font(9.0f));
    g.setColour(juce::Colour(mst::theme::textMid));
    g.drawText(leftText, 14, 0, 300, (int)bounds.getHeight(), juce::Justification::centredLeft);

    // \u2500\u2500 v1.0.2 \u00A73 Toast / density-toggle crossfade \u2500\u2500\u2500\u2500\u2500\u2500\u2500\u2500\u2500\u2500\u2500\u2500\u2500\u2500\u2500\u2500\u2500\u2500\u2500\u2500\u2500\u2500\u2500\u2500
    // While a knob/handle is being dragged, the center slot shows a status
    // toast (module-accent colored). On mouseUp it fades out over 300 ms and
    // the density toggle fades back in starting 150 ms into that fade.
    const double now = juce::Time::getMillisecondCounterHiRes();
    float toastAlpha  = 0.0f;   // 0..1
    float toggleAlpha = 1.0f;   // density toggle visibility

    if (toastActive) {
        toastAlpha  = 1.0f;
        toggleAlpha = 0.0f;     // toggle hidden under active toast
    } else if (toastFadeStartMs > 0.0) {
        // Fade-out path.  toast: 300 ms ease-out \u2192 0. toggle: starts at 150 ms,
        // fades in over 200 ms.  Crossfade, not snap.
        constexpr double kToastFadeMs   = 300.0;
        constexpr double kToggleDelayMs = 150.0;
        constexpr double kToggleFadeMs  = 200.0;

        const double elapsed = now - toastFadeStartMs;
        toastAlpha = juce::jlimit(0.0f, 1.0f, 1.0f - (float)(elapsed / kToastFadeMs));
        if (elapsed >= kToggleDelayMs) {
            toggleAlpha = juce::jlimit(0.0f, 1.0f,
                (float)((elapsed - kToggleDelayMs) / kToggleFadeMs));
        } else {
            toggleAlpha = 0.0f;
        }
        if (elapsed >= kToastFadeMs && elapsed >= (kToggleDelayMs + kToggleFadeMs)) {
            toastFadeStartMs = 0.0;   // fade complete
            toastAlpha = 0.0f;
            toggleAlpha = 1.0f;
        }
    }

    // Footer center: Density toggle (alpha-faded around toast).
    densityToggle->setBounds(bounds.getCentreX() - 100, 4, 200, 24);
    densityToggle->setAlpha(toggleAlpha);

    if (toastAlpha > 0.01f) {
        g.setFont(juce::Font(11.0f).boldened());
        // Leading colored dot (7 px, currentColor glow handled by drawing a
        // soft inner circle).
        const float cx = bounds.getCentreX() - 78.0f;
        const float cy = bounds.getCentreY();
        g.setColour(toastColor.withAlpha(0.35f * toastAlpha));
        g.fillEllipse(cx - 5.5f, cy - 5.5f, 11.0f, 11.0f);      // soft halo
        g.setColour(toastColor.withAlpha(toastAlpha));
        g.fillEllipse(cx - 3.5f, cy - 3.5f, 7.0f,  7.0f);       // core dot
        // Text \u2014 drawn in module accent.
        g.setColour(toastColor.withAlpha(toastAlpha));
        g.drawText(toastText, (int)(cx + 10.0f), 0, 260, (int)bounds.getHeight(),
                   juce::Justification::centredLeft);
    }

    // Footer right: green dot + DSP %, CPU %, plus a context-aware status slot.
    // The slot shows v1.0 at rest; transient toasts ("Preset loaded: PUNCHY",
    // "Sample rate: 96 kHz") fade in for 2s on relevant events.
    g.setColour(juce::Colour(mst::theme::statusGreen));
    g.fillEllipse(bounds.getRight() - 280.0f, bounds.getCentreY() - 3.0f, 6.0f, 6.0f);

    float cpu = processor.getCPUUsage() * 100.0f;
    juce::String metrics = juce::String::fromUTF8("DSP ") + juce::String(cpu, 1)
                         + juce::String::fromUTF8("% \u00B7 CPU ") + juce::String(cpu * 0.8f, 1) + "%";
    g.setFont(juce::Font(9.0f));
    g.setColour(juce::Colour(mst::theme::textMid));
    g.drawText(metrics, bounds.getRight() - 270.0f, 0, 130, (int)bounds.getHeight(), juce::Justification::centredLeft);

    // Status slot (right-aligned, mono).
    juce::String preset = processor.getStatusMessage();
    juce::String statusText = preset.isNotEmpty() ? preset : juce::String("v1.0");
    g.setColour(preset.isNotEmpty() ? juce::Colour(mst::theme::tabEq)   // cyan when active preset toast
                                    : juce::Colour(mst::theme::textLow));
    g.drawText(statusText, bounds.getRight() - 140.0f, 0, 130, (int)bounds.getHeight(),
               juce::Justification::centredRight);
}

void FooterBar::setToast(juce::String text, juce::Colour accent) {
    toastText  = std::move(text);
    toastColor = accent;
    toastActive = true;
    toastFadeStartMs = 0.0;
    repaint();
}

void FooterBar::clearToast() {
    if (toastActive) {
        toastActive = false;
        toastFadeStartMs = juce::Time::getMillisecondCounterHiRes();
        repaint();
    }
}

void FooterBar::resized() {}

void FooterBar::refresh() {
    repaint();
}
