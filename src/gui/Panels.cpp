#include "Panels.h"

#include <cmath>

#include "../app/ParameterIDs.h"

namespace bjf::gui
{

namespace
{
    constexpr int knobH       = 86;
    constexpr int headerRowH  = 28;
    constexpr int gap         = 8;

    void layoutRow (juce::Rectangle<int> row, std::initializer_list<juce::Component*> items)
    {
        if (items.size() == 0) return;
        const int each = row.getWidth() / static_cast<int> (items.size());
        for (auto* c : items)
            c->setBounds (row.removeFromLeft (each).reduced (2));
    }
}

// ─── OscillatorPanel ────────────────────────────────────────────────────
OscillatorPanel::OscillatorPanel (juce::AudioProcessorValueTreeState& apvts)
    : panel        ("Oscillator", theme::indigo),
      waveformChip (apvts, pid::waveform, theme::indigo),
      octaveKnob   ("Octave",  theme::indigo),
      detuneKnob   ("Detune",  theme::indigo),
      phaseKnob    ("Phase",   theme::indigo),
      stackKnob    ("Stack",   theme::indigo),
      spreadKnob   ("Spread",  theme::indigo)
{
    addAndMakeVisible (panel);
    addAndMakeVisible (waveformChip);
    for (auto* k : { &octaveKnob, &detuneKnob, &phaseKnob, &stackKnob, &spreadKnob })
        addAndMakeVisible (*k);

    attachments[0] = std::make_unique<detail::SliderAttachment> (apvts, pid::oscOctave,   octaveKnob.getSlider());
    attachments[1] = std::make_unique<detail::SliderAttachment> (apvts, pid::detuneCents, detuneKnob.getSlider());
    attachments[2] = std::make_unique<detail::SliderAttachment> (apvts, pid::oscPhase,    phaseKnob.getSlider());
    attachments[3] = std::make_unique<detail::SliderAttachment> (apvts, pid::stackSize,   stackKnob.getSlider());
    attachments[4] = std::make_unique<detail::SliderAttachment> (apvts, pid::oscSpread,   spreadKnob.getSlider());
}

void OscillatorPanel::resized()
{
    panel.setBounds (getLocalBounds());
    auto content = panel.getContentBounds();

    auto chipRow = content.removeFromTop (headerRowH);
    waveformChip.setBounds (chipRow);
    content.removeFromTop (gap);

    auto knobRow = content.removeFromTop (knobH);
    layoutRow (knobRow, { &octaveKnob, &detuneKnob, &phaseKnob, &stackKnob, &spreadKnob });
}

// ─── GranularPanel ──────────────────────────────────────────────────────
GranularPanel::GranularPanel (juce::AudioProcessorValueTreeState& apvts)
    : panel       ("Granular", theme::violet),
      densityKnob ("Density", theme::violet),
      sizeKnob    ("Grain",   theme::violet),
      pitchKnob   ("Pitch",   theme::violet),
      scatterKnob ("Scatter", theme::violet),
      sprayKnob   ("Spray",   theme::violet),
      mixKnob     ("Mix",     theme::violet)
{
    addAndMakeVisible (panel);

    if (auto* choice = dynamic_cast<juce::AudioParameterChoice*> (apvts.getParameter (pid::granSample)))
    {
        sampleBox.addItemList (choice->choices, 1);
        sampleBox.setColour (juce::ComboBox::backgroundColourId, theme::track);
        sampleBox.setColour (juce::ComboBox::textColourId,       theme::textPrimary);
    }
    addAndMakeVisible (sampleBox);

    for (auto* k : { &densityKnob, &sizeKnob, &pitchKnob, &scatterKnob, &sprayKnob, &mixKnob })
        addAndMakeVisible (*k);

    sampleAtt = std::make_unique<detail::ComboBoxAttachment> (apvts, pid::granSample, sampleBox);
    knobAttachments[0] = std::make_unique<detail::SliderAttachment> (apvts, pid::granDensity, densityKnob.getSlider());
    knobAttachments[1] = std::make_unique<detail::SliderAttachment> (apvts, pid::granSizeMs,  sizeKnob.getSlider());
    knobAttachments[2] = std::make_unique<detail::SliderAttachment> (apvts, pid::granPitch,   pitchKnob.getSlider());
    knobAttachments[3] = std::make_unique<detail::SliderAttachment> (apvts, pid::granScatter, scatterKnob.getSlider());
    knobAttachments[4] = std::make_unique<detail::SliderAttachment> (apvts, pid::granSpray,   sprayKnob.getSlider());
    knobAttachments[5] = std::make_unique<detail::SliderAttachment> (apvts, pid::granMix,     mixKnob.getSlider());
}

void GranularPanel::resized()
{
    panel.setBounds (getLocalBounds());
    auto content = panel.getContentBounds();

    auto top = content.removeFromTop (headerRowH);
    sampleBox.setBounds (top);
    content.removeFromTop (gap);

    auto knobRow = content.removeFromTop (knobH);
    layoutRow (knobRow, { &densityKnob, &sizeKnob, &pitchKnob,
                          &scatterKnob, &sprayKnob, &mixKnob });
}

void GranularPanel::paint (juce::Graphics& g)
{
    // Stub waveform strip below the knobs — a deterministic curve so the
    // panel looks like it's monitoring something while the real granular
    // engine isn't wired yet. Replace with a real grain-cloud display once
    // GranularEngine lands.
    auto content = panel.getContentBounds();
    content.removeFromTop (headerRowH + gap + knobH + 4);
    auto strip = content.removeFromTop (40).toFloat();
    if (strip.isEmpty())
        return;

    g.setColour (theme::track);
    g.fillRoundedRectangle (strip, 4.0f);

    juce::Path wave;
    const float w = strip.getWidth();
    const float h = strip.getHeight();
    const int steps = juce::jmax (32, juce::roundToInt (w / 4.0f));
    for (int i = 0; i <= steps; ++i)
    {
        const float t = static_cast<float> (i) / static_cast<float> (steps);
        const float x = strip.getX() + t * w;
        const float amp = 0.6f * std::sin (t * juce::MathConstants<float>::twoPi * 3.0f)
                       + 0.3f * std::sin (t * juce::MathConstants<float>::twoPi * 11.0f + 1.0f);
        const float y = strip.getCentreY() + amp * h * 0.35f;
        if (i == 0) wave.startNewSubPath (x, y);
        else        wave.lineTo (x, y);
    }
    g.setColour (theme::violet.withAlpha (0.85f));
    g.strokePath (wave, juce::PathStrokeType (1.4f));
}

// ─── FilterPanel ────────────────────────────────────────────────────────
FilterPanel::FilterPanel (juce::AudioProcessorValueTreeState& apvts)
    : panel        ("Filter", theme::cyan),
      typeChip     (apvts, pid::filterType, theme::cyan),
      cutoffKnob   ("Cutoff",   theme::cyan),
      resoKnob     ("Reso",     theme::cyan),
      envAmtKnob   ("Env Amt",  theme::cyan),
      keyTrackKnob ("Key Trk",  theme::cyan),
      driveKnob    ("Drive",    theme::cyan)
{
    addAndMakeVisible (panel);
    addAndMakeVisible (typeChip);
    for (auto* k : { &cutoffKnob, &resoKnob, &envAmtKnob, &keyTrackKnob, &driveKnob })
        addAndMakeVisible (*k);

    attachments[0] = std::make_unique<detail::SliderAttachment> (apvts, pid::filterCutoff,    cutoffKnob.getSlider());
    attachments[1] = std::make_unique<detail::SliderAttachment> (apvts, pid::filterReso,      resoKnob.getSlider());
    attachments[2] = std::make_unique<detail::SliderAttachment> (apvts, pid::filterEnvAmount, envAmtKnob.getSlider());
    attachments[3] = std::make_unique<detail::SliderAttachment> (apvts, pid::filterKeyTrack,  keyTrackKnob.getSlider());
    attachments[4] = std::make_unique<detail::SliderAttachment> (apvts, pid::filterDrive,     driveKnob.getSlider());
}

void FilterPanel::resized()
{
    panel.setBounds (getLocalBounds());
    auto content = panel.getContentBounds();

    auto chipRow = content.removeFromTop (headerRowH);
    typeChip.setBounds (chipRow);
    content.removeFromTop (gap);

    auto knobRow = content.removeFromTop (knobH);
    layoutRow (knobRow, { &cutoffKnob, &resoKnob, &envAmtKnob, &keyTrackKnob, &driveKnob });
}

void FilterPanel::paint (juce::Graphics& g)
{
    // Frequency response curve placeholder — a smooth log-spaced low-pass
    // shape parametrised by the current cutoff slider, so the strip moves
    // with the knob even before the audio engine consumes the new params.
    auto content = panel.getContentBounds();
    content.removeFromTop (headerRowH + gap + knobH + 4);
    auto strip = content.removeFromTop (44).toFloat();
    if (strip.isEmpty())
        return;

    g.setColour (theme::track);
    g.fillRoundedRectangle (strip, 4.0f);

    const float cutoffNorm = cutoffKnob.getSlider().valueToProportionOfLength (
        cutoffKnob.getSlider().getValue());

    juce::Path curve;
    const int steps = juce::jmax (40, juce::roundToInt (strip.getWidth() / 4.0f));
    for (int i = 0; i <= steps; ++i)
    {
        const float t = static_cast<float> (i) / static_cast<float> (steps);
        const float x = strip.getX() + t * strip.getWidth();
        const float distance = (t - cutoffNorm) * 6.0f;
        const float resp = 1.0f / (1.0f + std::exp (distance));
        const float y = strip.getBottom() - resp * strip.getHeight() * 0.92f
                       - strip.getHeight() * 0.04f;
        if (i == 0) curve.startNewSubPath (x, y);
        else        curve.lineTo (x, y);
    }
    g.setColour (theme::cyan);
    g.strokePath (curve, juce::PathStrokeType (1.6f));
}

// ─── EnvelopePanel ──────────────────────────────────────────────────────
EnvelopePanel::EnvelopePanel (juce::AudioProcessorValueTreeState& apvts)
    : panel       ("Envelope", theme::cream),
      delayKnob   ("Delay",   theme::cream),
      attackKnob  ("Attack",  theme::cream),
      holdKnob    ("Hold",    theme::cream),
      decayKnob   ("Decay",   theme::cream),
      sustainKnob ("Sustain", theme::cream),
      releaseKnob ("Release", theme::cream),
      curveKnob   ("Curve",   theme::cream)
{
    addAndMakeVisible (panel);
    for (auto* k : { &delayKnob, &attackKnob, &holdKnob, &decayKnob,
                     &sustainKnob, &releaseKnob, &curveKnob })
        addAndMakeVisible (*k);

    attachments[0] = std::make_unique<detail::SliderAttachment> (apvts, pid::envDelay, delayKnob.getSlider());
    attachments[1] = std::make_unique<detail::SliderAttachment> (apvts, pid::attack,   attackKnob.getSlider());
    attachments[2] = std::make_unique<detail::SliderAttachment> (apvts, pid::envHold,  holdKnob.getSlider());
    attachments[3] = std::make_unique<detail::SliderAttachment> (apvts, pid::decay,    decayKnob.getSlider());
    attachments[4] = std::make_unique<detail::SliderAttachment> (apvts, pid::sustain,  sustainKnob.getSlider());
    attachments[5] = std::make_unique<detail::SliderAttachment> (apvts, pid::release,  releaseKnob.getSlider());
    attachments[6] = std::make_unique<detail::SliderAttachment> (apvts, pid::envCurve, curveKnob.getSlider());
}

void EnvelopePanel::resized()
{
    panel.setBounds (getLocalBounds());
    auto content = panel.getContentBounds();
    auto knobRow = content.removeFromTop (knobH);
    layoutRow (knobRow, { &delayKnob, &attackKnob, &holdKnob, &decayKnob,
                          &sustainKnob, &releaseKnob, &curveKnob });
}

void EnvelopePanel::paint (juce::Graphics& g)
{
    // DAHDSR profile preview. Each stage gets a proportional horizontal slot
    // based on its slider value; sustain holds a horizontal segment at the
    // sustain level. Curve slider shapes the attack curve from exp to log.
    auto content = panel.getContentBounds();
    content.removeFromTop (knobH + 6);
    auto strip = content.removeFromTop (60).toFloat();
    if (strip.isEmpty())
        return;

    g.setColour (theme::track);
    g.fillRoundedRectangle (strip, 4.0f);

    auto norm = [] (const Knob& k)
    {
        return static_cast<float> (k.getSlider().valueToProportionOfLength (
            k.getSlider().getValue()));
    };

    const float d = norm (delayKnob);
    const float a = norm (attackKnob);
    const float h = norm (holdKnob);
    const float dc = norm (decayKnob);
    const float s = juce::jlimit (0.0f, 1.0f,
        static_cast<float> (sustainKnob.getSlider().getValue()));
    const float r = norm (releaseKnob);
    const float curve = norm (curveKnob);

    const float totalT = juce::jmax (0.01f, d + a + h + dc + 0.3f + r);
    const auto W = strip.getWidth();
    const auto H = strip.getHeight() - 8.0f;
    const auto baseY = strip.getBottom() - 4.0f;

    auto xAt = [&] (float t) { return strip.getX() + (t / totalT) * W; };
    auto yAt = [&] (float v) { return baseY - v * H; };

    juce::Path env;
    env.startNewSubPath (xAt (0.0f), baseY);
    env.lineTo (xAt (d), baseY);

    // Attack with curve bias — exponential for low curve, logarithmic for high.
    const int attackSteps = 16;
    for (int i = 1; i <= attackSteps; ++i)
    {
        const float t = static_cast<float> (i) / attackSteps;
        const float shaped = std::pow (t, 1.0f + (0.5f - curve) * 3.0f);
        env.lineTo (xAt (d + a * t), yAt (shaped));
    }
    env.lineTo (xAt (d + a + h), yAt (1.0f));

    const int decaySteps = 12;
    for (int i = 1; i <= decaySteps; ++i)
    {
        const float t = static_cast<float> (i) / decaySteps;
        const float v = 1.0f - (1.0f - s) * t;
        env.lineTo (xAt (d + a + h + dc * t), yAt (v));
    }
    env.lineTo (xAt (d + a + h + dc + 0.3f), yAt (s));

    const int releaseSteps = 12;
    for (int i = 1; i <= releaseSteps; ++i)
    {
        const float t = static_cast<float> (i) / releaseSteps;
        env.lineTo (xAt (d + a + h + dc + 0.3f + r * t), yAt (s * (1.0f - t)));
    }

    g.setColour (theme::cream);
    g.strokePath (env, juce::PathStrokeType (1.6f, juce::PathStrokeType::curved,
                                                    juce::PathStrokeType::rounded));
}

// ─── ReverbPanel ────────────────────────────────────────────────────────
ReverbPanel::ReverbPanel (juce::AudioProcessorValueTreeState& apvts)
    : panel         ("HOA Reverb", theme::phosphor),
      roomKnob      ("Room",      theme::phosphor),
      decayKnob     ("Decay",     theme::phosphor),
      dampingKnob   ("Damping",   theme::phosphor),
      preDelayKnob  ("Pre-Dly",   theme::phosphor),
      diffusionKnob ("Diffusion", theme::phosphor),
      erMixKnob     ("ER Mix",    theme::phosphor),
      wetDryKnob    ("Wet/Dry",   theme::phosphor)
{
    addAndMakeVisible (panel);
    for (auto* k : { &roomKnob, &decayKnob, &dampingKnob, &preDelayKnob,
                     &diffusionKnob, &erMixKnob, &wetDryKnob })
        addAndMakeVisible (*k);

    attachments[0] = std::make_unique<detail::SliderAttachment> (apvts, pid::revRoomSize,  roomKnob.getSlider());
    attachments[1] = std::make_unique<detail::SliderAttachment> (apvts, pid::revDecay,     decayKnob.getSlider());
    attachments[2] = std::make_unique<detail::SliderAttachment> (apvts, pid::revDamping,   dampingKnob.getSlider());
    attachments[3] = std::make_unique<detail::SliderAttachment> (apvts, pid::revPreDelay,  preDelayKnob.getSlider());
    attachments[4] = std::make_unique<detail::SliderAttachment> (apvts, pid::revDiffusion, diffusionKnob.getSlider());
    attachments[5] = std::make_unique<detail::SliderAttachment> (apvts, pid::erMix,        erMixKnob.getSlider());
    attachments[6] = std::make_unique<detail::SliderAttachment> (apvts, pid::revWetDry,    wetDryKnob.getSlider());
}

void ReverbPanel::resized()
{
    panel.setBounds (getLocalBounds());
    auto content = panel.getContentBounds();
    auto knobRow = content.removeFromTop (knobH);
    layoutRow (knobRow, { &roomKnob, &decayKnob, &dampingKnob, &preDelayKnob,
                          &diffusionKnob, &erMixKnob, &wetDryKnob });
}

// ─── TopBar ─────────────────────────────────────────────────────────────
TopBar::TopBar (juce::AudioProcessorValueTreeState& apvts)
    : volumeKnob ("Volume", theme::polaris),
      panKnob    ("Pan",    theme::polaris),
      voicesKnob ("Voices", theme::polaris),
      bpmKnob    ("BPM",    theme::polaris)
{
    addAndMakeVisible (volumeKnob);
    addAndMakeVisible (panKnob);
    addAndMakeVisible (voicesKnob);
    addAndMakeVisible (bpmKnob);

    attachments[0] = std::make_unique<detail::SliderAttachment> (apvts, pid::gain,   volumeKnob.getSlider());
    attachments[1] = std::make_unique<detail::SliderAttachment> (apvts, pid::pan,    panKnob.getSlider());
    attachments[2] = std::make_unique<detail::SliderAttachment> (apvts, pid::voices, voicesKnob.getSlider());
    attachments[3] = std::make_unique<detail::SliderAttachment> (apvts, pid::bpm,    bpmKnob.getSlider());
}

void TopBar::paint (juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    g.setColour (theme::panel);
    g.fillRoundedRectangle (bounds, theme::panelCornerRadius);
    g.setColour (theme::border);
    g.drawRoundedRectangle (bounds, theme::panelCornerRadius, 1.0f);

    // Logo block
    g.setColour (theme::textPrimary);
    g.setFont (juce::Font (juce::FontOptions ("Menlo", 16.0f, juce::Font::bold)));
    g.drawText ("BINAURAL JUNGLE FORGE",
                bounds.toNearestInt().reduced (20, 0),
                juce::Justification::centredLeft);

    g.setColour (theme::indigo);
    const auto accentLine = juce::Rectangle<float> (20.0f, bounds.getBottom() - 14.0f, 220.0f, 1.5f);
    g.fillRect (accentLine);
}

void TopBar::resized()
{
    auto bounds = getLocalBounds().reduced (12, 8);

    // Right-aligned cluster: volume, pan, voices, bpm.
    const int knobW = 78;
    const int knobsTotalW = knobW * 4 + 8 * 3;
    auto knobs = bounds.removeFromRight (knobsTotalW);
    auto place = [&] (Knob& k)
    {
        k.setBounds (knobs.removeFromLeft (knobW));
        knobs.removeFromLeft (8);
    };
    place (volumeKnob);
    place (panKnob);
    place (voicesKnob);
    place (bpmKnob);
}

// ─── BottomBar ──────────────────────────────────────────────────────────
BottomBar::BottomBar (juce::MidiKeyboardState& keyboardState, int baseOctave)
    : keyboard (keyboardState, juce::MidiKeyboardComponent::horizontalKeyboard),
      meter (2)
{
    keyboard.setKeyWidth (16.0f);
    keyboard.setAvailableRange (24, 24 + 4 * 12); // 4 octaves starting at C1
    keyboard.setLowestVisibleKey (24 + baseOctave * 12);
    keyboard.setColour (juce::MidiKeyboardComponent::whiteNoteColourId, theme::panel.brighter (0.4f));
    keyboard.setColour (juce::MidiKeyboardComponent::keySeparatorLineColourId, theme::border);
    keyboard.setColour (juce::MidiKeyboardComponent::keyDownOverlayColourId, theme::indigo.withAlpha (0.6f));
    keyboard.setColour (juce::MidiKeyboardComponent::mouseOverKeyOverlayColourId, theme::indigo.withAlpha (0.25f));
    keyboard.setColour (juce::MidiKeyboardComponent::textLabelColourId, theme::textSecondary);
    addAndMakeVisible (keyboard);
    addAndMakeVisible (meter);
}

void BottomBar::paint (juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    g.setColour (theme::panel);
    g.fillRoundedRectangle (bounds, theme::panelCornerRadius);
    g.setColour (theme::border);
    g.drawRoundedRectangle (bounds, theme::panelCornerRadius, 1.0f);
}

void BottomBar::resized()
{
    auto bounds = getLocalBounds().reduced (10, 10);
    const int meterW = 64;
    auto meterArea = bounds.removeFromRight (meterW);
    bounds.removeFromRight (10);
    keyboard.setBounds (bounds);
    meter.setBounds (meterArea);
}

} // namespace bjf::gui
