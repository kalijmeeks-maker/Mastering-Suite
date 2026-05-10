#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

namespace Theme {
    // ─────────────────────────────────── Colors ─────────────────────────────────
    // Base palette from index.html CSS variables (dark theme)
    namespace Color {
        // Background hierarchy
        constexpr juce::uint32 BG_0          = 0xff07070a;  // Darkest: #07070a
        constexpr juce::uint32 BG_1          = 0xff0d0d12;  // #0d0d12
        constexpr juce::uint32 BG_2          = 0xff13131a;  // #13131a
        constexpr juce::uint32 BG_3          = 0xff1a1a23;  // #1a1a23

        // Borders and dividers
        constexpr juce::uint32 LINE          = 0xff23232d;  // Primary border
        constexpr juce::uint32 LINE_2        = 0xff2c2c38;  // Secondary border

        // Text hierarchy
        constexpr juce::uint32 TEXT_DIM      = 0xff52525c;  // Dimmed/secondary text
        constexpr juce::uint32 TEXT          = 0xffa8a8b4;  // Normal text
        constexpr juce::uint32 TEXT_BRIGHT   = 0xffebebf2;  // Bright/primary text

        // Accent colors (semantic)
        constexpr juce::uint32 ACCENT        = 0xffc4ff3d;  // Neon green (primary, success)
        constexpr juce::uint32 CYAN          = 0xff3fe5ff;  // Bright cyan (info, secondary)
        constexpr juce::uint32 MAGENTA       = 0xffff3d80;  // Bright pink/magenta (warning)
        constexpr juce::uint32 AMBER         = 0xffffb547;  // Orange/amber (caution)
        constexpr juce::uint32 DANGER        = 0xffff4d4d;  // Red (critical)

        inline juce::Colour toColour(juce::uint32 hex) {
            return juce::Colour(hex);
        }
    }

    // ─────────────────────────────────── Typography ──────────────────────────────
    namespace Font {
        static const juce::String FAMILY_INTER = "Inter Tight";      // For UI labels
        static const juce::String FAMILY_MONO  = "JetBrains Mono";   // For numeric values

        // Size presets (points)
        constexpr float SIZE_MICRO    = 9.0f;   // Uppercase micro-labels (e.g., "FREQ", "GAIN")
        constexpr float SIZE_LABEL    = 10.0f;  // Knob labels, field labels
        constexpr float SIZE_NORMAL   = 11.0f;  // Body text, control labels
        constexpr float SIZE_TITLE    = 13.0f;  // Section titles ("EQ", "LIMITER")
        constexpr float SIZE_DISPLAY  = 22.0f;  // Large value displays (LUFS meter)

        // Weight presets (normalized 0.0 to 1.0, will be cast to int)
        constexpr float WEIGHT_NORMAL = 0.0f;   // Regular
        constexpr float WEIGHT_SEMI   = 0.1f;   // Semi-bold
        constexpr float WEIGHT_BOLD   = 0.2f;   // Bold

        inline juce::Font make(float size, float weight = WEIGHT_NORMAL) {
            auto boldStyle = (weight > WEIGHT_NORMAL) ? "Bold" : "Regular";
            return juce::Font(size).withTypefaceStyle(boldStyle);
        }

        inline juce::Font makeMono(float size, float weight = WEIGHT_NORMAL) {
            auto boldStyle = (weight > WEIGHT_NORMAL) ? "Bold" : "Regular";
            return juce::Font(size).withTypefaceStyle(boldStyle);
        }
    }

    // ─────────────────────────────────── Layout ──────────────────────────────────
    namespace Layout {
        // Window dimensions
        constexpr int WINDOW_WIDTH_DEFAULT  = 1000;
        constexpr int WINDOW_HEIGHT_DEFAULT = 700;
        constexpr int WINDOW_WIDTH_MIN      = 880;
        constexpr int WINDOW_HEIGHT_MIN     = 540;
        constexpr int WINDOW_WIDTH_MAX      = 1400;
        constexpr int WINDOW_HEIGHT_MAX     = 900;

        // Fixed aspect ratio: 1000:700 = 1.428...
        constexpr float ASPECT_RATIO = 1000.0f / 700.0f;

        // Grid spacing and margins
        constexpr int MARGIN_DEFAULT        = 14;  // Standard margin around main content
        constexpr int MARGIN_CONTROL_DECK   = 14;  // Left/right padding of control deck
        constexpr int GAP_CONTROL_GROUP     = 24;  // Horizontal gap between knob groups
        constexpr int GAP_VERTICAL          = 8;   // Vertical gap between rows
        constexpr int GAP_SEGMENT           = 2;   // Gap between meter segments

        // Knob sizes
        constexpr int KNOB_SIZE_SMALL       = 42;  // EQ band knobs
        constexpr int KNOB_SIZE_MEDIUM      = 56;  // General purpose knobs
        constexpr int KNOB_SIZE_LARGE       = 68;  // Limiter/featured knobs

