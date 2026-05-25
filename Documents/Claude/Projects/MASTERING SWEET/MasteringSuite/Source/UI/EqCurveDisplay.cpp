#include "EqCurveDisplay.h"
#include "../PluginProcessor.h"
#include "EqBandCell.h"

EqCurveDisplay::EqCurveDisplay(MasteringSuiteProcessor& proc) : processor(proc) {
    auto& apvts = processor.getAPVTS();
    const char* bandNames[] = {"eq0", "eq1", "eq2", "eq3", "eq4", "eq5"};
    
    for (int i = 0; i < 6; ++i) {
        apvts.addParameterListener(juce::String(bandNames[i]) + "Type", this);
        apvts.addParameterListener(juce::String(bandNames[i]) + "Gain", this);
        apvts.addParameterListener(juce::String(bandNames[i]) + "Freq", this);
        apvts.addParameterListener(juce::String(bandNames[i]) + "Q", this);
        
        juce::Colour c = (i == 0) ? juce::Colour(0xFFFF00FF) : 
                        (i == 1) ? juce::Colour(0xFF00D9FF) :
                        (i == 2) ? juce::Colour(0xFF00FF41) :
                        (i == 3) ? juce::Colour(0xFFFF9500) :
                        (i == 4) ? juce::Colour(0xFFBB00FF) :
                        juce::Colour(0xFF00FFCC);
        
        handles.push_back({i, {0,0}, c, false});
    }
    
    spectrumBuffer.resize(96, -100.0f);
    peakHoldDb.assign(96, -100.0f);
    peakHoldTimeMs.assign(96, 0.0);
}

EqCurveDisplay::~EqCurveDisplay() {
    const char* bandNames[] = {"eq0", "eq1", "eq2", "eq3", "eq4", "eq5"};
    for (int i = 0; i < 6; ++i) {
        processor.getAPVTS().removeParameterListener(juce::String(bandNames[i]) + "Type", this);
        processor.getAPVTS().removeParameterListener(juce::String(bandNames[i]) + "Gain", this);
        processor.getAPVTS().removeParameterListener(juce::String(bandNames[i]) + "Freq", this);
        processor.getAPVTS().removeParameterListener(juce::String(bandNames[i]) + "Q", this);
    }
}

