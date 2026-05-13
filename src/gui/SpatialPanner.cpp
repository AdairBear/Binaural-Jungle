#include "SpatialPanner.h"

#include <cmath>

#include "Theme.h"

namespace bjf::gui
{

namespace
{
    // Inverse of the processor's degrees-to-radians conversion. The
    // panner's UI math is happiest in radians so we convert in/out at the
    // edges only.
    constexpr float deg2rad = 0.017453292519943295f;
}

SpatialPanner::SpatialPanner (juce::AudioProcessorValueTreeState& apvts_,
                              const juce::String& azParamID,
                              const juce::String& elParamID,
                              const juce::String& spreadAzParamID,
                              const juce::String& spreadElParamID,
                              const juce::String& voicesParamID)
    : apvts (apvts_)
{
    azParam       = apvts.getParameter (azParamID);
    elParam       = apvts.getParameter (elParamID);
    spreadAzParam = apvts.getParameter (spreadAzParamID);
    spreadElParam = apvts.getParameter (spreadElParamID);
    voicesParam   = apvts.getParameter (voicesParamID);

    startTimerHz (30);
    setOpaque (false);
}

SpatialPanner::~SpatialPanner() { stopTimer(); }

void SpatialPanner::timerCallback()
{
    auto get = [] (juce::RangedAudioParameter* p) -> float
    { return p != nullptr ? p->getValue() : 0.0f; };

    const float az  = get (azParam);
    const float el  = get (elParam);
    const float saz = get (spreadAzParam);
    const float sel = get (spreadElParam);
    const int voicesNow = voicesParam != nullptr
        ? juce::roundToInt (voicesParam->convertFrom0to1 (voicesParam->getValue()))
        : 16;

    if (! juce::approximatelyEqual (az,  cachedAz)
     || ! juce::approximatelyEqual (el,  cachedEl)
     || ! juce::approximatelyEqual (saz, cachedSpreadAz)
     || ! juce::approximatelyEqual (sel, cachedSpreadEl)
     || voicesNow != cachedVoices)
    {
        cachedAz       = az;
        cachedEl       = el;
        cachedSpreadAz = saz;
        cachedSpreadEl = sel;
        cachedVoices   = voicesNow;
        repaint();
    }
}

void SpatialPanner::resized() {}

// Replicates the spread distribution from VoiceManager::setSpatial — slot i
// of N is placed at offset (i + 0.5)/N − 0.5 of the spread range, centred on
// (az, el). Keeping the two in lockstep means the panner is a faithful
// preview, not just a decorative orbit.
void SpatialPanner::computeVoicePositions (std::vector<juce::Point<float>>& outAzEl) const
{
    auto azRad = [this] () -> float
    {
        if (azParam == nullptr) return 0.0f;
        return azParam->convertFrom0to1 (azParam->getValue()) * deg2rad;
    }();
    auto elRad = [this] () -> float
    {
        if (elParam == nullptr) return 0.0f;
        return elParam->convertFrom0to1 (elParam->getValue()) * deg2rad;
    }();
    auto sazRad = [this] () -> float
    {
        if (spreadAzParam == nullptr) return 0.0f;
        return spreadAzParam->convertFrom0to1 (spreadAzParam->getValue()) * deg2rad;
    }();
    auto selRad = [this] () -> float
    {
        if (spreadElParam == nullptr) return 0.0f;
        return spreadElParam->convertFrom0to1 (spreadElParam->getValue()) * deg2rad;
    }();
    const int N = juce::jlimit (1, 16, cachedVoices);

    outAzEl.clear();
    outAzEl.reserve (static_cast<std::size_t> (N));
    for (int i = 0; i < N; ++i)
    {
        const float t = (static_cast<float> (i) + 0.5f) / static_cast<float> (N) - 0.5f;
        outAzEl.push_back ({ azRad + sazRad * t, elRad + selRad * t });
    }
}

void SpatialPanner::paint (juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();

    // ── Background ────────────────────────────────────────────────────────
    g.setColour (theme::panelDeep);
    g.fillRoundedRectangle (bounds, theme::panelCornerRadius);
    g.setColour (theme::border);
    g.drawRoundedRectangle (bounds, theme::panelCornerRadius, 1.0f);

    auto plot = bounds.reduced (28.0f);
    const auto centre = plot.getCentre();

    // ── Concentric ellipses (orbital "rings") ────────────────────────────
    g.setColour (theme::border.withAlpha (0.45f));
    constexpr int rings = 4;
    for (int i = 1; i <= rings; ++i)
    {
        const float t = i / static_cast<float> (rings);
        const auto ring = juce::Rectangle<float> (
            plot.getWidth() * t, plot.getHeight() * t * 0.6f)
            .withCentre (centre);
        g.drawEllipse (ring, 1.0f);
    }

    // ── Cross hairs ──────────────────────────────────────────────────────
    g.setColour (theme::border.withAlpha (0.4f));
    g.drawLine (plot.getX(), centre.y, plot.getRight(), centre.y, 1.0f);
    g.drawLine (centre.x, plot.getY(), centre.x, plot.getBottom(), 1.0f);

    // ── Voice particles ──────────────────────────────────────────────────
    std::vector<juce::Point<float>> positions;
    computeVoicePositions (positions);

    const float orbitRx = plot.getWidth()  * 0.5f - 8.0f;
    const float orbitRy = plot.getHeight() * 0.5f * 0.6f - 8.0f;

    for (std::size_t i = 0; i < positions.size(); ++i)
    {
        const float az = positions[i].x; // radians, 0 = front
        const float el = positions[i].y; // radians, + = up

        // Elevation modulates ellipse height: high el = particles arc up.
        // Mapping is purely visual — the engine treats az/el as the truth.
        const float dx = std::sin (az) * orbitRx;
        const float dy = -std::cos (az) * orbitRy * std::cos (el);

        const auto pt = juce::Point<float> (centre.x + dx, centre.y + dy);
        const float voiceT = static_cast<float> (i) / juce::jmax (1.0f,
                                static_cast<float> (positions.size()) - 1.0f);
        const auto col = theme::indigo.interpolatedWith (theme::phosphor, voiceT);

        g.setColour (col.withAlpha (0.18f));
        g.fillEllipse (pt.x - 9.0f, pt.y - 9.0f, 18.0f, 18.0f);
        g.setColour (col);
        g.fillEllipse (pt.x - 3.5f, pt.y - 3.5f, 7.0f, 7.0f);
    }

    // ── Listener head (centre puck with ear markers) ─────────────────────
    constexpr float headR = 16.0f;
    const auto head = juce::Rectangle<float> (headR * 2.0f, headR * 2.0f)
                          .withCentre (centre);
    g.setColour (theme::panel.brighter (0.05f));
    g.fillEllipse (head);
    g.setColour (theme::polaris);
    g.drawEllipse (head, 1.4f);

    auto drawEar = [&] (float side)
    {
        const auto ear = juce::Rectangle<float> (5.0f, 9.0f)
                             .withCentre ({ centre.x + side * (headR + 1.0f), centre.y });
        g.setColour (theme::polaris);
        g.fillRoundedRectangle (ear, 2.0f);
    };
    drawEar (-1.0f);
    drawEar ( 1.0f);

    // Nose marker — visual orientation cue so the user knows "front" is up.
    juce::Path nose;
    nose.addTriangle (centre.x - 3.0f, centre.y - headR + 2.0f,
                      centre.x + 3.0f, centre.y - headR + 2.0f,
                      centre.x,        centre.y - headR - 5.0f);
    g.setColour (theme::polaris);
    g.fillPath (nose);

    // ── Caption ──────────────────────────────────────────────────────────
    g.setColour (theme::textSecondary);
    g.setFont (juce::Font (juce::FontOptions ("Menlo", 9.5f, juce::Font::plain)));
    g.drawText ("SPATIAL FIELD",
                bounds.toNearestInt().removeFromTop (22).reduced (12, 6),
                juce::Justification::topLeft);
}

void SpatialPanner::mouseDown (const juce::MouseEvent& e) { writeFromMouse (e); }
void SpatialPanner::mouseDrag (const juce::MouseEvent& e) { writeFromMouse (e); }

void SpatialPanner::writeFromMouse (const juce::MouseEvent& e)
{
    if (azParam == nullptr || elParam == nullptr)
        return;

    const auto bounds = getLocalBounds().toFloat().reduced (28.0f);
    if (bounds.isEmpty())
        return;

    const float w = juce::jmax (1.0f, bounds.getWidth());
    const float h = juce::jmax (1.0f, bounds.getHeight());
    const float nx = juce::jlimit (0.0f, 1.0f,
                                   (static_cast<float> (e.x) - bounds.getX()) / w);
    const float ny = juce::jlimit (0.0f, 1.0f,
                                   1.0f - (static_cast<float> (e.y) - bounds.getY()) / h);

    // setValueNotifyingHost expects normalised 0..1 in parameter space.
    azParam->setValueNotifyingHost (nx);
    elParam->setValueNotifyingHost (ny);
    repaint();
}

} // namespace bjf::gui
