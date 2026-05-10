#include "CompressorGRMeter.h"

CompressorGRMeter::CompressorGRMeter(MasteringCompressor& comp) : compressor(comp)
{
    startTimer(33);
}

CompressorGRMeter::~CompressorGRMeter()
{
    stopTimer();
}

void CompressorGRMeter::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    
    g.setColour(Theme::Color::toColour(Theme::Color::BG_2));
    g.fillRect(bounds);
    
    int numSegments = 40;
    float segmentWidth = bounds.getWidth() / numSegments;
    int filledSegments = (int)((-currentGainReduction / 20.0f) * numSegments);
    filledSegments = juce::jlimit(0, numSegments, filledSegments);
    
    for (int i = 0; i < numSegments; ++i) {
        float x = bounds.getX() + i * segmentWidth + 1.0f;
        float y = bounds.getY() + 2.0f;
        float w = segmentWidth - 2.0f;
        float h = bounds.getHeight() - 4.0f;
        
        if (i < filledSegments) {
            if (currentGainReduction < 6.0f) {
                g.setColour(Theme::Color::toColour(Theme::Color::AMBER));
            } else if (currentGainReduction < 12.0f) {
                g.setColour(Theme::Color::toColour(Theme::Color::CYAN));
            } else {
                g.setColour(Theme::Color::toColour(Theme::Color::MAGENTA));
            }
        } else {
            g.setColour(Theme::Color::toColour(Theme::Color::BG_3));
        }
        g.fillRect(x, y, w, h);
    }
    
    g.setColour(Theme::Color::toColour(Theme::Color::TEXT_DIM));
    g.setFont(Theme::Font::makeMono(10.0f));
    g.drawText(juce::String::formatted("–%.1f dB", -currentGainReduction), 
               bounds, juce::Justification::centredRight);
}

void CompressorGRMeter::timerCallback()
{
    currentGainReduction = -0.5f;
    repaint();
}
