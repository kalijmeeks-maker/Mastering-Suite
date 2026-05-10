#include "FooterBar.h"

FooterBar::FooterBar()
{
}

void FooterBar::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    
    g.setColour(Theme::Color::toColour(Theme::Color::BG_0));
    g.fillRect(bounds);
    g.setColour(Theme::Color::toColour(Theme::Color::LINE));
    g.drawHorizontalLine(0, bounds.getX(), bounds.getRight());
    
    const auto indicatorSize = 8.0f;
    g.setColour(Theme::Color::toColour(Theme::Color::ACCENT));
    g.fillEllipse(12.0f, bounds.getCentreY() - indicatorSize / 2.0f, indicatorSize, indicatorSize);
    
    g.setColour(Theme::Color::toColour(Theme::Color::TEXT_DIM));
    g.setFont(Theme::Font::make(9.0f, Theme::Font::WEIGHT_BOLD));
    g.drawText("ACTIVE", juce::Rectangle<float>(24, 0, 80, bounds.getHeight()),
               juce::Justification::centredLeft);
    
    g.setColour(Theme::Color::toColour(Theme::Color::TEXT_DIM));
    g.setFont(Theme::Font::makeMono(9.0f));
    g.drawText(statusText, juce::Rectangle<float>(320, 0, 400, bounds.getHeight()),
               juce::Justification::centred);
    
    const auto barWidth = 40.0f;
    const auto barHeight = 16.0f;
    const auto barX = bounds.getRight() - 200.0f;
    const auto barY = bounds.getCentreY() - barHeight / 2.0f;
    
    g.setColour(Theme::Color::toColour(Theme::Color::BG_2));
    g.drawRect(juce::Rectangle<float>(barX, barY, barWidth, barHeight), 1.0f);
    
    const auto segmentWidth = barWidth / 5.0f;
    const auto filledSegments = (int)((cpuPercent / 100.0f) * 5.0f);
    
    for (int i = 0; i < 5; ++i) {
        if (i < filledSegments) {
            if (cpuPercent < 50.0f)
                g.setColour(Theme::Color::toColour(Theme::Color::CYAN));
            else if (cpuPercent < 80.0f)
                g.setColour(Theme::Color::toColour(Theme::Color::AMBER));
            else
                g.setColour(Theme::Color::toColour(Theme::Color::DANGER));
        } else {
            g.setColour(Theme::Color::toColour(Theme::Color::BG_3));
        }
        
        g.fillRect(barX + i * segmentWidth + 1, barY + 1, segmentWidth - 2, barHeight - 2);
    }
    
    g.setColour(Theme::Color::toColour(Theme::Color::TEXT_DIM));
    g.setFont(Theme::Font::makeMono(9.0f));
    g.drawText(juce::String::formatted("%.1f%%", cpuPercent),
               juce::Rectangle<float>(barX + barWidth + 8, barY, 50, barHeight),
               juce::Justification::centredLeft);
}

void FooterBar::setStatusText(const juce::String& status)
{
    statusText = status;
    repaint();
}

void FooterBar::setCpuUsage(float percent)
{
    cpuPercent = juce::jlimit(0.0f, 100.0f, percent);
    repaint();
}