        // Component heights (grid rows)
        constexpr int HEADER_HEIGHT         = 44;
        constexpr int MODULE_STRIP_HEIGHT   = 44;
        constexpr int FOOTER_HEIGHT         = 56;
        constexpr int CONTROL_DECK_HEIGHT   = 156;  // Approx 44 * 3.5

        // Border radius (corner softness)
        constexpr float BORDER_RADIUS_PANEL    = 10.0f;   // Main panels, header/footer
        constexpr float BORDER_RADIUS_CONTROL  = 5.0f;    // Knob groups, meter boxes
        constexpr float BORDER_RADIUS_BUTTON   = 4.0f;    // Buttons, small controls
        constexpr float BORDER_RADIUS_SEGMENT  = 1.0f;    // Meter segments (very subtle)

        // Border widths
        constexpr float BORDER_WIDTH_THIN      = 1.0f;
        constexpr float BORDER_WIDTH_MEDIUM    = 1.5f;
    }

    // ─────────────────────────────────── Knob Constants ──────────────────────────
    namespace Knob {
        // Arc rendering parameters
        constexpr float ARC_WIDTH           = 3.0f;    // Outline stroke width
        constexpr float SWEEP_DEGREES       = 270.0f;  // Total arc sweep (NW to SW = 3/4 circle)
        constexpr float START_ANGLE         = 45.0f;   // Start at 45° (NW corner = upper-right)
        constexpr float END_ANGLE           = 315.0f;  // End at 315° (SW corner = lower-right)

        // Visual details
        constexpr float GLOW_SCALE          = 2.5f;    // How many times wider glow is vs arc
        constexpr float INDICATOR_SIZE      = 4.0f;    // Center dot diameter
    }

    // ─────────────────────────────────── Meter Constants ────────────────────────
    namespace Meter {
        // LUFS meter (vertical bar with segments)
        constexpr int LUFS_SEGMENTS            = 13;   // 13 segments for -23 to 0 LUFS (-1.77 dB per segment)
        constexpr float LUFS_SEGMENT_GAP       = 2.0f; // Pixel gap between segments
        constexpr float LUFS_MIN_DB            = -23.0f;
        constexpr float LUFS_MAX_DB            = 3.0f;
        constexpr float LUFS_PEAK_DECAY_DB_PER_SEC = 12.0f; // Peak hold falls at 12 dB/s

        // Loudness graph (ring buffer history)
        constexpr int LOUDNESS_GRAPH_SAMPLES   = 240;  // Ring buffer: ~8 sec at 30 Hz
        constexpr float LOUDNESS_HISTORY_MIN   = -23.0f;
        constexpr float LOUDNESS_HISTORY_MAX   = 7.0f;
        constexpr float LOUDNESS_REFERENCE_DB  = -14.0f; // -14 LUFS reference line

        // GR (gain reduction) meter (vertical bar with segments)
        constexpr int GR_SEGMENTS              = 12;   // 12 segments for 0–40 dB reduction
        constexpr float GR_SEGMENT_GAP         = 2.0f;
        constexpr float GR_MAX_DB              = 40.0f;

        // Level meter (input/output bars)
        constexpr int LEVEL_SEGMENTS           = 20;   // 20 segments per column
        constexpr float LEVEL_SEGMENT_GAP      = 1.0f;

        // Segment colors (thresholds for LUFS meter)
        constexpr float SEGMENT_DANGER_THRESHOLD  = -6.0f;   // Red above -6 LUFS
        constexpr float SEGMENT_AMBER_THRESHOLD   = -12.0f;  // Amber above -12 LUFS
        // Green below -12 LUFS
    }

    // ─────────────────────────────────── Animation Constants ───────────────────
    namespace Animation {
        constexpr int METER_REFRESH_MS        = 33;   // 30 Hz refresh rate
        constexpr float METER_PEAK_HOLD_SEC   = 3.0f; // How long peak hold persists
    }

    // ─────────────────────────────────── Helper Utilities ──────────────────────
    namespace Utils {
        // Clamp a value within [min, max]
        template <typename T>
        inline T clamp(T value, T minimum, T maximum) {
            return juce::jlimit(minimum, maximum, value);
        }

        // Normalize a value from [min, max] to [0, 1]
        inline float normalize(float value, float minimum, float maximum) {
            return (value - minimum) / (maximum - minimum);
        }

        // Interpolate between two values
        inline float lerp(float a, float b, float t) {
            return a + (b - a) * t;
        }

        // Convert dB to linear (0.0 to 1.0 range)
        inline float dbToLinear(float db) {
            return std::pow(10.0f, db / 20.0f);
        }

        // Convert linear to dB
        inline float linearToDb(float linear) {
            return 20.0f * std::log10(std::max(linear, 1e-6f));
        }
    }
}
