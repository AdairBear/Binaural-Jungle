// Smoke test for bjf::Filter. Drives sine tones through the SVF and verifies
// each mode (LP / BP / HP) attenuates frequencies it's supposed to and passes
// frequencies it's supposed to. Sine tones are a much cleaner test than
// broadband noise for a 12 dB/oct filter — the math is obvious.

#include "../src/dsp/voice/Filter.h"

#include <cmath>
#include <cstdlib>
#include <iostream>

namespace
{
constexpr double kTwoPi = 6.283185307179586;

double sineRmsThrough (bjf::Filter& f, double sr, double freqHz, int numSamples)
{
    double phase = 0.0;
    const double inc = kTwoPi * freqHz / sr;
    double sumSq = 0.0;
    for (int i = 0; i < numSamples; ++i)
    {
        const auto x = static_cast<float> (std::sin (phase));
        phase += inc;
        if (phase >= kTwoPi) phase -= kTwoPi;
        const auto y = f.process (x);
        sumSq += static_cast<double> (y) * y;
    }
    return std::sqrt (sumSq / numSamples);
}

int fail (const char* msg) { std::cerr << "FAIL: " << msg << '\n'; return 1; }
}

int main()
{
    constexpr double sr = 48000.0;
    constexpr int    n  = 12000; // 0.25 s — long enough for transient to settle
    // Reference RMS of a unity-amplitude sine = 1/sqrt(2) ≈ 0.707.

    bjf::Filter f;
    f.prepare (sr);
    f.setResonance (0.707f); // Butterworth, no resonant peak

    // ── Low-pass: passes 100 Hz ~unity, kills 8 kHz ──
    f.reset();
    f.setType (bjf::Filter::Type::LowPass);
    f.setCutoff (1000.0f);
    const auto lpPass    = sineRmsThrough (f, sr, 100.0,   n);
    f.reset();
    const auto lpStop    = sineRmsThrough (f, sr, 8000.0,  n);

    // ── High-pass: passes 8 kHz ~unity, kills 100 Hz ──
    f.reset();
    f.setType (bjf::Filter::Type::HighPass);
    f.setCutoff (1000.0f);
    const auto hpPass    = sineRmsThrough (f, sr, 8000.0,  n);
    f.reset();
    const auto hpStop    = sineRmsThrough (f, sr, 100.0,   n);

    // ── Band-pass: passes near cutoff, kills well-separated freqs ──
    f.reset();
    f.setType (bjf::Filter::Type::BandPass);
    f.setCutoff (1000.0f);
    const auto bpPass    = sineRmsThrough (f, sr, 1000.0,  n);
    f.reset();
    const auto bpStopLow = sineRmsThrough (f, sr, 50.0,    n);
    f.reset();
    const auto bpStopHi  = sineRmsThrough (f, sr, 18000.0, n);

    std::cout << "LP @1k: pass(100Hz)="    << lpPass    << "  stop(8kHz)="  << lpStop    << '\n';
    std::cout << "HP @1k: pass(8kHz)="     << hpPass    << "  stop(100Hz)=" << hpStop    << '\n';
    std::cout << "BP @1k: pass(1kHz)="     << bpPass
              << "  stop(50Hz)="           << bpStopLow
              << "  stop(18kHz)="          << bpStopHi  << '\n';

    // 2nd-order LP/HP at f_c/8 octaves of separation: ~36 dB attenuation
    // (factor ≈ 0.016). Use a generous threshold (factor 0.1) to keep the
    // assertion robust to filter-topology details and warm-up transients.
    if (lpPass < 0.5)               return fail ("LP pass-band too quiet (expected ~0.707)");
    if (lpStop > 0.1)               return fail ("LP failed to kill 8 kHz at 1 kHz cutoff");

    if (hpPass < 0.5)               return fail ("HP pass-band too quiet");
    if (hpStop > 0.1)               return fail ("HP failed to kill 100 Hz at 1 kHz cutoff");

    if (bpPass < 0.3)               return fail ("BP pass-band silent at cutoff");
    if (bpStopLow > bpPass * 0.3)   return fail ("BP failed to attenuate 50 Hz");
    if (bpStopHi  > bpPass * 0.3)   return fail ("BP failed to attenuate 18 kHz");

    std::cout << "PASS\n";
    return 0;
}
