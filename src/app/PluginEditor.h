#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginProcessor.h"

namespace bjf
{

class BinauralJungleForgeEditor final : public juce::AudioProcessorEditor
{
public:
    explicit BinauralJungleForgeEditor (BinauralJungleForgeProcessor&);
    ~BinauralJungleForgeEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    BinauralJungleForgeProcessor& processorRef;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BinauralJungleForgeEditor)
};

} // namespace bjf
