#include "Oscillator.h"

namespace bjf
{

void Oscillator::setSampleRate (double sr) noexcept
{
    sampleRateHz = sr;
}

void Oscillator::setFrequency (float hz) noexcept
{
    phaseInc = static_cast<double> (hz) / sampleRateHz;
}

void Oscillator::setWaveform (Waveform w) noexcept
{
    waveform = w;
}

void Oscillator::resetPhase() noexcept
{
    phase = 0.0;
}

float Oscillator::renderNextSample() noexcept
{
    float out = 0.0f;
    switch (waveform)
    {
        case Waveform::Saw:    out = static_cast<float> (2.0 * phase - 1.0); break;
        case Waveform::Square: out = phase < 0.5 ? 1.0f : -1.0f;             break;
    }

    phase += phaseInc;
    if (phase >= 1.0)
        phase -= 1.0;

    return out;
}

} // namespace bjf
