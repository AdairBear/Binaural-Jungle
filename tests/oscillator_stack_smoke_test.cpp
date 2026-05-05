// Smoke test for bjf::OscillatorStack. Verifies that the detuned unison
// produces audible output at multiple stack sizes, that level normalisation
// keeps the output within sane bounds, and that detune actually pulls the
// constituent oscillators apart in frequency.

#include "../src/dsp/voice/OscillatorStack.h"

#include <cmath>
#include <cstdlib>
#include <iostream>

namespace
{
struct Stats { float peak; double rms; int nonZeroCount; };

Stats render (bjf::OscillatorStack& s, int numSamples)
{
    Stats st {};
    double sumSq = 0.0;
    for (int i = 0; i < numSamples; ++i)
    {
        const auto x = s.renderNextSample();
        st.peak = std::max (st.peak, std::abs (x));
        sumSq += static_cast<double> (x) * x;
        if (std::abs (x) > 1e-6f) ++st.nonZeroCount;
    }
    st.rms = std::sqrt (sumSq / numSamples);
    return st;
}

int fail (const char* msg) { std::cerr << "FAIL: " << msg << '\n'; return 1; }
}

int main()
{
    constexpr double sr = 48000.0;
    constexpr int    n  = 4800; // 100 ms

    bjf::OscillatorStack s;
    s.setSampleRate (sr);
    s.setWaveform (bjf::Oscillator::Waveform::Saw);
    s.setBaseFrequency (220.0f);

    // ── Stack size 1: pure single-oscillator behaviour ──
    s.setStackSize (1);
    s.setDetuneCents (10.0f); // detune is irrelevant at N=1
    s.resetPhases();
    const auto one = render (s, n);
    std::cout << "N=1   : peak=" << one.peak
              << "  rms=" << one.rms
              << "  nonzero=" << one.nonZeroCount << "/" << n << '\n';
    if (one.peak < 0.5f)        return fail ("N=1 saw silent");
    if (one.peak > 1.01f)       return fail ("N=1 saw clipping");

    // ── Stack size 3 with detune: thicker chorus, level not amplified ──
    s.setStackSize (3);
    s.setDetuneCents (12.0f);
    s.resetPhases();
    const auto three = render (s, n);
    std::cout << "N=3   : peak=" << three.peak
              << "  rms=" << three.rms
              << "  nonzero=" << three.nonZeroCount << "/" << n << '\n';
    if (three.peak < 0.3f)      return fail ("N=3 unison too quiet");
    if (three.peak > 1.2f)      return fail ("N=3 unison too loud — sqrt-N normalisation broken");

    // ── Stack size 7 (max), detune 25 cents ──
    s.setStackSize (7);
    s.setDetuneCents (25.0f);
    s.resetPhases();
    const auto seven = render (s, n);
    std::cout << "N=7   : peak=" << seven.peak
              << "  rms=" << seven.rms
              << "  nonzero=" << seven.nonZeroCount << "/" << n << '\n';
    if (seven.peak < 0.3f)      return fail ("N=7 unison silent");
    if (seven.peak > 1.5f)      return fail ("N=7 unison level out of bounds");

    // ── Detune produces beating: sum-of-squares over a longer window for
    //    detune=0 (true unison, all phases collapse over time only because
    //    of phase scatter, frequencies identical) vs detune=25 should differ.
    //    Cheap proxy: with strong detune, peak over a long window should
    //    approach the un-normalised sum since beats line up periodically.
    s.setStackSize (5);
    s.setDetuneCents (0.0f);
    s.resetPhases();
    const auto unisonZero = render (s, static_cast<int> (sr)); // 1 second
    s.setDetuneCents (30.0f);
    s.resetPhases();
    const auto unisonDetuned = render (s, static_cast<int> (sr));
    std::cout << "1s 0c : peak=" << unisonZero.peak    << "  rms=" << unisonZero.rms    << '\n';
    std::cout << "1s 30c: peak=" << unisonDetuned.peak << "  rms=" << unisonDetuned.rms << '\n';

    if (unisonDetuned.peak <= unisonZero.peak * 1.01f)
        return fail ("detuned unison should peak higher than zero-detune (beats line up)");

    std::cout << "PASS\n";
    return 0;
}
