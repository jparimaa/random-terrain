#pragma once

#include "constants.h"

#include <glm/glm.hpp>

#include <cmath>
#include <algorithm>
#include <vector>

float normalDistribution(float mean, float standardDeviation, float x)
{
    float variance = std::pow(standardDeviation, 2.0f);
    float exponent = -(std::pow((x - mean), 2.0f)) / (2.0f * variance);
    return pow(e, exponent) / (standardDeviation * std::sqrt(2.0f * pi * variance));
}
