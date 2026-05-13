#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>

#include <memory>
#include <vector>

#include "Theme.h"

namespace bjf::gui
{

// ── Panel ─────────────────────────────────────────────────────────────────
// A bordered dark container with an accent bar above the title in the
// panel's accent colour. Pure container — children mount inside getContentBounds().
class Panel final : public juce::Component
{
public:
    Panel (const juce::String& titleText, juce::Colour accentColour);

    juce::Colour getAccent() const noexcept { return accent; }
    juce::Rectangle<int> getContentBounds() const noexcept;

    void paint (juce::Graphics&) override;

private:
    juce::String title;
    juce::Colour accent;
};

// ── Labeled rotary Knob ──────────────────────────────────────────────────
// Title above the knob; value-text below (drawn by the slider itself).
// Accent colour propagates to the LookAndFeel via slider colour IDs.
class Knob final : public juce::Component
{
public:
    Knob (const juce::String& titleText, juce::Colour accentColour);

    juce::Slider& getSlider() noexcept { return slider; }

    void resized() override;

private:
    juce::Label  title;
    juce::Slider slider;
};

// ── Labeled horizontal slider ────────────────────────────────────────────
class HSlider final : public juce::Component
{
public:
    HSlider (const juce::String& titleText, juce::Colour accentColour);

    juce::Slider& getSlider() noexcept { return slider; }

    void resized() override;

private:
    juce::Label  title;
    juce::Slider slider;
};

// ── Chip toggle ─────────────────────────────────────────────────────────
// Segmented control: one chip per choice, all in a single horizontal row.
// Binds to an AudioParameterChoice via ParameterAttachment. Each chip is a
// juce::TextButton in toggle mode and the group enforces single selection.
class ChipToggle final : public juce::Component
{
public:
    ChipToggle (juce::AudioProcessorValueTreeState& apvts,
                const juce::String& parameterID,
                juce::Colour accentColour);

    void resized() override;

private:
    void onChipClicked (int index);
    void refreshFromParameter();

    juce::AudioProcessorValueTreeState& apvts;
    juce::String paramID;
    juce::Colour accent;
    juce::RangedAudioParameter* param { nullptr };
    std::vector<std::unique_ptr<juce::TextButton>> chips;
    std::unique_ptr<juce::ParameterAttachment> attachment;
};

// ── Level meter ─────────────────────────────────────────────────────────
// Vertical level meter. Pure visual — the editor pushes peak values in
// via setLevel(); decay happens on a 30 Hz timer. Two-channel layout
// (L/R) supports the master output strip.
class LevelMeter final : public juce::Component, private juce::Timer
{
public:
    explicit LevelMeter (int numChannels);
    ~LevelMeter() override;

    void setLevel (int channel, float linearPeak) noexcept;
    void paint (juce::Graphics&) override;

private:
    void timerCallback() override;

    std::vector<std::atomic<float>> targetLevels;
    std::vector<float> displayLevels;
};

// ── Background paint helpers ────────────────────────────────────────────
namespace paintutil
{
    void fillBackground (juce::Graphics& g, juce::Rectangle<float> bounds,
                         juce::Colour fill);

    void drawAccentBar (juce::Graphics& g, juce::Rectangle<float> headerArea,
                        juce::Colour accent);
}

} // namespace bjf::gui
