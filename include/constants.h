#pragma once

#include <string>

const float e = 2.7182818284f;
const float pi = 3.14159265f;

const int c_screenWidth = 1600;
const int c_screenHeight = 900;

const std::string shaderPath = SHADER_PATH;

const float c_mouseSensitivity = 0.1f;
const float c_movementSpeedMultiplier = 3.0f;

// World
const int c_worldWidth = 500;
const int c_worldHeight = 500;
const float c_worldScale = 0.01f;

// Mountains
const int c_numMountains = 5;
const int c_minMountainLength = 200;
const int c_maxMountainLength = 300;
const float c_minHeightMultiplier = 40.0f;
const float c_maxHeightMultiplier = 50.0f;
const float c_minBumpDeviation = 11.0f;
const float c_maxBumpDeviation = 17.0f;
const float c_mountainWaveLength = 0.1f;
const int c_bumpDensity = 3;
const float c_standardDeviationArea = 4.0f;

// River
const float c_riverDepth = 0.2f;
const float c_riverDeviation = 15.0f;
const int c_riverPitDensity = 10;
const float c_riverSlopeSteepness = 1.5f;
const int c_riverEndPointMargin = 30;