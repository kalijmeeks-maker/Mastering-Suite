#pragma once
#include <JuceHeader.h>

namespace mst {
namespace theme {

// ────────── Colors (exact from spec) ──────────
constexpr juce::uint32 bgBase           = 0xFF0A0A10;  // Near-black background
constexpr juce::uint32 panelTop         = 0xFF14141D;  // Top gradient
constexpr juce::uint32 panelInner       = 0xFF0D0D14;  // Inner panel
constexpr juce::uint32 border           = 0xFF23232D;  // Border color

constexpr juce::uint32 textHigh         = 0xFFF3F3FA;  // Bright text (16:1 contrast on bgBase)
constexpr juce::uint32 textMid          = 0xFFA8A8BD;  // Medium text
constexpr juce::uint32 textLow          = 0xFF6A6A7E;  // Dim text

// Accent colors (per knob spec)
constexpr juce::uint32 cEqGain          = 0xFF00D9FF;  // Cyan (EQ Gain)
constexpr juce::uint32 cEqFreq          = 0xFFFF00FF;  // Magenta (EQ Freq)
constexpr juce::uint32 cEqQ             = 0xFF00FF41;  // Lime (EQ Q)
constexpr juce::uint32 cLimThresh       = 0xFFFF9500;  // Orange (Limiter Threshold)
constexpr juce::uint32 cLimRelease      = 0xFFBB00FF;  // Violet (Limiter Release)
constexpr juce::uint32 cLimMakeup       = 0xFF00FFCC;  // Mint (Limiter Makeup)

// Semantic colors
constexpr juce::uint32 statusGreen      = 0xFF00FF41;  // Status indicator
constexpr juce::uint32 peakRed          = 0xFFFF3333;  // Peak/clipping indicator

// Tab accent colors (exact from Claude Design)
constexpr juce::uint32 tabEq            = 0xFF00E5FF;  // Cyan
constexpr juce::uint32 tabDyn           = 0xFFFF7B3A;  // Orange
constexpr juce::uint32 tabImg           = 0xFF9B7BFF;  // Violet
constexpr juce::uint32 tabLim           = 0xFF3AFFB0;  // Mint

// ────────── Layout Constants ──────────
constexpr int headerHeight              = 56;
constexpr int footerHeight              = 32;
constexpr int panelPadding              = 18;
constexpr int gridGap                   = 14;
constexpr int knobSize                  = 56;

// Window constraints
constexpr int windowDefaultW             = 1000;
constexpr int windowDefaultH             = 700;
constexpr int windowMinW                 = 800;
constexpr int windowMinH                 = 560;
constexpr int windowMaxW                 = 1400;
constexpr int windowMaxH                 = 980;
constexpr float aspectRatio              = 1000.0f / 700.0f;

// Meter constants
constexpr int lufsSegments              = 13;
constexpr float lufsSegmentGap          = 2.0f;
constexpr int grSegments                = 12;
constexpr float grSegmentGap            = 2.0f;
constexpr float grPeakDecayPerSec       = 12.0f;

} // namespace theme
} // namespace mst
