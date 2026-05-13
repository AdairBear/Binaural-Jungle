// Smoke test for the step-10 EarlyReflections module.
//
// What we want to prove without a DAW:
//   1. Mix = 0 is bit-exactly a passthrough — wiring ER into the chain at
//      its default never alters the dry signal.
//   2. With mix > 0 the module adds 6 distinct first-order taps to the HOA
//      bus, at delays that grow with room size and shrink with smaller rooms.
//   3. Wall damping reduces broadband energy of the reflection cluster
//      monotonically (heavier damping → less RMS in the tail).
//   4. The output stays finite — no NaN, no Inf, no runaway feedback.
//   5. Reset zeros the delay line so an impulse fired post-reset behaves
//      identically to one fired at construction.

#include "../src/dsp/spatial/EarlyReflections.h"
#include "../src/dsp/spatial/HOACoefficients.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <iostream>

namespace
{
constexpr double kSampleRate = 48000.0;
constexpr int    kBlockSamples = 8192;

int fail (const char* msg) { std::cerr << "FAIL: " << msg << '\n'; return 1; }

bool isFinite16 (const float* hoa)
{
    for (int i = 0; i < bjf::spatial::kNumHoaChannels; ++i)
        if (! std::isfinite (hoa[i])) return false;
    return true;
}

// Feed an impulse on W (mono dry = 1.0 at t=0, 0 thereafter) through the
// model and return the total absolute energy summed across all 16 HOA
// channels over `numSamples` samples.
double impulseEnergy (bjf::spatial::EarlyReflections& er, int numSamples)
{
    double energy = 0.0;
    float in[bjf::spatial::kNumHoaChannels] {};
    for (int n = 0; n < numSamples; ++n)
    {
        // Pure mono drive: only W carries signal; the other HOA channels
        // start at zero. Mirrors what the production processBlock will pass
        // when no voice is encoding directionally at this sample.
        std::fill_n (in, bjf::spatial::kNumHoaChannels, 0.0f);
        in[0] = (n == 0) ? 1.0f : 0.0f;

        float out[bjf::spatial::kNumHoaChannels];
        std::copy_n (in, bjf::spatial::kNumHoaChannels, out);
        er.processSample (in, out);

        if (! isFinite16 (out))
            return std::nan ("");

        // We're interested in what ER added on top of the dry impulse.
        out[0] -= in[0];

        for (int c = 0; c < bjf::spatial::kNumHoaChannels; ++c)
            energy += static_cast<double> (out[c]) * static_cast<double> (out[c]);
    }
    return energy;
}
}

