#pragma once

#include "HOACoefficients.h"

namespace bjf::spatial
{

// Encodes a mono signal into a 16-channel ACN/SN3D 3rd-order HOA bus given
// a single (azimuth, elevation) position. Coefficients are recomputed only
// when the position changes — at audio rate that's still cheap (16 SH evals
// per change), and the encoder consumes negligible CPU per sample.
//
// Position smoothing belongs upstream (modulation matrix in step 7+); this
// class just translates the current position into per-sample HOA channels.
class HOAEncoder
{
public:
    HOAEncoder() noexcept;

    // Set the current source position. Recomputes the coefficient vector;
    // call from the audio thread before processing each block.
    void setPosition (float azimuthRad, float elevationRad) noexcept;

    // Per-sample encode: writes mono * coef[i] into out[i] for i in [0, 16).
    void encodeSample (float monoIn, float* out) const noexcept;

    const float* getCoefficients() const noexcept { return coefs; }

private:
    float coefs[kNumHoaChannels] {};
};

} // namespace bjf::spatial
