#include "PluginEditor.h"

#include "ParameterIDs.h"

namespace bjf
{

namespace
{
    // Window geometry from the HTML prototype. Treated as the canonical
    // editor size — the host will request resize via setResizable() if we
    // ever opt in, but for v1 the editor renders at a fixed 1280×800.
    constexpr int kWindowW = 1280;
    constexpr int kWindowH = 800;

    constexpr int kPad         = 12;
    constexpr int kTopBarH     = 64;
    constexpr int kBottomBarH  = 120;
    constexpr int kSpatialW    = 416;
    constexpr int kGap         = 10;
}

BinauralJungleForgeEditor::BinauralJungleForgeEditor (BinauralJungleForgeProcessor& p)
    : AudioProcessorEditor (&p), processorRef (p),
      topBar          (p.apvts),
      spatialPanner   (p.apvts, pid::spatialAz, pid::spatialEl,
                       pid::spatialSpreadAz, pid::spatialSpreadEl,
                       pid::voices),
      oscillatorPanel (p.apvts),
      granularPanel   (p.apvts),
      filterPanel     (p.apvts),
      envelopePanel   (p.apvts),
      reverbPanel     (p.apvts),
      bottomBar       (p.keyboardState, /*baseOctave*/ 2)
{
    setLookAndFeel (&lnf);

    addAndMakeVisible (topBar);
    addAndMakeVisible (spatialPanner);
    addAndMakeVisible (oscillatorPanel);
    addAndMakeVisible (granularPanel);
    addAndMakeVisible (filterPanel);
    addAndMakeVisible (envelopePanel);
    addAndMakeVisible (reverbPanel);
    addAndMakeVisible (bottomBar);

    setSize (kWindowW, kWindowH);

    // 30 Hz is the lowest rate that still feels responsive on a level meter
    // without burning CPU on repaints.
    startTimerHz (30);
}

BinauralJungleForgeEditor::~BinauralJungleForgeEditor()
{
    stopTimer();
    setLookAndFeel (nullptr);
}

void BinauralJungleForgeEditor::timerCallback()
{
    bottomBar.getMeter().setLevel (0,
        processorRef.outputPeakL.load (std::memory_order_relaxed));
    bottomBar.getMeter().setLevel (1,
        processorRef.outputPeakR.load (std::memory_order_relaxed));
}

void BinauralJungleForgeEditor::paint (juce::Graphics& g)
{
    g.fillAll (gui::theme::background);

    // Hairline separators echo the prototype's panel chrome.
    const auto bounds = getLocalBounds().toFloat();
    g.setColour (gui::theme::border);
    g.drawRect (bounds, 1.0f);
}

void BinauralJungleForgeEditor::resized()
{
    auto bounds = getLocalBounds().reduced (kPad);

    auto top = bounds.removeFromTop (kTopBarH);
    topBar.setBounds (top);
    bounds.removeFromTop (kGap);

    auto bottom = bounds.removeFromBottom (kBottomBarH);
    bottomBar.setBounds (bottom);
    bounds.removeFromBottom (kGap);

    // Left: spatial showpiece. Right: stacked parameter panels (5 of them,
    // sharing the vertical span equally with a small gap between).
    auto left = bounds.removeFromLeft (kSpatialW);
    spatialPanner.setBounds (left);
    bounds.removeFromLeft (kGap);

    auto stackArea = bounds;
    constexpr int numPanels = 5;
    const int rowH = (stackArea.getHeight() - kGap * (numPanels - 1)) / numPanels;

    auto layoutNext = [&] (juce::Component& c)
    {
        c.setBounds (stackArea.removeFromTop (rowH));
        if (stackArea.getHeight() > 0)
            stackArea.removeFromTop (kGap);
    };

    layoutNext (oscillatorPanel);
    layoutNext (granularPanel);
    layoutNext (filterPanel);
    layoutNext (envelopePanel);
    layoutNext (reverbPanel);
}

} // namespace bjf
