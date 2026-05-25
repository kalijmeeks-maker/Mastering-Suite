#include "LimiterPanel.h"
#include "../PluginProcessor.h"

LimiterPanel::LimiterPanel(MasteringSuiteProcessor& proc, NeonLookAndFeel& laf)
    : processor(proc), lookAndFeel(laf) {

    auto createKnob = [&](std::unique_ptr<juce::Slider>& k, std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>& a, juce::String paramID, juce::Colour color) {
        k = std::make_unique<juce::Slider>(juce::Slider::RotaryVerticalDrag, juce::Slider::NoTextBox);
        k->setLookAndFeel(&lookAndFeel);
        k->setColour(juce::Slider::rotarySliderFillColourId, color);
        a = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(processor.getAPVTS(), paramID, *k);
        if (auto* param = processor.getAPVTS().getParameter(paramID))
            k->setTooltip(param->getName(64));
        addAndMakeVisible(*k);
    };

    createKnob(threshK,  threshA,  "limThreshold", juce::Colour(mst::theme::tabLim));
    createKnob(ceilingK, ceilingA, "limCeiling",   juce::Colour(mst::theme::tabLim));
    createKnob(releaseK, releaseA, "limRelease",   juce::Colour(mst::theme::tabLim));
    createKnob(makeupK,  makeupA,  "limMakeup",    juce::Colour(mst::theme::tabLim));

    // styleC + truePeakB still exist for state-attachment purposes (hidden — the
    // mode picker on the panel itself drives limStyle via paint+mouseDown).
    styleC = std::make_unique<juce::ComboBox>();
    styleC->addItemList(juce::StringArray("Transparent", "Punchy", "Warm"), 1);
    styleA = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(processor.getAPVTS(), "limStyle", *styleC);
    addAndMakeVisible(*styleC);
    styleC->setVisible(false);

    truePeakB = std::make_unique<juce::ToggleButton>("TRUE PEAK");
    truePeakA = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(processor.getAPVTS(), "limTruePeak", *truePeakB);
    addAndMakeVisible(*truePeakB);
    truePeakB->setVisible(false);
}

