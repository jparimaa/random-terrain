#pragma once

#include "transformation.h"
#include <glm/glm.hpp>

class Camera
{
public:
    explicit Camera();
    ~Camera(){};

    const glm::mat4x4& updateViewMatrix();
    const glm::mat4x4& updateProjectionMatrix();

    const glm::mat4x4& getViewMatrix();
    const glm::mat4x4& getProjectionMatrix();

    Transformation& getTransformation();

private:
    float FOV = 45.0f;
    float ratio = 1.33333f;
    float nearClipDistance = 0.1f;
    float farClipDistance = 100.0f;

    glm::mat4 viewMatrix;
    glm::mat4 projectionMatrix;

    Transformation transformation;
};
