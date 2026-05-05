#include "OscillatorStack.h"

#include <algorithm>
#include <cmath>
#include <cstddef>

namespace bjf
{

void OscillatorStack::setSampleRate (double sr) noexcept
{
    for (auto& o : oscillators)
        o.setSampleRate (sr);
    recomputeFrequencies();
}

void OscillatorStack::setBaseFrequency (float hz) noexcept
{
    baseHz = hz;
    recomputeFrequencies();
}

void OscillatorStack::setStackSize (int count) noexcept
{
    stackSize = std::clamp (count, 1, kMaxOscs);
    recomputeFrequencies();
}

void OscillatorStack::setDetuneCents (float cents) noexcept
{
    detuneCents = cents;
    recomputeFrequencies();
}

void OscillatorStack::setWaveform (Oscillator::Waveform w) noexcept
{
    for (auto& o : oscillators)
        o.setWaveform (w);
}

void OscillatorStack::resetPhases() noexcept
{
    // Scatter phases evenly across the active stack so unisons sum to a thick
    // chorus rather than a synchronised single transient.
    const auto denom = static_cast<float> (stackSize);
    const auto n     = static_cast<std::size_t> (stackSize);
    for (std::size_t i = 0; i < n; ++i)
        oscillators[i].setPhase (static_cast<float> (i) / denom);
}

void OscillatorStack::recomputeFrequencies() noexcept
{
    const auto n = static_cast<std::size_t> (stackSize);
    for (std::size_t i = 0; i < n; ++i)
    {
        const float cents = (stackSize == 1)
            ? 0.0f
            : -detuneCents
              + 2.0f * detuneCents
                * (static_cast<float> (i) / static_cast<float> (stackSize - 1));

        const float ratio = std::pow (2.0f, cents / 1200.0f);
        oscillators[i].setFrequency (baseHz * ratio);
    }
}

float OscillatorStack::renderNextSample() noexcept
{
    float sum = 0.0f;
    const auto n = static_cast<std::size_t> (stackSize);
    for (std::size_t i = 0; i < n; ++i)
        sum += oscillators[i].renderNextSample();

    return sum / std::sqrt (static_cast<float> (stackSize));
}

} // namespace bjf
