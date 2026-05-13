#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_gui_basics/juce_gui_basics.h>

#include <array>
#include <memory>

#include "Controls.h"

namespace bjf::gui
{

// Every right-side panel follows the same skeleton: header chip-toggle or
// dropdown across the top, a row of knobs below, optionally a custom
// visualisation strip pinned to the bottom. The five concrete classes below
// are thin compositions of the Controls primitives plus param attachments.

namespace detail
{
    using SliderAttachment   = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ComboBoxAttachment = juce::AudioProcessorValueTreeState::ComboBoxAttachment;
}

// ── Oscillator panel (indigo) ────────────────────────────────────────────
class OscillatorPanel final : public juce::Component
{
public:
    explicit OscillatorPanel (juce::AudioProcessorValueTreeState& apvts);
    void resized() override;

private:
    Panel panel;
    ChipToggle waveformChip;
    Knob octaveKnob, detuneKnob, phaseKnob, stackKnob, spreadKnob;
    std::array<std::unique_ptr<detail::SliderAttachment>, 5> attachments;
};

// ── Granular panel (violet) ──────────────────────────────────────────────
class GranularPanel final : public juce::Component
{
public:
    explicit GranularPanel (juce::AudioProcessorValueTreeState& apvts);
    void resized() override;
    void paint (juce::Graphics&) override;

private:
    Panel panel;
    juce::ComboBox sampleBox;
    Knob densityKnob, sizeKnob, pitchKnob, scatterKnob, sprayKnob, mixKnob;
    std::unique_ptr<detail::ComboBoxAttachment> sampleAtt;
    std::array<std::unique_ptr<detail::SliderAttachment>, 6> knobAttachments;
};

// ── Filter panel (cyan) ──────────────────────────────────────────────────
class FilterPanel final : public juce::Component
{
public:
    explicit FilterPanel (juce::AudioProcessorValueTreeState& apvts);
    void resized() override;
    void paint (juce::Graphics&) override;

private:
    Panel panel;
    ChipToggle typeChip;
    Knob cutoffKnob, resoKnob, envAmtKnob, keyTrackKnob, driveKnob;
    std::array<std::unique_ptr<detail::SliderAttachment>, 5> attachments;
};

// ── Envelope panel (cream) ───────────────────────────────────────────────
class EnvelopePanel final : public juce::Component
{
public:
    explicit EnvelopePanel (juce::AudioProcessorValueTreeState& apvts);
    void resized() override;
    void paint (juce::Graphics&) override;

private:
    Panel panel;
    Knob delayKnob, attackKnob, holdKnob, decayKnob, sustainKnob, releaseKnob, curveKnob;
    std::array<std::unique_ptr<detail::SliderAttachment>, 7> attachments;
};

// ── HOA Reverb panel (phosphor) ──────────────────────────────────────────
class ReverbPanel final : public juce::Component
{
public:
    explicit ReverbPanel (juce::AudioProcessorValueTreeState& apvts);
    void resized() override;

private:
    Panel panel;
    Knob roomKnob, decayKnob, dampingKnob, preDelayKnob, diffusionKnob, erMixKnob, wetDryKnob;
    std::array<std::unique_ptr<detail::SliderAttachment>, 7> attachments;
};

// ── Top bar ──────────────────────────────────────────────────────────────
// Logo on the left; master volume / pan knobs, voices spinner, and BPM read
// across the right. All bound to APVTS so host automation reaches them too.
class TopBar final : public juce::Component
{
public:
    explicit TopBar (juce::AudioProcessorValueTreeState& apvts);
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    Knob volumeKnob, panKnob, voicesKnob, bpmKnob;
    std::array<std::unique_ptr<detail::SliderAttachment>, 4> attachments;
};

// ── Bottom bar ───────────────────────────────────────────────────────────
// 4-octave keyboard + master output meter. The keyboard pumps the host MIDI
// keyboard state, which the processor reads in its handleMidi flow.
class BottomBar final : public juce::Component
{
public:
    BottomBar (juce::MidiKeyboardState& keyboardState, int baseOctave);

    void resized() override;
    void paint (juce::Graphics&) override;

    LevelMeter& getMeter() noexcept { return meter; }

private:
    juce::MidiKeyboardComponent keyboard;
    LevelMeter meter;
};

} // namespace bjf::gui
