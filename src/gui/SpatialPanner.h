#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>

namespace bjf::gui
{

// Spatial showpiece. Renders the listener's head at the centre of an
// orbital elliptical plane and draws 16 voice particles distributed around
// that orbit using the same spread math as VoiceManager::setSpatial — so
// what the user sees on screen is exactly where the voices will actually
// sit in the HOA field.
//
// Click-and-drag anywhere on the panner moves the center (az/el) like an
// XY pad. The spread is wired through Spread Az/El knobs, not this widget,
// to keep the gesture clean.
class SpatialPanner final : public juce::Component, private juce::Timer
{
public:
    SpatialPanner (juce::AudioProcessorValueTreeState& apvts,
                   const juce::String& azParam,
                   const juce::String& elParam,
                   const juce::String& spreadAzParam,
                   const juce::String& spreadElParam,
                   const juce::String& voicesParam);

    ~SpatialPanner() override;

    void paint (juce::Graphics&) override;
    void resized() override;
    void mouseDown (const juce::MouseEvent&) override;
    void mouseDrag (const juce::MouseEvent&) override;

private:
    void timerCallback() override;
    void writeFromMouse (const juce::MouseEvent&);

    // Mirror of VoiceManager spread maths — kept here as a pure function so
    // the panner doesn't link the audio engine. If the DSP-side logic ever
    // changes, both sides need to update.
    void computeVoicePositions (std::vector<juce::Point<float>>& outAzEl) const;

    juce::AudioProcessorValueTreeState& apvts;
    juce::RangedAudioParameter* azParam       { nullptr };
    juce::RangedAudioParameter* elParam       { nullptr };
    juce::RangedAudioParameter* spreadAzParam { nullptr };
    juce::RangedAudioParameter* spreadElParam { nullptr };
    juce::RangedAudioParameter* voicesParam   { nullptr };

    // Cached normalised state — only repaint when these drift.
    float cachedAz { 0.0f }, cachedEl { 0.0f };
    float cachedSpreadAz { 0.0f }, cachedSpreadEl { 0.0f };
    int   cachedVoices { 16 };
};

} // namespace bjf::gui
