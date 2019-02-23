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

const int c_screenWidth = 1600;
const int c_screenHeight = 900;

Camera g_camera;
double g_mousePosX = 0.0;
double g_mousePosY = 0.0;
const float c_mouseSensitivity = 0.1f;
const float c_movementSpeedMultiplier = 3.0f;

const int c_worldWidth = 25;
const int c_worldHeight = 25;

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

void generateWorld(std::vector<std::vector<float>>& world)
{
    world.resize(c_worldHeight + 1);
    for (std::vector<float>& width : world)
    {
        width.resize(c_worldWidth + 1);
    }

    std::mt19937 rng(g_randomDevice());
    std::uniform_int_distribution<int> randomWidth(0, c_worldWidth);
    std::uniform_int_distribution<int> randomHeight(0, c_worldHeight);

    for (int i = 0; i < 50; ++i)
    {
        int rw = randomWidth(rng);
        int rh = randomWidth(rng);
        world[rh][rw] += (i % 2 == 0 ? -1.0f : 1.0f);
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
            float x = static_cast<float>(w);
            float z = static_cast<float>(h);

            // First triangle
            glm::vec3 a(x, world[h][w], z);
            glm::vec3 b(x + 1.0f, world[h][w + 1], z);
            glm::vec3 c(x, world[h + 1][w], z + 1.0f);

            addVertex(vertices, a, b, c);

            // Second triangle
            a = glm::vec3(x + 1.0f, world[h][w + 1], z);
            b = glm::vec3(x + 1.0f, world[h + 1][w + 1], z + 1.0f);
            c = glm::vec3(x, world[h + 1][w], z + 1.0f);

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
