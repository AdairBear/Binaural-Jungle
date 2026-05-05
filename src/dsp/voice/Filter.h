#pragma once

#include <juce_dsp/juce_dsp.h>

namespace bjf
{

// State-variable TPT filter, mono. Wraps juce::dsp::StateVariableTPTFilter and
// is processed sample-by-sample inside the voice render loop — voices already
// produce one mono sample at a time, so block processing buys nothing.
//
// Modes are LP / BP / HP (notch + peak deferred until the parameter layout
// asks for them). Resonance follows JUCE's TPT convention: ~0.7 = Butterworth,
// up to ~10 for self-oscillation territory.
class Filter
{
public:
    enum class Type { LowPass, BandPass, HighPass };

    void prepare (double sampleRate);
    void reset() noexcept;

    void setType (Type t) noexcept;
    void setCutoff (float hz) noexcept;
    void setResonance (float q) noexcept;

    float process (float in) noexcept;

private:
    juce::dsp::StateVariableTPTFilter<float> svf;
};

} // namespace bjf
