#include "DecoderMatrix.h"

#include "HOACoefficients.h"

#include <algorithm>
#include <cmath>
#include <cstddef>

namespace bjf::spatial
{

const float* DecodeResult::getFilter (int ear, int channel) const noexcept
{
    if (ear < 0 || ear >= numEars || channel < 0 || channel >= numHoaChannels)
        return nullptr;
    const std::size_t perEar = static_cast<std::size_t> (numHoaChannels)
                             * static_cast<std::size_t> (filterLength);
    const std::size_t offset = static_cast<std::size_t> (ear) * perEar
                             + static_cast<std::size_t> (channel)
                                 * static_cast<std::size_t> (filterLength);
    return filters.data() + offset;
}

namespace
{
// In-place Cholesky factorisation of an n×n symmetric PSD matrix in row-major
// form. On return, the lower triangle of `a` holds L such that A = L · Lᵀ.
// Returns false if A is not positive-definite (within tolerance).
bool cholesky (float* a, int n)
{
    for (int i = 0; i < n; ++i)
    {
        float diag = a[i * n + i];
        for (int k = 0; k < i; ++k)
            diag -= a[i * n + k] * a[i * n + k];
        if (diag <= 1.0e-12f)
            return false;
        a[i * n + i] = std::sqrt (diag);

        for (int j = i + 1; j < n; ++j)
        {
            float s = a[j * n + i];
            for (int k = 0; k < i; ++k)
                s -= a[j * n + k] * a[i * n + k];
            a[j * n + i] = s / a[i * n + i];
        }
    }
    return true;
}

// Solve A · x = b in-place using a precomputed Cholesky factor (A = L · Lᵀ).
// `L` must be the lower triangle of `a` produced by cholesky() above. `x`
// holds the right-hand side on entry and the solution on return.
void choleskySolve (const float* L, int n, float* x)
{
    // Forward: L · y = b
    for (int i = 0; i < n; ++i)
    {
        float s = x[i];
        for (int k = 0; k < i; ++k)
            s -= L[i * n + k] * x[k];
        x[i] = s / L[i * n + i];
    }
    // Backward: Lᵀ · x = y
    for (int i = n - 1; i >= 0; --i)
    {
        float s = x[i];
        for (int k = i + 1; k < n; ++k)
            s -= L[k * n + i] * x[k];
        x[i] = s / L[i * n + i];
    }
}
} // namespace

DecodeResult solveLSDecoder (const float* directions,
                             const float* hrirs,
                             int numDirections,
                             int numEars,
                             int filterLength)
{
    DecodeResult result;
    if (directions == nullptr || hrirs == nullptr
        || numDirections < kNumHoaChannels
        || numEars       <= 0
        || filterLength  <= 0)
    {
        return result;
    }

    // ── Y matrix: M × 16, each row is the SH coefficients at direction m. ──
    std::vector<float> Y (static_cast<std::size_t> (numDirections)
                          * static_cast<std::size_t> (kNumHoaChannels));
    for (int m = 0; m < numDirections; ++m)
    {
        const float az = directions[m * 2 + 0];
        const float el = directions[m * 2 + 1];
        computeAcnSn3d (az, el, &Y[static_cast<std::size_t> (m)
                                    * kNumHoaChannels]);
    }

    // ── YᵀY: 16 × 16, symmetric PSD. Compute lower triangle, mirror. ──
    constexpr std::size_t C = kNumHoaChannels;
    std::vector<float> YtY (C * C, 0.0f);
    for (std::size_t i = 0; i < C; ++i)
    {
        for (std::size_t j = 0; j <= i; ++j)
        {
            float s = 0.0f;
            for (int m = 0; m < numDirections; ++m)
                s += Y[static_cast<std::size_t> (m) * C + i]
                   * Y[static_cast<std::size_t> (m) * C + j];
            YtY[i * C + j] = s;
            YtY[j * C + i] = s;
        }
    }

    if (! cholesky (YtY.data(), kNumHoaChannels))
        return result;

    // ── Output filters: numEars · 16 · N ──
    result.numHoaChannels = kNumHoaChannels;
    result.filterLength   = filterLength;
    result.numEars        = numEars;
    result.filters.assign (static_cast<std::size_t> (numEars)
                            * static_cast<std::size_t> (kNumHoaChannels)
                            * static_cast<std::size_t> (filterLength),
                            0.0f);

    // For each ear and each tap n: rhs[c] = Σ_m Y[m, c] · HRIR[m, ear, n].
    // Then Cholesky-solve YᵀY · x = rhs to get filter[c, ear, n] = x[c].
    std::vector<float> rhs (kNumHoaChannels);
    const std::size_t perEar = static_cast<std::size_t> (kNumHoaChannels)
                             * static_cast<std::size_t> (filterLength);

    for (int ear = 0; ear < numEars; ++ear)
    {
        for (int n = 0; n < filterLength; ++n)
        {
            std::fill (rhs.begin(), rhs.end(), 0.0f);

            for (int m = 0; m < numDirections; ++m)
            {
                const float* yRow = &Y[static_cast<std::size_t> (m) * kNumHoaChannels];
                const float ir =
                    hrirs[(static_cast<std::size_t> (m) * static_cast<std::size_t> (numEars)
                              + static_cast<std::size_t> (ear))
                          * static_cast<std::size_t> (filterLength)
                          + static_cast<std::size_t> (n)];
                for (int c = 0; c < kNumHoaChannels; ++c)
                    rhs[static_cast<std::size_t> (c)] += yRow[c] * ir;
            }

            choleskySolve (YtY.data(), kNumHoaChannels, rhs.data());

            for (int c = 0; c < kNumHoaChannels; ++c)
            {
                const std::size_t idx = static_cast<std::size_t> (ear) * perEar
                                      + static_cast<std::size_t> (c)
                                          * static_cast<std::size_t> (filterLength)
                                      + static_cast<std::size_t> (n);
                result.filters[idx] = rhs[static_cast<std::size_t> (c)];
            }
        }
    }

    result.success = true;
    return result;
}

} // namespace bjf::spatial
