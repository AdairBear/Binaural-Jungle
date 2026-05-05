#include "Filter.h"

namespace bjf
{

void Filter::prepare (double sampleRate)
{
    juce::dsp::ProcessSpec spec {};
    spec.sampleRate       = sampleRate;
    spec.maximumBlockSize = 1; // sample-by-sample inside voice render
    spec.numChannels      = 1;

    svf.prepare (spec);
    svf.setType (juce::dsp::StateVariableTPTFilterType::lowpass);
    svf.setCutoffFrequency (1200.0f);
    svf.setResonance (0.707f);
}

void Filter::reset() noexcept
{
    svf.reset();
}

void Filter::setType (Type t) noexcept
{
    using JT = juce::dsp::StateVariableTPTFilterType;
    switch (t)
    {
        case Type::LowPass:  svf.setType (JT::lowpass);  break;
        case Type::BandPass: svf.setType (JT::bandpass); break;
        case Type::HighPass: svf.setType (JT::highpass); break;
    }
}

void Filter::setCutoff (float hz) noexcept     { svf.setCutoffFrequency (hz); }
void Filter::setResonance (float q) noexcept   { svf.setResonance (q); }

float Filter::process (float in) noexcept
{
    return svf.processSample (0, in);
}

} // namespace bjf