int main()
{
    using bjf::spatial::EarlyReflections;
    using bjf::spatial::kNumHoaChannels;

    // ── 1. Mix = 0 → exact passthrough. ────────────────────────────────────
    {
        EarlyReflections er;
        er.prepare (kSampleRate);
        er.setRoomSize    (8.0f);
        er.setWallDamping (0.3f);
        er.setMix         (0.0f);

        for (int n = 0; n < 1024; ++n)
        {
            float in[kNumHoaChannels] {};
            in[0] = (n == 0) ? 1.0f : 0.5f;
            in[3] = 0.25f;  // some directional content too

            float out[kNumHoaChannels];
            std::copy_n (in, kNumHoaChannels, out);
            er.processSample (in, out);

            for (int c = 0; c < kNumHoaChannels; ++c)
                if (out[c] != in[c])
                    return fail ("mix=0 should be exact passthrough");
        }
        std::cout << "mix=0 passthrough: OK\n";
    }

    // ── 2. Reflection delays scale with room size. ────────────────────────
    {
        EarlyReflections er;
        er.prepare (kSampleRate);
        er.setWallDamping (0.0f);
        er.setMix         (1.0f);

        er.setRoomSize (2.0f);
        int delaysSmall[EarlyReflections::kNumReflections];
        for (int i = 0; i < EarlyReflections::kNumReflections; ++i)
            delaysSmall[i] = er.getReflectionDelaySamples (i);

        er.setRoomSize (20.0f);
        int delaysLarge[EarlyReflections::kNumReflections];
        for (int i = 0; i < EarlyReflections::kNumReflections; ++i)
            delaysLarge[i] = er.getReflectionDelaySamples (i);

        std::cout << "delays (2 m room):  ";
        for (int d : delaysSmall) std::cout << d << " ";
        std::cout << '\n';
        std::cout << "delays (20 m room): ";
        for (int d : delaysLarge) std::cout << d << " ";
        std::cout << '\n';

        // Sanity — 2 m room should give one-way distances ~1–1.5 m, so taps
        // around 140–220 samples at 48 kHz; 20 m room should be 10× longer.
        for (int i = 0; i < EarlyReflections::kNumReflections; ++i)
        {
            if (delaysSmall[i] <= 0)
                return fail ("non-positive delay for small room");
            if (delaysLarge[i] <= delaysSmall[i])
                return fail ("delay did not grow with room size");
            if (delaysLarge[i] >= EarlyReflections::kDelayBufferSize)
                return fail ("delay overflowed ring buffer");
        }
    }

    // ── 3. Mix > 0 produces energy on top of the dry path. ────────────────
    {
        EarlyReflections er;
        er.prepare (kSampleRate);
        er.setRoomSize    (8.0f);
        er.setWallDamping (0.0f);
        er.setMix         (1.0f);

        const auto energy = impulseEnergy (er, kBlockSamples);
        if (! std::isfinite (energy))
            return fail ("non-finite energy in impulse response");
        if (energy <= 0.0)
            return fail ("ER produced no energy at mix=1.0");
        std::cout << "impulse energy (mix=1, damp=0): " << energy << '\n';
    }

    // ── 4. Wall damping reduces total energy monotonically. ───────────────
    {
        EarlyReflections er;
        er.prepare (kSampleRate);
        er.setRoomSize (8.0f);
        er.setMix      (1.0f);

        er.setWallDamping (0.0f);
        const auto eOpen = impulseEnergy (er, kBlockSamples);

        er.reset();
        er.setWallDamping (0.5f);
        const auto eMid = impulseEnergy (er, kBlockSamples);

        er.reset();
        er.setWallDamping (1.0f);
        const auto eDamped = impulseEnergy (er, kBlockSamples);

        std::cout << "energy by damping: open=" << eOpen
                  << "  mid=" << eMid
                  << "  damped=" << eDamped << '\n';

        if (! (eOpen > eMid && eMid > eDamped))
            return fail ("energy should decrease monotonically with damping");
    }

    // ── 5. Mix scales total energy ≈ quadratically. ───────────────────────
    //     (Each reflection is multiplied by `mix` once before encode →
    //      output is ∝ mix, energy is ∝ mix² up to a small tolerance.)
    {
        EarlyReflections er;
        er.prepare (kSampleRate);
        er.setRoomSize    (8.0f);
        er.setWallDamping (0.0f);

        er.setMix (1.0f);
        const auto e1 = impulseEnergy (er, kBlockSamples);

        er.reset();
        er.setMix (0.5f);
        const auto e2 = impulseEnergy (er, kBlockSamples);

        if (e2 <= 0.0 || e1 <= 0.0) return fail ("zero ER energy");
        const auto ratio = e1 / e2;
        std::cout << "energy ratio (mix=1 / mix=0.5): " << ratio
                  << "  (expect ~4)\n";
        if (ratio < 3.5 || ratio > 4.5)
            return fail ("mix scaling not approximately quadratic");
    }

    // ── 6. Reset clears the delay line. ───────────────────────────────────
    {
        EarlyReflections er;
        er.prepare (kSampleRate);
        er.setRoomSize    (8.0f);
        er.setWallDamping (0.0f);
        er.setMix         (1.0f);

        const auto eA = impulseEnergy (er, kBlockSamples);

        // Pour more energy in to dirty the buffer.
        float in[kNumHoaChannels] {};
        in[0] = 0.7f;
        for (int n = 0; n < 1024; ++n)
        {
            float out[kNumHoaChannels];
            std::copy_n (in, kNumHoaChannels, out);
            er.processSample (in, out);
        }

        er.reset();
        const auto eB = impulseEnergy (er, kBlockSamples);

        std::cout << "impulse energy pre-reset = " << eA
                  << "  post-reset = " << eB << '\n';

        const auto rel = std::abs (eA - eB) / std::max (eA, 1e-12);
        if (rel > 1e-5)
            return fail ("reset did not restore initial state");
    }

    // ── 7. DC steady-state stays finite (no feedback runaway). ────────────
    {
        EarlyReflections er;
        er.prepare (kSampleRate);
        er.setRoomSize    (8.0f);
        er.setWallDamping (0.5f);
        er.setMix         (1.0f);

        float in[kNumHoaChannels] {};
        in[0] = 1.0f;

        float peak = 0.0f;
        for (int n = 0; n < 96000; ++n)  // 2 seconds of DC drive
        {
            float out[kNumHoaChannels];
            std::copy_n (in, kNumHoaChannels, out);
            er.processSample (in, out);

            if (! isFinite16 (out))
                return fail ("non-finite sample under DC drive");

            for (int c = 0; c < kNumHoaChannels; ++c)
                peak = std::max (peak, std::abs (out[c]));
        }
        std::cout << "DC drive peak after 2s: " << peak << '\n';
        if (peak > 50.0f)
            return fail ("DC drive runaway — peak too high");
    }

    std::cout << "PASS\n";
    return 0;
}