void LimiterPanel::paint(juce::Graphics& g) {
    auto bounds = getLocalBounds().toFloat();

    juce::ColourGradient bg(
        juce::Colour(mst::theme::panelTop),   0.0f, 0.0f,
        juce::Colour(mst::theme::panelInner), 0.0f, bounds.getHeight(),
        false);
    g.setGradientFill(bg);
    g.fillRoundedRectangle(bounds, 8.0f);
    g.setColour(juce::Colour(mst::theme::border).withAlpha(0.3f));
    g.drawRoundedRectangle(bounds, 8.0f, 1.0f);

    g.setFont(juce::Font(11.0f).boldened());
    g.setColour(juce::Colour(mst::theme::textHigh));
    g.drawText(juce::String::fromUTF8("LIMITER · TRUE PEAK"),
               14, 8, (int)bounds.getWidth() - 28, 14, juce::Justification::topLeft);

    // ── Mode picker (left column) ─────────────────────────────────────
    drawModePicker(g, modeArea);

    // ── Knob labels (center 2×2) ──────────────────────────────────────
    g.setFont(juce::Font(9.0f));
    g.setColour(juce::Colour(mst::theme::textMid));
    auto labelUnder = [&](juce::Component& k, juce::String text) {
        g.drawText(text, k.getX(), k.getBottom() - 2, k.getWidth(), 14, juce::Justification::centred);
    };
    labelUnder(*threshK,  "THRESH");
    labelUnder(*ceilingK, "CEILING");
    labelUnder(*releaseK, "RELEASE");
    labelUnder(*makeupK,  "MAKEUP");

    // ── Large segmented GR meter (right column) ───────────────────────
    auto m = meterArea.toFloat().reduced(4);
    g.setColour(juce::Colour(mst::theme::panelInner));
    g.fillRoundedRectangle(m, 4.0f);
    g.setColour(juce::Colour(mst::theme::border).withAlpha(0.4f));
    g.drawRoundedRectangle(m, 4.0f, 1.0f);

    // Header label centered above the segments
    g.setFont(juce::Font(8.0f).boldened());
    g.setColour(juce::Colour(mst::theme::textLow));
    g.drawText("GR", m.toNearestInt().removeFromTop(14), juce::Justification::centred);

    auto segArea = m.reduced(6, 18);
    const int N = 24;
    const float gap = 1.5f;
    const float segH = (segArea.getHeight() - gap * (N - 1)) / (float)N;

    float gr = std::abs(processor.getLimiter().getCurrentGainReduction());
    float norm = juce::jlimit(0.0f, 1.0f, gr / 18.0f);
    int lit = (int)(norm * N);

    // ── v1.0.2 §2 pulse + throb motion (state ticked in refresh()) ────────
    const double now = juce::Time::getMillisecondCounterHiRes();
    float glowAlpha = 0.0f;
    float yScale    = 1.0f;
    juce::Colour glowColor = juce::Colour(0xFFFF8A3A);  // amber baseline

    if (isThrobbing) {
        // 1.2 s sin cycle. Phase∈[0,1] → glow alpha lerp 0.35 → 0.55 (+60% peak).
        const double t = (now - throbRefMs) / 1200.0;
        const float phase = (float)(0.5 + 0.5 * std::sin(t * 2.0 * juce::MathConstants<double>::pi));
        glowAlpha = juce::jmap(phase, 0.0f, 1.0f, 0.35f, 0.55f);
        glowColor = juce::Colour(0xFFFF8A3A);   // amber baseline, no red lerp on throb
    } else if (isPulsing) {
        // 240 ms ease-out: triangular rise 0→peak at 35%, fall peak→0 by 100%.
        // glow lerps amber → red → amber; scaleY 1.0 → 1.025 → 1.0.
        const float t = (float)((now - pulseStartMs) / 240.0);
        const float t01 = juce::jlimit(0.0f, 1.0f, t);
        constexpr float peakAt = 0.35f;
        const float p = (t01 < peakAt) ? (t01 / peakAt) : (1.0f - (t01 - peakAt) / (1.0f - peakAt));
        glowAlpha = 0.60f * p;
        glowColor = juce::Colour::fromFloatRGBA(
            1.0f,
            juce::jmap(p, 0.0f, 1.0f, 0.54f, 0.23f),   // amber green-chan 0xFF8A → red 0xFF3A
            juce::jmap(p, 0.0f, 1.0f, 0.23f, 0.30f),   // amber blue 0x3A → red 0x4D
            1.0f);
        yScale = 1.0f + 0.025f * p;
    }

    // Halo behind the segment column. Soft, behind, no scale (only fill scales).
    if (glowAlpha > 0.01f) {
        g.setColour(glowColor.withAlpha(glowAlpha));
        g.fillRoundedRectangle(segArea.expanded(8.0f, 10.0f), 6.0f);
    }

    // Segment column. scaleY pivots from bottom (transform-origin: bottom) so
    // the fill stretches upward; never scaleX (would distort against the frame).
    juce::Graphics::ScopedSaveState save(g);
    if (yScale != 1.0f) {
        g.addTransform(juce::AffineTransform::scale(
            1.0f, yScale, segArea.getCentreX(), segArea.getBottom()));
    }
    for (int i = 0; i < N; ++i) {
        float y = segArea.getY() + i * (segH + gap);
        bool on = (i >= (N - lit));
        // GR meter convention: more reduction = more segments lit downward from top.
        // Color grade: top 3 mint, next 4 lime, middle yellow, then orange, then red at bottom (heavy gain reduction).
        juce::Colour c = (i < 3)  ? juce::Colour(0xFF3AFFB0)
                       : (i < 7)  ? juce::Colour(0xFFB9FF3A)
                       : (i < 11) ? juce::Colour(0xFFFFD54A)
                       : (i < 16) ? juce::Colour(0xFFFF8A3A)
                                  : juce::Colour(0xFFFF3A4D);
        g.setColour(on ? c : c.withAlpha(0.10f));
        g.fillRect(segArea.getX(), y, segArea.getWidth(), segH);
    }

    // GR numeric readout at the bottom
    g.setFont(juce::Font(11.0f));
    g.setColour(juce::Colour(0xFFFF8A3A));
    juce::String grStr = (gr > 0.05f) ? "-" + juce::String(gr, 1) : "0.0";
    g.drawText(grStr, m.toNearestInt().removeFromBottom(14), juce::Justification::centred);
}

void LimiterPanel::drawModePicker(juce::Graphics& g, juce::Rectangle<int> area) {
    static const char* modes[3] = { "TRANSPARENT", "PUNCHY", "WARM" };
    static const char* descs[3] = {
        "Minimum coloration · fastest attack",
        "ISP detection · midrange forward",
        "Soft saturation · vintage warmth"
    };
    int activeIdx = 0;
    if (auto* p = dynamic_cast<juce::AudioParameterChoice*>(processor.getAPVTS().getParameter("limStyle")))
        activeIdx = p->getIndex();

    const int cellH = (area.getHeight() - 8) / 3;
    for (int i = 0; i < 3; ++i) {
        auto cell = juce::Rectangle<int>(area.getX(), area.getY() + i * (cellH + 4), area.getWidth(), cellH).toFloat();
        bool active = (i == activeIdx);

        // Active cell uses the limiter mint accent at low alpha + accent border.
        const juce::Colour accent = juce::Colour(mst::theme::tabLim);
        g.setColour(active ? accent.withAlpha(0.12f) : juce::Colour(mst::theme::panelInner));
        g.fillRoundedRectangle(cell, 4.0f);
        g.setColour(active ? accent : juce::Colour(mst::theme::border).withAlpha(0.4f));
        g.drawRoundedRectangle(cell, 4.0f, active ? 1.5f : 1.0f);

        // Title (mint when active, mid otherwise)
        g.setFont(juce::Font(10.0f).boldened());
        g.setColour(active ? accent : juce::Colour(mst::theme::textMid));
        g.drawText(modes[i],
                   cell.toNearestInt().reduced(10, 4).removeFromTop(16),
                   juce::Justification::centredLeft);
        // Description (mono, dim)
        g.setFont(juce::Font(8.0f));
        g.setColour(juce::Colour(mst::theme::textLow));
        g.drawText(descs[i],
                   cell.toNearestInt().reduced(10, 4).removeFromBottom(14),
                   juce::Justification::centredLeft);
    }
}