void EqCurveDisplay::paint(juce::Graphics& g) {
    auto bounds = getLocalBounds().toFloat();
    
    // Background grid
    g.setColour(juce::Colour(mst::theme::panelInner));
    g.fillRoundedRectangle(bounds, 8.0f);
    
    g.setColour(juce::Colour(mst::theme::border).withAlpha(0.3f));
    for (float f = 100; f < 20000; f *= (f < 1000 ? 10 : 2)) {
        float x = freqToX(f);
        g.drawVerticalLine((int)x, 0, bounds.getHeight());
    }
    
    for (float db = -12; db <= 12; db += 6) {
        float y = gainToY(db);
        g.drawHorizontalLine((int)y, 0, bounds.getWidth());
    }

    // Spectrum — vertical bars (FabFilter / v2 convention) with peak-hold dots.
    g.saveState();
    g.reduceClipRegion(bounds.toNearestInt());

    const int N = (int)spectrumBuffer.size();
    const double nowMs = juce::Time::getMillisecondCounterHiRes();
    if (N > 0) {
        const float gap = 1.0f;
        const float binW = bounds.getWidth() / (float)N;
        const float topRangeDb = 6.0f, botRangeDb = -60.0f;
        const float minBarH = 2.0f;
        const float canvasH = bounds.getHeight() * 0.55f;

        auto dbToY = [&](float db) {
            db = juce::jlimit(botRangeDb, topRangeDb, db);
            float norm = (db - botRangeDb) / (topRangeDb - botRangeDb);
            return bounds.getBottom() - juce::jmax(minBarH, norm * canvasH);
        };

        // Draw bars
        for (int i = 0; i < N; ++i) {
            float db = spectrumBuffer[i];
            if (!std::isfinite(db)) continue;
            float y = dbToY(db);
            float x = bounds.getX() + i * binW;
            float h = bounds.getBottom() - y;
            juce::ColourGradient barGrad(
                juce::Colour(0xFFFF5CD1).withAlpha(0.85f), x, y,
                juce::Colour(0xFF4A2080).withAlpha(0.20f), x, bounds.getBottom(),
                false);
            barGrad.addColour(0.40f, juce::Colour(0xFF9B5ADC).withAlpha(0.55f));
            g.setGradientFill(barGrad);
            g.fillRect(x, y, juce::jmax(1.0f, binW - gap), h);

            // Peak-hold tracker: bump dot up to new peak, otherwise decay after 200ms hold.
            if ((int)peakHoldDb.size() <= i) { peakHoldDb.push_back(db); peakHoldTimeMs.push_back(nowMs); continue; }
            if (db > peakHoldDb[i]) {
                peakHoldDb[i]     = db;
                peakHoldTimeMs[i] = nowMs;
            } else {
                double age = nowMs - peakHoldTimeMs[i];
                if (age > 200.0) {
                    // Decay 80 dB over 1500 ms after the 200 ms hold (per Design spec)
                    float drop = (float)((age - 200.0) / 1500.0) * 80.0f;
                    peakHoldDb[i]     = juce::jmax(db, peakHoldDb[i] - drop);
                    peakHoldTimeMs[i] = nowMs;
                }
            }
        }

        // 2×2 white peak dots above the bars
        g.setColour(juce::Colours::white.withAlpha(0.8f));
        for (int i = 0; i < N && i < (int)peakHoldDb.size(); ++i) {
            if (peakHoldDb[i] < -55.0f) continue;
            float py = dbToY(peakHoldDb[i]);
            float x  = bounds.getX() + i * binW + juce::jmax(0.0f, (binW - gap) * 0.5f) - 1.0f;
            g.fillRect(x, py - 1.0f, 2.0f, 2.0f);
        }
    }
    g.restoreState();

    // EQ Curve
    if (pathDirty) updatePath();
    
    g.setColour(juce::Colour(mst::theme::tabEq));
    g.strokePath(curvePath, juce::PathStrokeType(2.5f));
    
    // Fill under curve
    auto fillPath = curvePath;
    fillPath.lineTo(bounds.getRight(), bounds.getBottom());
    fillPath.lineTo(bounds.getX(), bounds.getBottom());
    fillPath.closeSubPath();
    g.setGradientFill(juce::ColourGradient(juce::Colour(mst::theme::tabEq).withAlpha(0.2f), 0, bounds.getY(),
                                           juce::Colour(mst::theme::tabEq).withAlpha(0.0f), 0, bounds.getBottom(), false));
    g.fillPath(fillPath);

    // Handles
    for (auto& h : handles) {
        bool isBypassed = *processor.getAPVTS().getRawParameterValue("eq" + juce::String(h.index) + "Type") == 0;
        bool isHovered  = (h.index == hoveredHandleIndex);
        bool isDragging = (h.index == draggingHandleIndex);

        if (!isBypassed) {
            // Neon Glow
            g.setColour(h.color.withAlpha(0.2f));
            g.fillEllipse(h.pos.x - 14, h.pos.y - 14, 28, 28);
            g.setColour(h.color.withAlpha(0.4f));
            g.fillEllipse(h.pos.x - 11, h.pos.y - 11, 22, 22);
        }

        g.setColour(isBypassed ? juce::Colour(mst::theme::textLow) : h.color);
        g.fillEllipse(h.pos.x - 8, h.pos.y - 8, 16, 16);

        g.setColour(juce::Colour(mst::theme::bgBase));
        g.setFont(juce::Font(10.0f).boldened());
        g.drawText(juce::String(h.index + 1), h.pos.x - 8, h.pos.y - 8, 16, 16, juce::Justification::centred);

        // v1.0.1-H3: hover state mirrors the knobs — brighten the outer ring.
        if (isDragging || isHovered) {
            float alpha = isDragging ? 0.9f : 0.7f;
            float stroke = isDragging ? 2.0f : 1.5f;
            g.setColour(h.color.withAlpha(alpha));
            g.drawEllipse(h.pos.x - 12, h.pos.y - 12, 24, 24, stroke);
        }
    }
}

