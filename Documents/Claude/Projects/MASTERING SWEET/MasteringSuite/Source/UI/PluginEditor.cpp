#include "PluginEditor.h"
#include "../PluginProcessor.h"
#include "DynamicsPanel.cpp"
#include "ImagerPanel.cpp"

PluginEditor::PluginEditor(MasteringSuiteProcessor& proc)
    : juce::AudioProcessorEditor(&proc), processor(proc) {

    header = std::make_unique<HeaderBar>(proc);
    addAndMakeVisible(*header);

    footer = std::make_unique<FooterBar>(proc);
    addAndMakeVisible(*footer);

    lufsPanel = std::make_unique<LufsPanel>(proc);
    addAndMakeVisible(*lufsPanel);

    graphPanel = std::make_unique<LoudnessGraph>(proc);
    addAndMakeVisible(*graphPanel);

    // Tab Bar
    tabBar = std::make_unique<TabBarComponent>(proc);
    addAndMakeVisible(*tabBar);
    tabBar->onSelectionChanged = [this](int index) {
        eqPanel->setVisible(index == 0);
        dynPanel->setVisible(index == 1);
        imgPanel->setVisible(index == 2);
        limPanel->setVisible(index == 3);
    };

    // Modules
    eqPanel = std::make_unique<EqPanel>(proc, laf);
    addAndMakeVisible(*eqPanel);

    dynPanel = std::make_unique<DynamicsPanel>(proc, laf);
    addAndMakeVisible(*dynPanel);

    imgPanel = std::make_unique<ImagerPanel>(proc, laf);
    addAndMakeVisible(*imgPanel);

    limPanel = std::make_unique<LimiterPanel>(proc, laf);
    addAndMakeVisible(*limPanel);

    // Initial state
    eqPanel->setVisible(true);
    dynPanel->setVisible(false);
    imgPanel->setVisible(false);
    limPanel->setVisible(false);
    tabBar->setSelectedIndex(0); // This now triggers visibility via callback

    setSize(mst::theme::windowDefaultW, mst::theme::windowDefaultH);
    setResizable(true, true);
    setResizeLimits(mst::theme::windowMinW, mst::theme::windowMinH,
                    mst::theme::windowMaxW, mst::theme::windowMaxH);
    getConstrainer()->setFixedAspectRatio(mst::theme::aspectRatio);

    // Register Slider::Listener on every rotary knob so DAW automation also lights the glow.
    attachKnobListeners(this);

    // Keyboard tab shortcuts: 1=EQ, 2=DYN, 3=IMG, 4=LIM
    setWantsKeyboardFocus(true);
    addKeyListener(this);

    // Floating drag-value bubble overlay (hidden until a drag begins).
    addChildComponent(valueBubble);
    valueBubble.setInterceptsMouseClicks(false, false);

    // v1.0.2 §3 — listen to mouse events on EVERY descendant component so the
    // footer toast can light up on any Slider drag (and so EqCurveDisplay can
    // pipe its handle drags through the same setToast pipeline). Sliders still
    // handle their own events first; we only observe.
    addMouseListener(this, true);

    startTimerHz(30);
}

// Translate the slider's accent color (set per-knob via rotarySliderFillColourId)
// into the module token shown in the toast: EQ / DYN / IMG / LIM. Any other
// color returns empty — the toast then degrades to "PARAM · value" form rather
// than failing.
static juce::String moduleForAccent(juce::Colour c) {
    const auto argb = c.getARGB();
    if (argb == juce::Colour(mst::theme::tabEq).getARGB())  return "EQ";
    if (argb == juce::Colour(mst::theme::tabDyn).getARGB()) return "DYN";
    if (argb == juce::Colour(mst::theme::tabImg).getARGB()) return "IMG";
    if (argb == juce::Colour(mst::theme::tabLim).getARGB()) return "LIM";
    return {};
}

// Build the toast string from a slider. Reads:
//   getTooltip()        — set to the APVTS parameter's friendly name in
//                         each panel ctor (param->getName(64)).
//   getTextFromValue()  — JUCE's formatted value text (honors the slider's
//                         value-to-text decorator if installed by the param).
//   getTextValueSuffix() — unit suffix if attached.
static juce::String formatSliderToast(juce::Slider* s) {
    if (s == nullptr) return {};
    const auto accent = s->findColour(juce::Slider::rotarySliderFillColourId);
    const auto mod    = moduleForAccent(accent);
    auto label = s->getTooltip();
    if (label.isEmpty()) label = "PARAM";
    label = label.toUpperCase();
    juce::String value = s->getTextFromValue(s->getValue());
    auto suffix = s->getTextValueSuffix().trim();

    juce::String out;
    if (mod.isNotEmpty()) out += mod + " ";
    out += label + " · " + value;
    if (suffix.isNotEmpty()) out += " " + suffix;
    return out;
}

