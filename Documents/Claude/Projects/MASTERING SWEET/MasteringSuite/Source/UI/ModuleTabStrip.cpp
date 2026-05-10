#include "ModuleTabStrip.h"

ModuleTabStrip::ModuleTabStrip()
{
}

void ModuleTabStrip::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    
    g.setColour(Theme::Color::toColour(Theme::Color::BG_1));
    g.fillRect(bounds);
    g.setColour(Theme::Color::toColour(Theme::Color::LINE));
    g.drawHorizontalLine((int)bounds.getBottom() - 1, bounds.getX(), bounds.getRight());
    
    juce::StringArray tabs{"EQUALIZER", "DYNAMICS", "HARMONICS", "IMAGER", "CLIPPER", "LIMITER"};
    float tabWidth = 120.0f;
    
    for (int i = 0; i < tabs.size(); ++i) {
        auto x = 10.0f + i * (tabWidth + 5.0f);
        g.setColour(i == activeModule ? Theme::Color::toColour(Theme::Color::TEXT_BRIGHT) 
                                      : Theme::Color::toColour(Theme::Color::TEXT_DIM));
        g.setFont(Theme::Font::make(10.0f));
        g.drawText(tabs[i], juce::Rectangle<float>(x, 0, tabWidth, bounds.getHeight()), 
                   juce::Justification::centredLeft);
        
        if (i == activeModule) {
            g.setColour(Theme::Color::toColour(Theme::Color::ACCENT));
            g.drawHorizontalLine((int)bounds.getBottom() - 2, x, x + tabWidth);
        }
    }
    
    g.setColour(Theme::Color::toColour(Theme::Color::TEXT_DIM));
    g.setFont(Theme::Font::make(9.0f));
    auto sampleRateStr = juce::String::formatted("SR %.0fK  BUF %d", sampleRate / 1000.0, bufferSize);
    g.drawText(sampleRateStr, bounds.reduced(10, 0), juce::Justification::centredRight);
}

void ModuleTabStrip::resized()
{
}

void ModuleTabStrip::setActiveModule(int index)
{
    if (index != activeModule) {
        activeModule = index;
        if (onModuleSelected) onModuleSelected(index);
        repaint();
    }
}

void ModuleTabStrip::updateSystemInfo(double sr, int bufSize)
{
    sampleRate = sr;
    bufferSize = bufSize;
    repaint();
}
