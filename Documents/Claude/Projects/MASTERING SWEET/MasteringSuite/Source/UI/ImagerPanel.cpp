#include "ImagerPanel.h"
#include "../PluginProcessor.h"

ImagerPanel::ImagerPanel(MasteringSuiteProcessor& proc, NeonLookAndFeel& laf)
    : processor(proc), lookAndFeel(laf) {

    auto createKnob = [&](std::unique_ptr<juce::Slider>& k, std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>& a, juce::String paramID, juce::Colour color) {
        k = std::make_unique<juce::Slider>(juce::Slider::RotaryVerticalDrag, juce::Slider::TextBoxBelow);
        k->setLookAndFeel(&lookAndFeel);
        k->setColour(juce::Slider::rotarySliderFillColourId, color);
        a = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(processor.getAPVTS(), paramID, *k);
        if (auto* param = processor.getAPVTS().getParameter(paramID))
            k->setTooltip(param->getName(64));
        addAndMakeVisible(*k);
    };

    createKnob(widthK, widthA, "imgWidth", juce::Colour(mst::theme::tabImg));
    createKnob(panK, panA, "imgPan", juce::Colour(mst::theme::tabImg));

    // Two-state pill: AUTO (RMS) vs FIXED -20 dB. Cycles the gonioScale choice param.
    scaleToggle = std::make_unique<PillButton>();
    scaleToggle->setVariant(PillButton::Variant::Outlined);
    scaleToggle->setAccentColor(juce::Colour(mst::theme::tabImg));
    addAndMakeVisible(*scaleToggle);
    auto refreshToggleLabel = [this] {
        if (auto* p = dynamic_cast<juce::AudioParameterChoice*>(processor.getAPVTS().getParameter("gonioScale"))) {
            scaleToggle->setButtonText(p->choices[p->getIndex()].toUpperCase());
            scaleToggle->setToggleState(p->getIndex() == 0, juce::dontSendNotification);
        }
    };
    scaleToggle->onClick = [this, refreshToggleLabel] {
        if (auto* p = dynamic_cast<juce::AudioParameterChoice*>(processor.getAPVTS().getParameter("gonioScale"))) {
            int next = (p->getIndex() + 1) % p->choices.size();
            p->setValueNotifyingHost(p->getNormalisableRange().convertTo0to1((float)next));
            refreshToggleLabel();
        }
    };
    refreshToggleLabel();
}

void ImagerPanel::paint(juce::Graphics& g) {
    auto bounds = getLocalBounds().toFloat();

    // Background
    juce::ColourGradient bg(
        juce::Colour(mst::theme::panelTop), 0.0f, 0.0f,
        juce::Colour(mst::theme::panelInner), 0.0f, bounds.getHeight(),
        false);
    g.setGradientFill(bg);
    g.fillRoundedRectangle(bounds, 8.0f);
    g.setColour(juce::Colour(mst::theme::border).withAlpha(0.3f));
    g.drawRoundedRectangle(bounds, 8.0f, 1.0f);

    // Header
    g.setFont(juce::Font(11.0f).boldened());
    g.setColour(juce::Colour(mst::theme::textHigh));
    g.drawText(juce::String::fromUTF8("IMAGER \u00B7 STEREO FIELD"), 14, 8, (int)bounds.getWidth() - 28, 14, juce::Justification::topLeft);

    // Goniometer Area
    auto visualArea = bounds.reduced(100, 40).withY(40).withHeight(bounds.getHeight() - 150);
    lastGonioArea = visualArea;
    drawGoniometer(g, visualArea);
}

