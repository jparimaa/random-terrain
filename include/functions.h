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

float distance(int x1, int y1, int x2, int y2)
{
    float xDistance = static_cast<float>(x1 - x2);
    float yDistance = static_cast<float>(y1 - y2);
    return std::sqrt(std::pow(xDistance, 2.0f) + std::pow(yDistance, 2.0f));
}

float parabola(float a, float x, float exponent)
{
    return a * std::pow(x, exponent);
}