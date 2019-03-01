#include "shader.h"
#include "camera.h"
#include "transformation.h"
#include "functions.h"
#include "constants.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>
#include <glm/glm.hpp>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <vector>
#include <random>
#include <algorithm>
#include <cstdlib>

Camera g_camera;
double g_mousePosX = 0.0;
double g_mousePosY = 0.0;

std::random_device g_randomDevice;
std::mt19937 g_rng(c_randomSeed ? g_randomDevice() : c_seed);

void processInput(GLFWwindow* window, float deltaTime)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, true);
    }

    Transformation& transformation = g_camera.getTransformation();

    double currentMousePosX;
    double currentMousePosY;
    glfwGetCursorPos(window, &currentMousePosX, &currentMousePosY);
    double mouseDeltaX = (currentMousePosX - g_mousePosX);
    double mouseDeltaY = (currentMousePosY - g_mousePosY);
    g_mousePosX = currentMousePosX;
    g_mousePosY = currentMousePosY;

    float rotationSpeed = deltaTime * c_mouseSensitivity;
    transformation.rotate(Transformation::UP, -static_cast<float>(mouseDeltaX) * rotationSpeed);
    transformation.rotate(Transformation::LEFT, static_cast<float>(mouseDeltaY) * rotationSpeed);

    float movementSpeed = c_movementSpeedMultiplier * deltaTime;
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    {
        transformation.move(transformation.getForward() * movementSpeed);
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    {
        transformation.move(-transformation.getForward() * movementSpeed);
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    {
        transformation.move(transformation.getLeft() * movementSpeed);
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    {
        transformation.move(-transformation.getLeft() * movementSpeed);
    }
    if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
    {
        transformation.position = {2.0f, 2.0f, 6.0f};
        transformation.rotation = {0.0f, 0.0f, 0.0f};
    }
    transformation.updateModelMatrix();
}

float createBump(std::vector<std::vector<float>>& world, int centerX, int centerY, float bumpHeightMultiplier, float deviation)
{
    int limit = static_cast<int>(deviation * c_standardDeviationArea);
    int minX = std::max(0, centerX - limit);
    int maxX = std::min(c_worldWidth, centerX + limit);
    int minY = std::max(0, centerY - limit);
    int maxY = std::min(c_worldHeight, centerY + limit);

    for (int y = minY; y <= maxY; ++y)
    {
        for (int x = minX; x <= maxX; ++x)
        {
            float d = distance(centerX, centerY, x, y);
            float height = normalDistribution(0.0f, deviation, d) * bumpHeightMultiplier;
            world[x][y] += height;
        }
    }
    return world[centerX][centerY];
}

bool getBumpPosition(int iteration, int centerX, int centerY, int& x, int& y, float a, int exp)
{
    float relativeY = 0.0f;
    relativeY = parabola(a, static_cast<float>(iteration), exp);

    int newX = centerX + iteration;
    int newY = centerY + static_cast<int>(relativeY);
    int widthLimit = c_worldWidth - c_mountainEdgeMargin;
    int heightLimit = c_worldHeight - c_mountainEdgeMargin;
    bool insideLimits = newX >= 0 && newX <= widthLimit && newY >= 0 && newY <= heightLimit;

    x = std::min(std::max(0, newX), widthLimit);
    y = std::min(std::max(0, newY), heightLimit);
    return insideLimits;
}

void createMountain(std::vector<std::vector<float>>& world)
{
    std::uniform_int_distribution<int> randomX(0, c_worldWidth);
    std::uniform_int_distribution<int> randomY(0, c_worldHeight);
    std::uniform_int_distribution<int> randomMountainLength(c_minMountainLength, c_maxMountainLength);
    std::uniform_real_distribution<float> randomBumpHeightMultiplier(c_minHeightMultiplier, c_maxHeightMultiplier);
    std::uniform_real_distribution<float> randomDeviation(c_minBumpDeviation, c_maxBumpDeviation);
    std::uniform_real_distribution<float> randomCoefficient(c_minParabolaCoefficient, c_maxParabolaCoefficient);
    std::uniform_int_distribution<int> randomExponent(c_minParabolaExponent, c_maxParabolaExponent);

    int mountainLength = randomMountainLength(g_rng);
    std::uniform_int_distribution<int> randomIterationStart(-mountainLength / 2, mountainLength / 2);

    float bumpHeightBaseMultiplier = randomBumpHeightMultiplier(g_rng);

    int centerX = randomX(g_rng);
    int centerY = randomY(g_rng);
    int iterationStart = randomIterationStart(g_rng);
    int x = 0;
    int y = 0;
    float a = randomCoefficient(g_rng);
    int exp = randomExponent(g_rng);

    for (int i = 0; i < mountainLength; i += c_bumpDensity)
    {
        if (getBumpPosition(iterationStart + i, centerX, centerY, x, y, a, exp))
        {
            float sinStep = std::sin(static_cast<float>(i) * c_mountainWaveLength);
            sinStep = (sinStep + 2.0f) / 2.0f;
            float bumpHeightMultiplier = bumpHeightBaseMultiplier * sinStep;
            float deviation = randomDeviation(g_rng);
            createBump(world, x, y, bumpHeightMultiplier, deviation);
        }
    }
}

int getPitPosition(int startHeight, int endHeight, int x)
{
    float yt = slope(divide(x, c_worldWidth), c_riverSlopeSteepness);
    return interpolate(startHeight, endHeight, yt);
}

void createRiver(std::vector<std::vector<float>>& world)
{
    int startHeight = c_riverEndPointMargin;
    int endHeight = c_worldHeight - c_riverEndPointMargin;
    float currentDepth = 0.0f;

    for (int x = 0; x <= c_worldWidth; x += c_riverPitDensity)
    {
        int y = getPitPosition(startHeight, endHeight, x);
        currentDepth = world[x][y];
        while (currentDepth > -c_riverDepth)
        {
            currentDepth = createBump(world, x, y, -2.0f, c_riverDeviation);
        }
    }
}

void generateWorld(std::vector<std::vector<float>>& world)
{
    world.resize(c_worldHeight + 1);
    for (std::vector<float>& width : world)
    {
        width.resize(c_worldWidth + 1);
    }

    for (int i = 0; i < c_numMountains; ++i)
    {
        createMountain(world);
    }

    createRiver(world);
}

void addVertex(std::vector<float>& vertices, const glm::vec3& a, const glm::vec3& b, const glm::vec3& c)
{
    auto addVec3 = [](std::vector<float>& vec, const glm::vec3& v) {
        vec.push_back(v.x);
        vec.push_back(v.y);
        vec.push_back(v.z);
    };

    addVec3(vertices, a);
    addVec3(vertices, glm::normalize(glm::cross(c - a, b - a)));

    addVec3(vertices, b);
    addVec3(vertices, glm::normalize(glm::cross(a - b, c - b)));

    addVec3(vertices, c);
    addVec3(vertices, glm::normalize(glm::cross(b - c, a - c)));
}

void generateMesh(const std::vector<std::vector<float>>& world, std::vector<int>& indices, std::vector<float>& vertices)
{
    const int verticesPerSquare = 6;
    int count = c_worldWidth * c_worldHeight * verticesPerSquare;

    indices.reserve(count);
    vertices.reserve(count);

    for (int h = 0; h < c_worldWidth; ++h)
    {
        for (int w = 0; w < c_worldHeight; ++w)
        {
            float x = static_cast<float>(w) * c_worldScale;
            float z = static_cast<float>(h) * c_worldScale;

            // First triangle
            glm::vec3 a(x, world[h][w], z);
            glm::vec3 b(x + c_worldScale, world[h][w + 1], z);
            glm::vec3 c(x, world[h + 1][w], z + c_worldScale);

            addVertex(vertices, a, b, c);

            // Second triangle
            a = glm::vec3(x + c_worldScale, world[h][w + 1], z);
            b = glm::vec3(x + c_worldScale, world[h + 1][w + 1], z + c_worldScale);
            c = glm::vec3(x, world[h + 1][w], z + c_worldScale);

            addVertex(vertices, a, b, c);
        }
    }

    for (int i = 0; i < count; ++i)
    {
        indices.push_back(i);
    }
}

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(c_screenWidth, c_screenHeight, "GL", NULL, NULL);
    if (window == nullptr)
    {
        std::cout << "Failed to create GLFW window\n";
        glfwTerminate();
        return 1;
    }
    glfwMakeContextCurrent(window);
    glfwSetWindowPos(window, 1200, 300);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD\n";
        return 2;
    }

    std::vector<std::vector<float>> world;
    generateWorld(world);

    std::vector<int> indices;
    std::vector<float> vertices;
    generateMesh(world, indices, vertices);

    glEnable(GL_DEPTH_TEST);
    glViewport(0, 0, c_screenWidth, c_screenHeight);

    GLuint vertexArray;
    glGenVertexArrays(1, &vertexArray);
    glBindVertexArray(vertexArray);

    GLuint indexBuffer;
    glGenBuffers(1, &indexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(float) * indices.size(), indices.data(), GL_STATIC_DRAW);

    GLuint vertexBuffer;
    glGenBuffers(1, &vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(int) * vertices.size(), vertices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    Shader shader;
    shader.createProgram({shaderPath + "shader.vert", shaderPath + "shader.frag"});
    glUseProgram(shader.getProgram());

    double lastTime = 0.0;

    while (!glfwWindowShouldClose(window))
    {
        double currentTime = glfwGetTime();
        double deltaTime = currentTime - lastTime;
        lastTime = currentTime;

        processInput(window, static_cast<float>(deltaTime));
        glClearColor(0.0f, 0.0f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        g_camera.updateViewMatrix();
        glm::mat4 mvp = g_camera.getProjectionMatrix() * g_camera.getViewMatrix();
        glUniformMatrix4fv(0, 1, GL_FALSE, &mvp[0][0]);

        glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indices.size()), GL_UNSIGNED_INT, 0);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &vertexArray);
    glDeleteBuffers(1, &indexBuffer);
    glDeleteBuffers(1, &vertexBuffer);

    glfwTerminate();
    return 0;
}
