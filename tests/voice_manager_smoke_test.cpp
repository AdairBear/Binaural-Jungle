// Smoke test for bjf::VoiceManager. Verifies polyphonic allocation, oldest-
// note stealing when the pool is exhausted, and per-note routing of note-offs.

#include "../src/dsp/voice/VoiceManager.h"

#include <cmath>
#include <cstdlib>
#include <iostream>

namespace
{
struct Stats { float peak; double rms; };

Stats render (bjf::VoiceManager& vm, int numSamples)
{
    Stats s {};
    double sumSq = 0.0;
    for (int i = 0; i < numSamples; ++i)
    {
        const auto x = vm.renderNextSample();
        s.peak = std::max (s.peak, std::abs (x));
        sumSq += static_cast<double> (x) * x;
    }
    s.rms = std::sqrt (sumSq / numSamples);
    return s;
}

int fail (const char* msg) { std::cerr << "FAIL: " << msg << '\n'; return 1; }
}

int main()
{
    constexpr double sampleRate = 48000.0;
    constexpr int    numSamples = 4800; // 100 ms

    bjf::VoiceManager vm;
    vm.prepare (sampleRate);
    vm.setWaveform (bjf::Oscillator::Waveform::Saw);
    vm.setEnvelopeParameters (0.001f, 0.05f, 0.7f, 0.1f);

    // ── One note → one active voice ──
    vm.noteOn (60, 1.0f);
    if (vm.getActiveVoiceCount() != 1)
        return fail ("expected 1 active voice after single note-on");

    const auto one = render (vm, numSamples);
    std::cout << "1 note  : peak=" << one.peak
              << "  rms=" << one.rms
              << "  active=" << vm.getActiveVoiceCount() << '\n';

    if (one.peak < 0.1f) return fail ("single voice silent");

    // ── Stack a chord → multiple active voices, louder sum ──
    const int chord[] = { 60, 64, 67, 71, 74 };
    for (auto n : chord) vm.noteOn (n, 1.0f);

    if (vm.getActiveVoiceCount() < static_cast<int> (sizeof (chord) / sizeof (chord[0])))
        return fail ("expected one voice per chord note");

    const auto chordStats = render (vm, numSamples);
    std::cout << "chord   : peak=" << chordStats.peak
              << "  rms=" << chordStats.rms
              << "  active=" << vm.getActiveVoiceCount() << '\n';

    if (chordStats.rms <= one.rms)
        return fail ("chord rms should exceed single-note rms");

    // ── Note-off routes to the right voice ──
    vm.noteOff (60);
    vm.noteOff (64);
    vm.noteOff (67);
    vm.noteOff (71);
    vm.noteOff (74);

    // 500 ms tail — short release means everything should be silent by then.
    render (vm, static_cast<int> (sampleRate * 0.5));
    if (vm.getActiveVoiceCount() != 0)
        return fail ("voices still active after note-off + tail");

    // ── Voice stealing: fire 17 notes, pool only holds 16 ──
    for (int n = 60; n < 60 + bjf::VoiceManager::kMaxVoices + 1; ++n)
        vm.noteOn (n, 1.0f);

    if (vm.getActiveVoiceCount() != bjf::VoiceManager::kMaxVoices)
        return fail ("active voice count must cap at kMaxVoices");

    const auto poly = render (vm, numSamples);
    std::cout << "16+1    : peak=" << poly.peak
              << "  rms=" << poly.rms
              << "  active=" << vm.getActiveVoiceCount() << '\n';

    if (poly.rms < chordStats.rms)
        return fail ("16-voice rms should be >= 5-voice chord rms");

    // ── allNotesOff drains the pool ──
    vm.allNotesOff();
    render (vm, static_cast<int> (sampleRate * 0.5));
    if (vm.getActiveVoiceCount() != 0)
        return fail ("voices still active after allNotesOff + tail");

    std::cout << "PASS\n";
    return 0;
}
