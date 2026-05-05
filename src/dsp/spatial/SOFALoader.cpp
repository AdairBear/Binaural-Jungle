#include "SOFALoader.h"

#include <mysofa.h>

#include <cmath>
#include <sstream>

namespace bjf::spatial
{

namespace
{
constexpr float kDeg2Rad = 0.017453292519943295f;

const char* errToString (int err)
{
    switch (err)
    {
        case MYSOFA_OK:                                   return "OK";
        case MYSOFA_INTERNAL_ERROR:                       return "internal error";
        case MYSOFA_INVALID_FORMAT:                       return "invalid format";
        case MYSOFA_UNSUPPORTED_FORMAT:                   return "unsupported format";
        case MYSOFA_NO_MEMORY:                            return "out of memory";
        case MYSOFA_READ_ERROR:                           return "read error";
        case MYSOFA_INVALID_ATTRIBUTES:                   return "invalid attributes";
        case MYSOFA_INVALID_DIMENSIONS:                   return "invalid dimensions";
        case MYSOFA_INVALID_DIMENSION_LIST:               return "invalid dimension list";
        case MYSOFA_INVALID_COORDINATE_TYPE:              return "invalid coordinate type";
        case MYSOFA_ONLY_EMITTER_WITH_ECI_SUPPORTED:      return "only ECI emitter supported";
        case MYSOFA_ONLY_DELAYS_WITH_IR_OR_MR_SUPPORTED:  return "only IR/MR delays supported";
        case MYSOFA_ONLY_THE_SAME_SAMPLING_RATE_SUPPORTED:return "non-uniform sample rate";
        case MYSOFA_RECEIVERS_WITH_RCI_SUPPORTED:         return "RCI receivers required";
        case MYSOFA_RECEIVERS_WITH_CARTESIAN_SUPPORTED:   return "cartesian receivers required";
        case MYSOFA_INVALID_RECEIVER_POSITIONS:           return "invalid receiver positions";
        case MYSOFA_ONLY_SOURCES_WITH_MC_SUPPORTED:       return "only MC sources supported";
        default:                                          return "unknown libmysofa error";
    }
}
} // namespace

SOFALoader::LoadStatus SOFALoader::loadFromFile (const std::string& path)
{
    loaded = false;
    directions.clear();
    hrirs.clear();
    numDirections = filterLength = numReceivers = 0;
    sampleRate = 0.0f;

    int err = 0;
    MYSOFA_HRTF* hrtf = mysofa_load (path.c_str(), &err);
    if (hrtf == nullptr || err != MYSOFA_OK)
    {
        if (hrtf != nullptr) mysofa_free (hrtf);
        std::ostringstream msg;
        msg << "mysofa_load failed: " << errToString (err);
        return { false, err, msg.str() };
    }

    if (const auto checkErr = mysofa_check (hrtf); checkErr != MYSOFA_OK)
    {
        mysofa_free (hrtf);
        std::ostringstream msg;
        msg << "mysofa_check failed: " << errToString (checkErr);
        return { false, checkErr, msg.str() };
    }

    if (hrtf->R != 2)
    {
        const auto receivers = hrtf->R;
        mysofa_free (hrtf);
        std::ostringstream msg;
        msg << "expected 2 receivers (binaural), got " << receivers;
        return { false, 0, msg.str() };
    }

    // Pull source positions in spherical (deg-az, deg-el, m-radius) from libmysofa.
    mysofa_tospherical (hrtf);

    numDirections = static_cast<int> (hrtf->M);
    filterLength  = static_cast<int> (hrtf->N);
    numReceivers  = static_cast<int> (hrtf->R);

    if (hrtf->DataSamplingRate.elements >= 1)
        sampleRate = hrtf->DataSamplingRate.values[0];

    // Convert positions (deg → rad), drop radius.
    directions.resize (static_cast<std::size_t> (numDirections) * 2);
    for (int m = 0; m < numDirections; ++m)
    {
        const auto* pos = hrtf->SourcePosition.values + 3 * m;
        directions[static_cast<std::size_t> (m) * 2 + 0] = pos[0] * kDeg2Rad; // azimuth
        directions[static_cast<std::size_t> (m) * 2 + 1] = pos[1] * kDeg2Rad; // elevation
    }

    // Copy HRIRs: layout is already [m][r][n] in DataIR.values.
    const auto totalSamples = static_cast<std::size_t> (numDirections)
                            * static_cast<std::size_t> (numReceivers)
                            * static_cast<std::size_t> (filterLength);
    hrirs.assign (hrtf->DataIR.values, hrtf->DataIR.values + totalSamples);

    mysofa_free (hrtf);

    loaded = true;
    return { true, 0, "OK" };
}

void SOFALoader::getDirection (int m, float& azRad, float& elRad) const noexcept
{
    if (m < 0 || m >= numDirections)
    {
        azRad = 0.0f;
        elRad = 0.0f;
        return;
    }
    const auto idx = static_cast<std::size_t> (m) * 2;
    azRad = directions[idx + 0];
    elRad = directions[idx + 1];
}

const float* SOFALoader::getHRIR (int m, int ear) const noexcept
{
    if (! loaded || m < 0 || m >= numDirections || ear < 0 || ear >= numReceivers)
        return nullptr;
    const auto offset = (static_cast<std::size_t> (m) * static_cast<std::size_t> (numReceivers) + static_cast<std::size_t> (ear))
                      * static_cast<std::size_t> (filterLength);
    return hrirs.data() + offset;
}

} // namespace bjf::spatial
