#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_gui_basics/juce_gui_basics.h>

#include <array>
#include <memory>

#include "Controls.h"

namespace bjf { class PresetManager; }

namespace bjf::gui
{

// ── Preset bar ──────────────────────────────────────────────────────────
// The strip wedged into the TopBar between the logo and the master cluster:
// prev / preset name / next / save / init. The name acts as a button that
// pops a categorised browser menu (Factory > …, User > …) so the user can
// jump anywhere without cycling.
class PresetBar final : public juce::Component,
                        private juce::ChangeListener
{
public:
    explicit PresetBar (bjf::PresetManager& presetManager);
    ~PresetBar() override;

    void resized() override;
    void paint (juce::Graphics&) override;

private:
    void changeListenerCallback (juce::ChangeBroadcaster*) override;
    void refreshNameDisplay();
    void openBrowserMenu();
    void promptForSaveName();

    bjf::PresetManager& presets;
    juce::TextButton prevBtn   { "<" };
    juce::TextButton nextBtn   { ">" };
    juce::TextButton nameBtn   { "Init" };
    juce::TextButton saveBtn   { "Save" };
    juce::TextButton initBtn   { "Init" };

    std::unique_ptr<juce::AlertWindow> saveDialog;
};

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
// Logo on the left; preset bar in the middle (browse / save / init); master
// volume / pan / voices / bpm cluster on the right. The master controls are
// bound to APVTS so host automation reaches them too.
class TopBar final : public juce::Component
{
public:
    TopBar (juce::AudioProcessorValueTreeState& apvts,
            bjf::PresetManager& presetManager);
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    Knob volumeKnob, panKnob, voicesKnob, bpmKnob;
    std::array<std::unique_ptr<detail::SliderAttachment>, 4> attachments;
    PresetBar presetBar;
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
