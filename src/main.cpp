#include "shader.h"
#include "camera.h"
#include "transformation.h"

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

const int c_screenWidth = 1600;
const int c_screenHeight = 900;

Camera g_camera;
double g_mousePosX = 0.0;
double g_mousePosY = 0.0;
const float c_mouseSensitivity = 0.1f;
const float c_movementSpeedMultiplier = 3.0f;

const int c_worldWidth = 500;
const int c_worldHeight = 500;
const float c_worldScale = 0.01f;
const int c_numBumps = 15;
const int c_maxBumpHeight = 100;

const std::string shaderPath = SHADER_PATH;

std::random_device g_randomDevice;

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
        transformation.position = {5.0f, 3.0f, 15.0f};
        transformation.rotation = {0.0f, 0.0f, 0.0f};
    }
    transformation.updateModelMatrix();
}

void createBump(std::vector<std::vector<float>>& world, int centerX, int centerY, float bumpHeight, int numSteps)
{
    int numStepsForLimits = numSteps - 1;
    int minX = std::max(0, centerX - numStepsForLimits);
    int maxX = std::min(c_worldWidth, centerX + numStepsForLimits);
    int minY = std::max(0, centerY - numStepsForLimits);
    int maxY = std::min(c_worldHeight, centerY + numStepsForLimits);
    float bumpStep = bumpHeight / static_cast<float>(numSteps);

    std::mt19937 rng(g_randomDevice());
    std::uniform_int_distribution<int> randomNoise(1, 10);

    for (int y = minY; y <= maxY; ++y)
    {
        for (int x = minX; x <= maxX; ++x)
        {
            int distance = std::max(std::abs(y - centerY), std::abs(x - centerX));
            float height = bumpHeight - static_cast<float>(distance) * bumpStep;
            float noise = 1.0f; //1.1f / static_cast<float>(randomNoise(rng));
            world[x][y] = std::max(height + (noise * bumpStep), world[x][y]);
        }
    }
}

void getBumpPosition(int iteration, int centerX, int centerY, int& x, int& y)
{
    float relativeY = 0.01f * std::pow(static_cast<float>(iteration), 2.0f);
    x = std::min(std::max(0, centerX + iteration), c_worldWidth);
    y = std::min(std::max(0, centerY + static_cast<int>(relativeY)), c_worldHeight);
}

void generateWorld(std::vector<std::vector<float>>& world)
{
    world.resize(c_worldHeight + 1);
    for (std::vector<float>& width : world)
    {
        width.resize(c_worldWidth + 1);
    }

    std::mt19937 rng(g_randomDevice());
    std::uniform_int_distribution<int> randomX(50, c_worldWidth - 50);
    std::uniform_int_distribution<int> randomY(50, c_worldHeight - 50);
    std::uniform_int_distribution<int> randomBumpHeight(0, c_maxBumpHeight);
    std::uniform_int_distribution<int> randomMountainLength(25, 50);

    int mountainLength = randomMountainLength(rng);
    int centerX = 250; //randomX(rng);
    int centerY = 250; //randomY(rng);
    int x = 0;
    int y = 0;

    for (int i = -mountainLength; i < mountainLength; ++i)
    {
        getBumpPosition(i, centerX, centerY, x, y);
        int bumpHeight = 25; //randomBumpHeight(rng);
        float scaledBumpHeight = static_cast<float>(bumpHeight) * c_worldScale;
        int minBumpSteps = static_cast<int>(std::tan(glm::radians(30.0f)) * bumpHeight);
        int maxBumpSteps = static_cast<int>(std::tan(glm::radians(45.0f)) * bumpHeight);
        std::uniform_int_distribution<int> randomBumpSteps(minBumpSteps, maxBumpSteps);
        int numSteps = randomBumpSteps(rng);
        createBump(world, x, y, scaledBumpHeight, numSteps);
    }
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
