#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

#include <memory>

#include "../gui/BJFLookAndFeel.h"
#include "../gui/Controls.h"
#include "../gui/Panels.h"
#include "../gui/SpatialPanner.h"
#include "PluginProcessor.h"

namespace bjf
{

// Full-window editor. Layout zones (matching the design HTML prototype):
//   ┌─────────────────────────────────────────────────────────────────┐
//   │                       TopBar (logo + master)                    │
//   ├─────────────────────┬───────────────────────────────────────────┤
//   │                     │              Oscillator                   │
//   │                     ├───────────────────────────────────────────┤
//   │   SpatialPanner     │              Granular                     │
//   │                     ├───────────────────────────────────────────┤
//   │                     │              Filter                       │
//   │                     ├───────────────────────────────────────────┤
//   │                     │              Envelope                     │
//   │                     ├───────────────────────────────────────────┤
//   │                     │              HOA Reverb                   │
//   ├─────────────────────┴───────────────────────────────────────────┤
//   │                  BottomBar (keyboard + meter)                   │
//   └─────────────────────────────────────────────────────────────────┘
//
// All inner panels mount their own APVTS attachments; the editor only owns
// the layout and the peak-poll timer that drives the master level meter.
class BinauralJungleForgeEditor final : public juce::AudioProcessorEditor,
                                        private juce::Timer
{
public:
    explicit BinauralJungleForgeEditor (BinauralJungleForgeProcessor&);
    ~BinauralJungleForgeEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;

    BinauralJungleForgeProcessor& processorRef;

    gui::BJFLookAndFeel lnf;

    gui::TopBar         topBar;
    gui::SpatialPanner  spatialPanner;
    gui::OscillatorPanel oscillatorPanel;
    gui::GranularPanel  granularPanel;
    gui::FilterPanel    filterPanel;
    gui::EnvelopePanel  envelopePanel;
    gui::ReverbPanel    reverbPanel;
    gui::BottomBar      bottomBar;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BinauralJungleForgeEditor)
};

} // namespace bjf
