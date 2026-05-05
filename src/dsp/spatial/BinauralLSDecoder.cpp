#include "BinauralLSDecoder.h"

#include <algorithm>
#include <cstddef>

namespace bjf::spatial
{

void BinauralLSDecoder::prepare (double /*sampleRate*/, int /*maxBlockSize*/)
{
    reset();
}

void BinauralLSDecoder::setFilters (const float* filters, int N)
{
    filterLength = N;
    if (N <= 0) return;

    const std::size_t perEar = static_cast<std::size_t> (kNumHoaChannels)
                             * static_cast<std::size_t> (N);

    firL.assign (filters,           filters + perEar);
    firR.assign (filters + perEar,  filters + 2 * perEar);

    delayLines.assign (perEar, 0.0f);
    writeIndices.assign (kNumHoaChannels, 0);
}

void BinauralLSDecoder::reset() noexcept
{
    if (filterLength <= 0) return;
    std::fill (delayLines.begin(), delayLines.end(), 0.0f);
    std::fill (writeIndices.begin(), writeIndices.end(), 0);
}

void BinauralLSDecoder::decodeSample (const float* hoa, float& outL, float& outR) noexcept
{
    if (filterLength <= 0)
    {
        outL = outR = 0.0f;
        return;
    }

    float L = 0.0f;
    float R = 0.0f;
    const int N = filterLength;

    for (int c = 0; c < kNumHoaChannels; ++c)
    {
        const std::size_t base = static_cast<std::size_t> (c)
                               * static_cast<std::size_t> (N);
        float* delay = delayLines.data() + base;

        // Write the new sample. Tap 0 of the FIR aligns with the most-recent
        // input sample — i.e. delay[wi] after this write.
        const int wi = writeIndices[static_cast<std::size_t> (c)];
        delay[wi] = hoa[c];

        const float* hL = firL.data() + base;
        const float* hR = firR.data() + base;
        int idx = wi;
        for (int n = 0; n < N; ++n)
        {
            const float x = delay[idx];
            L += hL[n] * x;
            R += hR[n] * x;
            if (--idx < 0) idx = N - 1;
        }

        writeIndices[static_cast<std::size_t> (c)] = (wi + 1) % N;
    }

    outL = L;
    outR = R;
}

} // namespace bjf::spatial