void EqCurveDisplay::resized() {
    pathDirty = true;
}

void EqCurveDisplay::parameterChanged(const juce::String&, float) {
    pathDirty = true;
    juce::MessageManager::callAsync([this] { repaint(); });
}

void EqCurveDisplay::refreshSpectrum() {
    std::array<float, 2048> fftData;
    processor.getNextFFTBlock(fftData.data());
    
    // Map FFT bins to 96 log-frequency bins for smoother, design-accurate display
    const int numBins = 96;
    std::vector<float> logBins(numBins, -100.0f);
    
    float lowFreq = 20.0f;
    float highFreq = 20000.0f;
    
    for (int i = 0; i < numBins; ++i) {
        float freq = lowFreq * std::pow(highFreq / lowFreq, (float)i / (float)numBins);
        int fftIdx = (int)(freq * (float)processor.fftSize / (float)processor.getSampleRate());
        fftIdx = juce::jlimit(0, (int)processor.fftSize / 2 - 1, fftIdx);
        
        float mag = fftData[fftIdx];
        float db = juce::Decibels::gainToDecibels(mag, -100.0f);
        
        // Add +3dB per octave tilt for more balanced "music-like" spectrum
        float tilt = std::log2(freq / 1000.0f) * 3.0f;
        logBins[i] = db + tilt;
    }

    spectrumBuffer = logBins;
    repaint();
}

void EqCurveDisplay::updatePath() {
    auto bounds = getLocalBounds().toFloat();
    curvePath.clear();
    
    for (int x = 0; x <= (int)bounds.getWidth(); ++x) {
        float freq = xToFreq((float)x);
        float mag = processor.getEQ().getFrequencyResponse(freq);
        float gainDb = juce::Decibels::gainToDecibels(mag);
        float y = gainToY(gainDb);
        
        if (x == 0) curvePath.startNewSubPath((float)x, y);
        else curvePath.lineTo((float)x, y);
    }
    
    // Update handle positions
    for (int i = 0; i < 6; ++i) {
        float freq = *processor.getAPVTS().getRawParameterValue("eq" + juce::String(i) + "Freq");
        float gain = *processor.getAPVTS().getRawParameterValue("eq" + juce::String(i) + "Gain");
        handles[i].pos = { freqToX(freq), gainToY(gain) };
    }
    
    pathDirty = false;
}

float EqCurveDisplay::freqToX(float freq) {
    auto bounds = getLocalBounds().toFloat();
    float logFreq = std::log10(freq / 20.0f) / std::log10(20000.0f / 20.0f);
    return bounds.getX() + logFreq * bounds.getWidth();
}

float EqCurveDisplay::xToFreq(float x) {
    auto bounds = getLocalBounds().toFloat();
    float normalizedX = (x - bounds.getX()) / bounds.getWidth();
    return 20.0f * std::pow(20000.0f / 20.0f, normalizedX);
}

float EqCurveDisplay::gainToY(float gainDb) {
    auto bounds = getLocalBounds().toFloat();
    float normalizedGain = (gainDb + 12.0f) / 24.0f; // range -12 to +12
    return bounds.getBottom() - normalizedGain * bounds.getHeight();
}

float EqCurveDisplay::yToGain(float y) {
    auto bounds = getLocalBounds().toFloat();
    float normalizedY = (bounds.getBottom() - y) / bounds.getHeight();
    return normalizedY * 24.0f - 12.0f;
}

