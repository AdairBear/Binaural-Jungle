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

void Oscillator::setPhase (float phase01) noexcept
{
    auto p = static_cast<double> (phase01);
    p -= static_cast<double> (static_cast<long long> (p)); // wrap to [0, 1) for non-negative inputs
    if (p < 0.0) p += 1.0;
    phase = p;
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