void PluginEditor::mouseDown(const juce::MouseEvent& e) {
    if (footer == nullptr) return;
    if (auto* s = dynamic_cast<juce::Slider*>(e.eventComponent)) {
        toastDragSrc      = s;
        lastToastUpdateMs = juce::Time::getMillisecondCounterHiRes();
        footer->setToast(formatSliderToast(s),
                         s->findColour(juce::Slider::rotarySliderFillColourId));
    }
}

void PluginEditor::mouseDrag(const juce::MouseEvent& e) {
    juce::ignoreUnused(e);
    if (footer == nullptr || toastDragSrc == nullptr) return;
    // 30 Hz throttle per spec — drag events fire on every mouseMove and we
    // don't need to re-format the toast at audio-thread cadence.
    const double now = juce::Time::getMillisecondCounterHiRes();
    if (now - lastToastUpdateMs < 33.0) return;
    lastToastUpdateMs = now;
    footer->setToast(formatSliderToast(toastDragSrc),
                     toastDragSrc->findColour(juce::Slider::rotarySliderFillColourId));
}

void PluginEditor::mouseUp(const juce::MouseEvent& e) {
    juce::ignoreUnused(e);
    if (footer == nullptr || toastDragSrc == nullptr) return;
    toastDragSrc = nullptr;
    footer->clearToast();
}

void PluginEditor::KnobValueBubble::paint(juce::Graphics& g) {
    if (tracked == nullptr) return;
    auto bounds = getLocalBounds().toFloat();
    juce::Colour accent = tracked->findColour(juce::Slider::rotarySliderFillColourId);
    if (accent == juce::Colours::transparentBlack) accent = juce::Colour(mst::theme::tabEq);
    // Dark panelTop background, 6px corner radius, 2px accent border (Design's spec).
    g.setColour(juce::Colour(0xFF0A0A10));
    g.fillRoundedRectangle(bounds, 6.0f);
    g.setColour(accent);
    g.drawRoundedRectangle(bounds.reduced(1.0f), 6.0f, 2.0f);
    // 13pt mono, accent-tinted value + unit suffix.
    g.setColour(accent);
    g.setFont(juce::Font(juce::Font::getDefaultMonospacedFontName(), 13.0f, juce::Font::plain));
    juce::String txt = tracked->getTextFromValue(tracked->getValue());
    g.drawText(txt, getLocalBounds(), juce::Justification::centred);
}

bool PluginEditor::keyPressed(const juce::KeyPress& k, juce::Component*) {
    // v1.0.1-H2: Cmd-Z / Cmd-Shift-Z undo+redo. APVTS already has an UndoManager
    // wired in the processor ctor; SliderAttachment + ButtonAttachment begin/end
    // gestures automatically so knob drags and pill toggles are undoable units.
    if (k.getModifiers().isCommandDown()) {
        if (k.getKeyCode() == 'Z' || k.getKeyCode() == 'z') {
            auto& um = processor.getUndoManager();
            if (k.getModifiers().isShiftDown()) {
                if (um.redo()) processor.postStatusMessage("Redo");
            } else {
                if (um.undo()) processor.postStatusMessage("Undo");
            }
            return true;
        }
    }
    auto ch = k.getTextCharacter();
    if (ch >= '1' && ch <= '4') {
        tabBar->setSelectedIndex(ch - '1');
        return true;
    }
    return false;
}

PluginEditor::~PluginEditor() {
    stopTimer();
    for (auto* s : trackedSliders) {
        if (s != nullptr) s->removeListener(this);
    }
}

void PluginEditor::attachKnobListeners(juce::Component* root) {
    for (auto* child : root->getChildren()) {
        if (auto* slider = dynamic_cast<juce::Slider*>(child)) {
            slider->addListener(this);
            trackedSliders.push_back(slider);
        }
        attachKnobListeners(child);
    }
}

void PluginEditor::sliderValueChanged(juce::Slider* s) {
    if (s != nullptr) {
        s->getProperties().set("lastTouched", juce::Time::getMillisecondCounterHiRes());
    }
}

void PluginEditor::paint(juce::Graphics& g) {
    juce::ColourGradient bg(
        juce::Colour(mst::theme::panelTop), 0.0f, 0.0f,
        juce::Colour(mst::theme::bgBase), 0.0f, (float)getHeight(),
        false);
    g.setGradientFill(bg);
    g.fillAll();

    g.setColour(juce::Colour(mst::theme::border));
    g.drawRect(getLocalBounds(), 1);
}

