#pragma once

#include "HOACoefficients.h"

namespace bjf::spatial
{

// Step-5 prototype HOA → stereo decoder. NOT MagLS — that lands in step 6+
// once SOFA HRIRs are loaded and the offline decoder-matrix solve runs.
// This is a sampling decoder with two virtual loudspeakers placed at the
// listener's left and right ears (azimuth ±π/2, elevation 0). The two
// resulting columns of the decoder matrix are orthogonal at order 3, so a
// source encoded at the left ear position decodes to L=1.0 / R=0.0 cleanly,
// which is enough to *hear* a voice rotate around the head and verify the
// encode→decode plumbing end-to-end.
//
// Known limitations of the prototype (all addressed by step 6+ MagLS):
//   - Front (az=0) and back (az=π) are indistinguishable — both decode to
//     L=R=0.125. Real binaural relies on HRTF spectral cues for front/back
//     disambiguation, which a 2-speaker sampling decoder cannot produce.
//   - Near-front sources (|az| ≲ 20°) suffer from order-3 sidelobes that
//     can briefly invert L/R dominance.
//   - No interaural time difference, no head shadow — only intensity panning.
// The test goal at step 5 is "hear motion in the lateral hemisphere", not
// "perceive a fully externalised binaural image".
//
// Decoder formula:
//     y_L = scale · Σ_n ( leftCoefs[n]  · hoa[n] )
//     y_R = scale · Σ_n ( rightCoefs[n] · hoa[n] )
//
// `scale` is chosen so that a unity-amplitude source aimed straight at the
// left ear produces y_L = 1.0; analytically that's 1 / |leftCoefs|² = 1/4
// at order 3.
class SimpleStereoDecoder
{
public:
    SimpleStereoDecoder() noexcept;

    void decodeSample (const float* hoa16, float& outL, float& outR) const noexcept;

    const float* getLeftCoefficients()  const noexcept { return leftCoefs;  }
    const float* getRightCoefficients() const noexcept { return rightCoefs; }
    float        getScale()             const noexcept { return scale;      }

private:
    float leftCoefs  [kNumHoaChannels] {};
    float rightCoefs [kNumHoaChannels] {};
    float scale = 0.25f; // 1 / |Y(±π/2, 0)|² at order 3
};

} // namespace bjf::spatial
