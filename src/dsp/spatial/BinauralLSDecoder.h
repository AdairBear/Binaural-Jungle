#pragma once

#include <vector>

#include "HOACoefficients.h"

namespace bjf::spatial
{

// Real-time HOA → binaural decoder. Holds the 16×2 time-domain FIRs produced
// by solveLSDecoder() and applies them per sample using one circular delay
// line per HOA channel. Per-sample cost: 32·N FMA. Real partitioned-
// convolution lands in step 9 once N ≫ 256 makes direct convolution costly;
// at typical N=128–256 from MIT KEMAR, direct is fine.
class BinauralLSDecoder
{
public:
    void prepare (double sampleRate, int maxBlockSize);

    // Replace the FIR set with new filters. `filters` is the flat layout
    // produced by DecodeResult::filters (ear-major). Allocates only here.
    void setFilters (const float* filters, int filterLength);

    void reset() noexcept;

    bool isReady() const noexcept       { return filterLength > 0; }
    int  getFilterLength() const noexcept { return filterLength; }

    // Per-sample apply. Caller passes the 16-channel HOA bus and receives L/R.
    void decodeSample (const float* hoa16, float& outL, float& outR) noexcept;

private:
    int filterLength = 0;

    // 16 × N delay lines, one per HOA channel.
    std::vector<float> delayLines;
    std::vector<int>   writeIndices;

    // L and R FIRs: 16 × N each, channel-major.
    std::vector<float> firL;
    std::vector<float> firR;
};

} // namespace bjf::spatial
