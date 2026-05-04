#include "PluginEditor.h"

namespace bjf
{

BinauralJungleForgeEditor::BinauralJungleForgeEditor (BinauralJungleForgeProcessor& p)
    : AudioProcessorEditor (&p), processorRef (p)
{
    juce::ignoreUnused (processorRef);
    setSize (480, 320);
}

BinauralJungleForgeEditor::~BinauralJungleForgeEditor() = default;

void BinauralJungleForgeEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (0xff121417));

    g.setColour (juce::Colours::white);
    g.setFont (juce::FontOptions (22.0f, juce::Font::bold));
    g.drawFittedText ("Binaural Jungle Forge",
                      getLocalBounds().reduced (16),
                      juce::Justification::centredTop, 1);

    g.setColour (juce::Colour (0xff7a8590));
    g.setFont (juce::FontOptions (12.0f));
    g.drawFittedText ("v0.1 — skeleton",
                      getLocalBounds().reduced (16, 48),
                      juce::Justification::centredTop, 1);
}

void BinauralJungleForgeEditor::resized()
{
}

} // namespace bjf