void ImagerPanel::drawGoniometer(juce::Graphics& g, juce::Rectangle<float> area) {
    // Backdrop + framing
    g.setColour(juce::Colour(mst::theme::bgBase));
    g.fillEllipse(area);
    g.setColour(juce::Colour(mst::theme::border).withAlpha(0.5f));
    g.drawEllipse(area, 1.0f);
    g.drawVerticalLine((int)area.getCentreX(), area.getY(), area.getBottom());
    g.drawHorizontalLine((int)area.getCentreY(), area.getX(), area.getRight());

    std::array<float, 1024> left, right;
    processor.getGoniometerSamples(left.data(), right.data());

    // RMS over the last 1024-sample window (≈ 21ms @ 48k). Raw RMS drove the
    // AUTO-scale norm directly, but block-to-block variance made the scatter's
    // size pop visibly on dynamic material. Now we smooth via a one-pole EMA
    // (α=0.15 → ~200ms time constant) so the scale settles cleanly. Loud→quiet
    // transitions take a beat to shrink — Design's call, intentional.
    double sumSq = 0.0;
    for (int i = 0; i < 1024; ++i) {
        sumSq += (double)left[i]  * left[i];
        sumSq += (double)right[i] * right[i];
    }
    const float rmsRaw = (float)std::sqrt(sumSq / (2.0 * 1024));
    if (rmsSmoothed < 0.0f) {
        rmsSmoothed = rmsRaw;                // first frame: snap, no settle-in
    } else {
        constexpr float alpha = 0.15f;
        rmsSmoothed = alpha * rmsRaw + (1.0f - alpha) * rmsSmoothed;
    }
    const float rms = rmsSmoothed;
    const float rmsDb = juce::Decibels::gainToDecibels(rms, -100.0f);

    const float radius = juce::jmin(area.getWidth(), area.getHeight()) * 0.45f;
    const float cx = area.getCentreX();
    const float cy = area.getCentreY();

    // Low-signal caption per Design — no scatter, just a dim inline label.
    if (rmsDb < -60.0f) {
        g.setColour(juce::Colour(mst::theme::textLow).withAlpha(0.7f));
        g.setFont(juce::Font(10.0f).boldened());
        g.drawText("INPUT TOO QUIET",
                   area.withSizeKeepingCentre(area.getWidth(), 20).toNearestInt(),
                   juce::Justification::centred);
        return;
    }

    // Pick scale mode. Default param value (0) is AUTO; 1 is FIXED -20 dBFS.
    int mode = 0;
    if (auto* m = processor.getAPVTS().getRawParameterValue("gonioScale"))
        mode = (int)*m;

    float norm;
    if (mode == 1) {
        // FIXED: a full-scale (1.0) sample reaches 80% of the field at -20 dBFS reference.
        norm = 1.0f / 0.1f;   // = 10x — so 0.1 (-20 dBFS) maps to 1.0 (full radius)
    } else {
        // AUTO RMS: 2× current RMS reaches 80% radius, with a sane floor.
        const float effRms = juce::jmax(0.005f, rms);  // floor -46 dBFS so micro-quiet signals still draw something
        norm = 0.8f / (2.0f * effRms);
    }

    const juce::Colour scatter = juce::Colour(mst::theme::tabImg);

    // Persistence-of-vision trail per Design v1.0.2 §1: maintain a ring of
    // recent stereo samples across frames, then draw each with an alpha that
    // decays with age. Newest dots full-bright; oldest fade to transparent.
    // Explicit per-dot alpha, not an overlay rect — keeps the violet color
    // pure and avoids the muddied look of fade-rect compositing.
    //
    // Decimation: 1024 audio samples / 25 ≈ 41 new entries per repaint.
    // 30 Hz × 41 = 1230 samples/sec → 960-entry ring drains in ~780 ms.
    constexpr int decimation = 25;
    for (int i = 0; i < 1024; i += decimation) {
        scopePoints.push_back({ left[i], right[i] });  // x = L, y = R
    }
    while ((int)scopePoints.size() > maxPoints) {
        scopePoints.pop_front();
    }

    // Draw oldest → newest so fresh dots paint on top of decayed ones. Front
    // of deque is oldest (age = 1), back is newest (age = 0). Alpha curve
    // pow(1 - age, 1.8) per spec — exponent > 1 keeps the head bright and
    // pulls the tail down sharply at the end (looks like a comet, not a fog).
    const int ringN = (int)scopePoints.size();
    for (int i = 0; i < ringN; ++i) {
        const auto& s = scopePoints[(size_t)i];
        const float l = s.x * norm;
        const float r = s.y * norm;
        // M/S basis: vertical = (L+R)/√2, horizontal = (L-R)/√2
        float sx = (l - r) * 0.7071f * radius;
        float sy = (l + r) * 0.7071f * radius;
        sx = juce::jlimit(-radius, radius, sx);
        sy = juce::jlimit(-radius, radius, sy);
        const float dx = cx + sx;
        const float dy = cy - sy;
        if (!std::isfinite(dx) || !std::isfinite(dy)) continue;

        const float age   = (ringN > 1) ? 1.0f - (float)i / (float)(ringN - 1) : 0.0f;
        const float alpha = std::pow(1.0f - age, 1.8f);
        g.setColour(scatter.withAlpha(alpha));
        g.fillEllipse(dx - 1.2f, dy - 1.2f, 2.4f, 2.4f);
    }
}

void ImagerPanel::resized() {
    auto bounds = getLocalBounds();
    auto knobArea = bounds.reduced(20, 20);
    knobArea.removeFromTop(bounds.getHeight() * 0.7f);

    int knobW = 80;
    widthK->setBounds(bounds.getWidth() / 2 - knobW - 10, knobArea.getY(), knobW, knobW);
    panK->setBounds(bounds.getWidth() / 2 + 10, knobArea.getY(), knobW, knobW);

    // Goniometer scale toggle: small pill anchored bottom-right under the goniometer area.
    if (scaleToggle)
        scaleToggle->setBounds(bounds.getWidth() - 90, knobArea.getY() + 8, 70, 22);
}

void ImagerPanel::refresh() {
    repaint();
}
