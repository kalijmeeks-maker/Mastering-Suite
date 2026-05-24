#include "PluginEditor.h"
#include "../PluginProcessor.h"

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

    eqPanel = std::make_unique<EqPanel>(proc, laf);
    addAndMakeVisible(*eqPanel);

    limPanel = std::make_unique<LimiterPanel>(proc, laf);
    addAndMakeVisible(*limPanel);

    setSize(mst::theme::windowDefaultW, mst::theme::windowDefaultH);
    setResizable(true, true);
    setResizeLimits(mst::theme::windowMinW, mst::theme::windowMinH,
                    mst::theme::windowMaxW, mst::theme::windowMaxH);
    getConstrainer()->setFixedAspectRatio(mst::theme::aspectRatio);

    startTimerHz(30);
}

PluginEditor::~PluginEditor() {
    stopTimer();
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

    // Work in scaled coordinates
    auto scaledrBounds = r.toFloat();
    scaledrBounds = scaledrBounds.reduced((r.getWidth() - mst::theme::windowDefaultW * s) / 2.0f,
                                          (r.getHeight() - mst::theme::windowDefaultH * s) / 2.0f);

    auto scaledR = scaledrBounds.toNearestInt();

    header->setBounds(scaledR.removeFromTop(mst::theme::headerHeight));
    footer->setBounds(scaledR.removeFromBottom(mst::theme::footerHeight));
    scaledR.reduce(mst::theme::panelPadding, mst::theme::panelPadding);

    const int gap = mst::theme::gridGap;
    const int colW = (scaledR.getWidth() - gap) / 2;
    auto row1 = scaledR.removeFromTop(248);
    scaledR.removeFromTop(gap);
    auto row2 = scaledR;

    lufsPanel->setBounds(row1.removeFromLeft(colW));
    row1.removeFromLeft(gap);
    graphPanel->setBounds(row1);

    eqPanel->setBounds(row2.removeFromLeft(colW));
    row2.removeFromLeft(gap);
    limPanel->setBounds(row2);
}

void PluginEditor::timerCallback() {
    lufsPanel->refresh();
    graphPanel->pushSample(-14.2f);  // Dummy sample for now
    limPanel->refresh();
    footer->refresh();
}
