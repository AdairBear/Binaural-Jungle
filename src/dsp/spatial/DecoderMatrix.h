#pragma once

#include <vector>

namespace bjf::spatial
{

// Offline HOA→binaural decoder solve. Computes time-domain FIR filters such
// that, for any direction encoded with the standard ACN/SN3D HOA encoder, the
// pair of (filter · hoa_channel) sums equals the measured HRIR at that
// direction in the least-squares sense:
//
//   for each ear ∈ {0, 1}, for each tap n ∈ [0, N):
//       Filter[c, ear, n] = (pinv(Y) · HRIR[:, ear, n])[c]
//
// where Y is the (M directions × 16 SH) ambisonic encoding matrix evaluated
// at the SOFA grid, and pinv(Y) is the Moore–Penrose pseudoinverse computed
// via the normal equations + Cholesky (M ≥ 16 always for any usable SOFA
// grid, so YᵀY is symmetric positive-definite). All work happens in the time
// domain — no FFT — because the LS objective is linear in the time samples.
//
// True MagLS magnitude-iterative refinement above the spatial-aliasing
// frequency is a follow-up: in the LS-only regime the decoder is exact at
// the encoded grid and degrades in localisation precision above f_alias ≈
// 1.9 kHz for order 3 + 9 cm head, but still produces a clearly externalised
// binaural image driven by real measured HRIRs.
struct DecodeResult
{
    bool success        = false;
    int  numHoaChannels = 0;
    int  filterLength   = 0;
    int  numEars        = 0;

    // Flat layout: filters[ear * (numHoaChannels * filterLength)
    //                    + channel * filterLength
    //                    + tap]
    std::vector<float> filters;

    const float* getFilter (int ear, int channel) const noexcept;
};

// directions: M × 2 floats, [m*2+0]=azRad, [m*2+1]=elRad   (matches SOFALoader)
// hrirs:      M · numEars · filterLength floats, row-major [m][ear][n]
// numEars must be 2; numDirections must be ≥ 16 (3rd-order channel count).
DecodeResult solveLSDecoder (const float* directions,
                             const float* hrirs,
                             int numDirections,
                             int numEars,
                             int filterLength);

} // namespace bjf::spatial