void EqCurveDisplay::mouseDown(const juce::MouseEvent& e) {
    draggingHandleIndex = -1;
    for (int i = 0; i < 6; ++i) {
        if (handles[i].pos.getDistanceSquaredFrom(e.position) < 400.0f) { // 20px radius
            draggingHandleIndex = i;
            handles[i].isDragging = true;
            break;
        }
    }
    repaint();
}

void EqCurveDisplay::mouseDrag(const juce::MouseEvent& e) {
    if (draggingHandleIndex != -1) {
        float freq = juce::jlimit(20.0f, 20000.0f, xToFreq(e.position.x));
        float gain = juce::jlimit(-12.0f, 12.0f, yToGain(e.position.y));

        processor.getAPVTS().getParameter("eq" + juce::String(draggingHandleIndex) + "Freq")->setValueNotifyingHost(
            processor.getAPVTS().getParameterRange("eq" + juce::String(draggingHandleIndex) + "Freq").convertTo0to1(freq));

        processor.getAPVTS().getParameter("eq" + juce::String(draggingHandleIndex) + "Gain")->setValueNotifyingHost(
            processor.getAPVTS().getParameterRange("eq" + juce::String(draggingHandleIndex) + "Gain").convertTo0to1(gain));

        // v1.0.1-H1: APVTS listener -> callAsync repaint can lag visibly during
        // a fast drag. Force adjacent band cells to repaint NOW.
        repaintSiblingCells();
    }
}

void EqCurveDisplay::repaintSiblingCells() {
    if (auto* parent = getParentComponent()) {
        for (auto* child : parent->getChildren()) {
            if (auto* cell = dynamic_cast<EqBandCell*>(child)) cell->repaint();
        }
    }
}

void EqCurveDisplay::mouseMove(const juce::MouseEvent& e) {
    int newHover = -1;
    for (int i = 0; i < 6; ++i) {
        if (handles[i].pos.getDistanceSquaredFrom(e.position) < 400.0f) {  // 20px radius
            newHover = i;
            break;
        }
    }
    if (newHover != hoveredHandleIndex) {
        hoveredHandleIndex = newHover;
        setMouseCursor(hoveredHandleIndex >= 0 ? juce::MouseCursor::PointingHandCursor
                                               : juce::MouseCursor::NormalCursor);
        repaint();
    }
}

void EqCurveDisplay::mouseExit(const juce::MouseEvent&) {
    if (hoveredHandleIndex != -1) {
        hoveredHandleIndex = -1;
        setMouseCursor(juce::MouseCursor::NormalCursor);
        repaint();
    }
}

void EqCurveDisplay::mouseUp(const juce::MouseEvent&) {
    if (draggingHandleIndex != -1) {
        handles[draggingHandleIndex].isDragging = false;
        draggingHandleIndex = -1;
    }
    repaint();
}

void EqCurveDisplay::mouseDoubleClick(const juce::MouseEvent& e) {
    for (int i = 0; i < 6; ++i) {
        if (handles[i].pos.getDistanceSquaredFrom(e.position) < 400.0f) {
            auto* param = processor.getAPVTS().getParameter("eq" + juce::String(i) + "Type");
            float curVal = param->getValue();
            if (curVal == 0.0f) param->setValueNotifyingHost(1.0f); // Simple toggle for now
            else param->setValueNotifyingHost(0.0f);
            break;
        }
    }
}

void EqCurveDisplay::mouseWheelMove(const juce::MouseEvent& e, const juce::MouseWheelDetails& wheel) {
    for (int i = 0; i < 6; ++i) {
        if (handles[i].pos.getDistanceSquaredFrom(e.position) < 1600.0f) { // Larger radius for wheel
            auto& param = *processor.getAPVTS().getParameter("eq" + juce::String(i) + "Q");
            float curQ = param.convertFrom0to1(param.getValue());
            float newQ = juce::jlimit(0.5f, 8.0f, curQ + wheel.deltaY * 0.5f);
            param.setValueNotifyingHost(param.getNormalisableRange().convertTo0to1(newQ));
            break;
        }
    }
}
