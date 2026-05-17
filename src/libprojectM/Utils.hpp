#pragma once

#include <cmath>
#include <string>

namespace libprojectM {
namespace Utils {

inline auto AdjustRateToFps(float perFrameRateAtRefFps, float refFps, float actualFps) -> float
{
    if (actualFps <= 0.0f || refFps <= 0.0f)
    {
        return perFrameRateAtRefFps;
    }
    float const perSecondRate = std::pow(perFrameRateAtRefFps, refFps);
    return std::pow(perSecondRate, 1.0f / actualFps);
}

inline auto AdjustRateToFps(float perFrameRateAtRefFps, double secondsSinceLastFrame) -> float
{
    if (secondsSinceLastFrame <= 0.0)
    {
        return perFrameRateAtRefFps;
    }
    float const perSecondRate = std::pow(perFrameRateAtRefFps, 30.0f);
    return std::pow(perSecondRate, static_cast<float>(secondsSinceLastFrame));
}

auto ToLower(const std::string& str) -> std::string;
auto ToUpper(const std::string& str) -> std::string;

void ToLowerInPlace(std::string& str);
void ToUpperInPlace(std::string& str);

/**
 * @brief Strips C and C++ style comments from source code.
 *
 * Replaces // line comments and block comments with spaces, preserving
 * string length and newline positions so that character offsets remain valid.
 *
 * @param source The source code string to strip comments from.
 * @return A copy of the source with all comment content replaced by spaces.
 */
auto StripComments(const std::string& source) -> std::string;

} // namespace Utils
} // namespace libprojectM
