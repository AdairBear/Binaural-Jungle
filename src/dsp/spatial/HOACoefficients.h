#pragma once

namespace bjf::spatial
{

// 3rd-order Higher-Order Ambisonic encoding parameters.
//   ACN channel ordering: index n = l*(l+1) + m for spherical-harmonic index (l, m).
//   SN3D normalisation:   real spherical harmonics scaled to unit max-amplitude
//                         per order (m != 0 includes a sqrt(2) factor relative
//                         to N3D — this is the AmbiX / Y3D convention).
constexpr int kOrder           = 3;
constexpr int kNumHoaChannels  = (kOrder + 1) * (kOrder + 1); // = 16

// Conventions for (azimuth, elevation), in radians:
//   azimuth   = 0 → directly in front of listener (+x axis)
//   azimuth   = +π/2 → to the listener's left (+y axis)
//   elevation = 0 → in the horizontal plane
//   elevation = +π/2 → directly above listener (+z axis)
//
// Fills `y` with the 16 real SN3D-normalised spherical-harmonic coefficients
// evaluated at (az, el). Output buffer must be sized ≥ kNumHoaChannels.
void computeAcnSn3d (float azimuthRad, float elevationRad,
                     float* y) noexcept;

} // namespace bjf::spatial