void PluginEditor::resized() {
    auto r = getLocalBounds();

    // Scale entire content to match window size while preserving aspect ratio
    const float sx = r.getWidth() / (float)mst::theme::windowDefaultW;
    const float sy = r.getHeight() / (float)mst::theme::windowDefaultH;
    const float s = juce::jmin(sx, sy);

    auto scaledrBounds = r.toFloat();
    scaledrBounds = scaledrBounds.reduced((r.getWidth() - mst::theme::windowDefaultW * s) / 2.0f,
                                          (r.getHeight() - mst::theme::windowDefaultH * s) / 2.0f);

    auto scaledR = scaledrBounds.toNearestInt();

    header->setBounds(scaledR.removeFromTop(mst::theme::headerHeight));
    footer->setBounds(scaledR.removeFromBottom(mst::theme::footerHeight));
    
    // Middle Divider (Tabs)
    tabBar->setBounds(scaledR.withHeight(30).withY(scaledR.getY() + 258));

    scaledR.reduce(mst::theme::panelPadding, mst::theme::panelPadding);

    const int gap = mst::theme::gridGap;
    
    // Row 1: Meters (Top Half)
    auto row1 = scaledR.removeFromTop(248);
    const int colW = (row1.getWidth() - gap) / 2;
    
    lufsPanel->setBounds(row1.removeFromLeft(colW));
    row1.removeFromLeft(gap);
    graphPanel->setBounds(row1);

    // Row 2: Active Module (Bottom Half)
    scaledR.removeFromTop(gap + 30); // skip Row 1 + Tab Bar
    auto moduleArea = scaledR;
    
    eqPanel->setBounds(moduleArea);
    dynPanel->setBounds(moduleArea);
    imgPanel->setBounds(moduleArea);
    limPanel->setBounds(moduleArea);
}

// Timer callback
void PluginEditor::timerCallback() {
    lufsPanel->refresh();

    // Pass actual Short-Term LUFS to the graph, or fallback to -70 if not initialized
    float shortTermLufs = processor.getMeter().getShortTermLufs();
    if (shortTermLufs < -70.0f) shortTermLufs = -70.0f;
    graphPanel->pushSample(shortTermLufs);

    // Refresh active panel data
    if (eqPanel->isVisible()) eqPanel->refreshSpectrum();
    if (dynPanel->isVisible()) dynPanel->refresh();
    if (imgPanel->isVisible()) imgPanel->refresh();

    limPanel->refresh();
    footer->refresh();
    header->refresh();   // updates preset modified bullet
    tabBar->repaint();  // keep tab activity indicators live

    // Update knob touch states for all tracked sliders.
    // Hovered or pressed → snap to full glow + refresh the timestamp.
    // Otherwise fade alpha over a 600ms window from the last interaction.
    double currentTime = juce::Time::getMillisecondCounterHiRes();
    juce::Slider* draggingNow = nullptr;
    for (auto* k : trackedSliders) {
        if (k == nullptr) continue;
        bool hot = k->isMouseOver(true) || k->isMouseButtonDown();
        if (hot) {
            k->getProperties().set("lastTouched", currentTime);
        }
        if (k->isMouseButtonDown()) draggingNow = k;
        double last = (double)k->getProperties().getWithDefault("lastTouched", 0.0);
        float alpha = hot ? 1.0f
                          : (float)juce::jlimit(0.0, 1.0, 1.0 - (currentTime - last) / 600.0);
        auto oldAlpha = (float)k->getProperties().getWithDefault("glowAlpha", -1.0f);
        if (std::abs(oldAlpha - alpha) > 0.005f) {
            k->getProperties().set("glowAlpha", alpha);
            k->repaint();
        }
    }

    // Drag-only value bubble per Design's spec — hover doesn't trigger it.
    if (draggingNow != nullptr) {
        lastDragged = draggingNow;
        lastDragTimeMs = currentTime;
        const int bw = 70, bh = 22;
        auto local = getLocalArea(draggingNow, draggingNow->getLocalBounds());
        valueBubble.setBounds(local.getCentreX() - bw / 2, local.getY() - bh - 4, bw, bh);
        valueBubble.tracked = draggingNow;
        if (!valueBubble.isVisible()) valueBubble.setVisible(true);
        valueBubble.toFront(false);
        valueBubble.repaint();
    } else if (valueBubble.isVisible() && (currentTime - lastDragTimeMs) > 200.0) {
        valueBubble.setVisible(false);
        valueBubble.tracked = nullptr;
    }
}
