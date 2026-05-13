#pragma once

#include <array>
#include <vector>

#include "HOACoefficients.h"

namespace bjf::spatial
{

// Step-10 early reflections, image-source model in the HOA domain.
//
// Models a shoebox room with the listener at the origin and a virtual source
// at a small fixed offset. Each of the six walls of the room produces one
// first-order image source; for each image we compute its position relative
// to the listener and from that derive
//   - a sample delay        (distance / speed of sound)
//   - a geometric gain      (1 / distance, capped at 1.0 near-field)
//   - a frequency damping   (one-pole low-pass — heavier walls roll off
//                            high frequencies more aggressively)
//   - an ACN/SN3D encode    (16 SH coefficients at the image's (az, el))
//
// The mono input for the model is the W (channel 0) component of the
// incoming HOA bus. With our SN3D normalisation y[0] = 1, so W is just the
// summed mono content of the scene — a good enough "scene-as-mono" source
// for cheap ER without paying for per-voice tapping.
//
// The output is added (with `mix` scaling) into the destination HOA bus,
// preserving the dry direct path while adding the reflection cluster. When
// the FDN diffuse reverb lands, the same reflection signal can be tapped
// off and fed in as the late-tail input (the architecture in
// design/01-architecture.md positions ER → late diffuse).
//
// Per-sample CPU: 6 reflections × (1 delay read + 1 one-pole LPF + 16
// mul-adds for the encode) ≈ 110 multiplies. Negligible at 48 kHz.
class EarlyReflections
{
public:
    // Six first-order image sources — one per shoebox wall: ±X, ±Y, ±Z.
    static constexpr int kNumReflections = 6;

    // Delay-line length, in samples. Sized for the worst case: 30 m room at
    // 96 kHz, ~88 ms one-way → ~8500 samples. Round up generously to power
    // of two so a wrap-around modulo collapses to a mask.
    static constexpr int kDelayBufferSize = 16384;
    static constexpr int kDelayMask       = kDelayBufferSize - 1;

    // Speed of sound, m/s at 20 °C. Constant by design — we don't model
    // temperature or humidity here; this is a creative reverb, not a
    // simulation. If a user wants tighter ER they reach for "room size".
    static constexpr float kSpeedOfSound = 343.0f;

    EarlyReflections();

    // Must be called before processing. Resets the delay buffer to zero and
    // recomputes reflection delays/positions for the current room.
    void prepare (double sampleRate);

    // Zero state without changing parameters.
    void reset() noexcept;

    // Read a mono sample from `hoaIn[0]` (W channel), run the image-source
    // model, and add the resulting HOA reflection cluster into `hoaOut`.
    //
    // `hoaOut` is written-modified, not overwritten. Callers typically pass
    // the same buffer for both — the dry HOA is preserved and the ER cluster
    // is mixed on top. Both buffers must have ≥ kNumHoaChannels (=16) floats.
    void processSample (const float* hoaIn, float* hoaOut) noexcept;

    // Room "size" — the edge length of the (mostly) cubic shoebox in meters.
    // The model uses slightly different x/y/z dimensions (aspect 1.2 / 1.0
    // / 0.75) so that same-axis reflection pairs don't comb-filter into
    // each other. 2 m ≈ closet, 30 m ≈ cathedral.
    void setRoomSize (float meters) noexcept;

    // Wall absorption, [0..1]. 0 → bright stone, 1 → heavily-damped carpet.
    // Mapped to a one-pole low-pass coefficient per reflection.
    void setWallDamping (float damping01) noexcept;

    // ER level, [0..1]. Scales the reflection cluster before it is summed
    // into the output HOA bus.
    void setMix (float mix01) noexcept;

    // Read-only accessors for tests.
    float getRoomSize()    const noexcept { return roomSize; }
    float getWallDamping() const noexcept { return wallDamping; }
    float getMix()         const noexcept { return mix; }

    int   getReflectionDelaySamples (int i) const noexcept { return reflections[(std::size_t) i].delaySamples; }
    float getReflectionGain         (int i) const noexcept { return reflections[(std::size_t) i].gain; }

private:
    // One image source. The HOA coefficients, delay tap, and per-tap
    // low-pass state are recomputed whenever room geometry changes.
    struct Reflection
    {
        int   delaySamples = 0;
        float gain         = 0.0f;
        float lpfA         = 0.0f;  // one-pole feedback coefficient (α)
        float lpfState     = 0.0f;  // running sample-and-hold
        std::array<float, kNumHoaChannels> hoaCoefs {};
    };

    void recomputeReflections() noexcept;

    double sampleRate = 48000.0;

    // Listener at origin. Source at small, fixed offset (front-of-head bias)
    // so the +X / -X reflection pair isn't degenerate.
    static constexpr float kSourceX = 0.30f;  // 30 cm in front of listener
    static constexpr float kSourceY = 0.10f;  // 10 cm to the listener's left
    static constexpr float kSourceZ = 0.00f;  // ear-plane

    // Room aspect ratio. Multiplied by `roomSize` to get per-axis half-widths.
    // The room is centred on the origin, so walls sit at ±halfX, ±halfY, ±halfZ.
    static constexpr float kAspectX = 1.20f;
    static constexpr float kAspectY = 1.00f;
    static constexpr float kAspectZ = 0.75f;

    float roomSize    = 8.0f;
    float wallDamping = 0.30f;
    float mix         = 0.50f;

    std::array<Reflection, kNumReflections> reflections {};

    // Shared mono delay line. Each reflection taps it at its own delay.
    std::vector<float> delayBuffer;
    int writeIndex = 0;
};

} // namespace bjf::spatial