void LimiterPanel::mouseDown(const juce::MouseEvent& e) {
    if (!modeArea.contains(e.getPosition())) return;
    int relY = e.getPosition().y - modeArea.getY();
    const int cellH = (modeArea.getHeight() - 8) / 3;
    int idx = relY / (cellH + 4);
    if (idx < 0 || idx > 2) return;
    if (auto* p = dynamic_cast<juce::AudioParameterChoice*>(processor.getAPVTS().getParameter("limStyle"))) {
        p->setValueNotifyingHost(p->getNormalisableRange().convertTo0to1((float)idx));
    }
    repaint();
}

void LimiterPanel::resized() {
    auto bounds = getLocalBounds();
    bounds.removeFromTop(34);   // header
    bounds.reduce(14, 8);

    // 3-column layout per Design v2 spec: 200px mode picker | 1fr knobs | 90px GR meter
    modeArea  = bounds.removeFromLeft(200);
    bounds.removeFromLeft(14);
    meterArea = bounds.removeFromRight(90);
    bounds.removeFromRight(14);
    auto knobArea = bounds;

    // Center 2×2 knob grid (THRESH/CEILING / RELEASE/MAKEUP)
    int colW = knobArea.getWidth() / 2;
    int rowH = knobArea.getHeight() / 2;
    auto top = knobArea.removeFromTop(rowH);
    auto bot = knobArea;
    threshK ->setBounds(top.removeFromLeft(colW).reduced(14, 8));
    ceilingK->setBounds(top.reduced(14, 8));
    releaseK->setBounds(bot.removeFromLeft(colW).reduced(14, 8));
    makeupK ->setBounds(bot.reduced(14, 8));
}

void LimiterPanel::refresh() {
    tickGrAnimation();
    repaint();
}

void LimiterPanel::tickGrAnimation() {
    // v1.0.2 §2 state machine. Read smoothed GR off the limiter, fire pulse
    // on rising-edge / engage throb on sustained reduction. Thresholds and
    // durations are from "Mastering Sweet v1.0.2 Polish Direction.html"
    // verbatim — don't tune without spec change.
    constexpr float  kThreshold      = 3.0f;     // dB
    constexpr double kPulseDebounce  = 100.0;    // ms (min between pulses)
    constexpr double kPulseDuration  = 240.0;    // ms (single-shot lifetime)
    constexpr double kSustainEngage  = 500.0;    // ms (over threshold to engage throb)
    constexpr double kThrobHysteresis = 250.0;   // ms (below threshold before disengage)

    const float  gr   = std::abs(processor.getLimiter().getCurrentGainReduction());
    const double now  = juce::Time::getMillisecondCounterHiRes();

    // Throb: engage / disengage with hysteresis
    if (gr >= kThreshold) {
        if (sustainStartMs == 0.0) sustainStartMs = now;
        sustainReleaseMs = 0.0;
        if (!isThrobbing && (now - sustainStartMs >= kSustainEngage)) {
            isThrobbing = true;
            throbRefMs  = now;
        }
    } else {
        sustainStartMs = 0.0;
        if (isThrobbing) {
            if (sustainReleaseMs == 0.0) sustainReleaseMs = now;
            if (now - sustainReleaseMs >= kThrobHysteresis) {
                isThrobbing = false;
                sustainReleaseMs = 0.0;
            }
        }
    }

    // Pulse: rising-edge across threshold, debounced. Suppressed while throbbing
    // (per spec — throb IS the sustained-reduction signal; pulse would be noise).
    if (!isThrobbing && prevGr < kThreshold && gr >= kThreshold) {
        if (now - lastPulseMs >= kPulseDebounce) {
            isPulsing   = true;
            pulseStartMs = now;
            lastPulseMs  = now;
        }
    }
    if (isPulsing && (now - pulseStartMs >= kPulseDuration)) {
        isPulsing = false;
    }

    prevGr = gr;
}
