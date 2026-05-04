#include "PluginProcessor.h"
#include "PluginEditor.h"

namespace bjf
{

BinauralJungleForgeProcessor::BinauralJungleForgeProcessor()
    : AudioProcessor (BusesProperties()
        .withOutput ("Output", juce::AudioChannelSet::stereo(), true))
{
}

BinauralJungleForgeProcessor::~BinauralJungleForgeProcessor() = default;

void BinauralJungleForgeProcessor::prepareToPlay (double /*sampleRate*/,
                                                  int /*samplesPerBlock*/)
{
}

void BinauralJungleForgeProcessor::releaseResources()
{
}

bool BinauralJungleForgeProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto& mainOut = layouts.getMainOutputChannelSet();
    return mainOut == juce::AudioChannelSet::stereo()
        || mainOut == juce::AudioChannelSet::mono();
}

void BinauralJungleForgeProcessor::processBlock (juce::AudioBuffer<float>& buffer,
                                                 juce::MidiBuffer& /*midiMessages*/)
{
    juce::ScopedNoDenormals noDenormals;

    for (int ch = 0; ch < getTotalNumOutputChannels(); ++ch)
        buffer.clear (ch, 0, buffer.getNumSamples());
}

juce::AudioProcessorEditor* BinauralJungleForgeProcessor::createEditor()
{
    return new BinauralJungleForgeEditor (*this);
}

void BinauralJungleForgeProcessor::getStateInformation (juce::MemoryBlock& /*destData*/)
{
}

void BinauralJungleForgeProcessor::setStateInformation (const void* /*data*/, int /*sizeInBytes*/)
{
}

} // namespace bjf

// JUCE plugin entry point.
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new bjf::BinauralJungleForgeProcessor();
}
