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

int c_width = 1600;
int c_height = 900;

// clang-format off
std::vector<float> vertices
{
    0.0f, 0.0f, 0.0f,   0.0f, 0.0f, 1.0f, 
	1.0f, 0.0f, 0.0f,   0.0f, 0.0f, 1.0f, 
	0.0f, 1.0f, 0.0f,   0.0f, 0.0f, 1.0f,

	0.0f, 0.0f, 0.0f,   0.0f, 1.0f, 0.0f, 
	1.0f, 0.0f, 0.0f,   0.0f, 1.0f, 0.0f, 
	0.0f, 0.0f, 1.0f,   0.0f, 1.0f, 0.0f
};

std::vector<int> indices
{
	0, 1, 2, 3, 4, 5
};
// clang-format on

Camera g_camera;
double g_mousePosX = 0.0;
double g_mousePosY = 0.0;
float g_mouseSensitivity = 0.1f;

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

    transformation.rotate(Transformation::UP, -static_cast<float>(mouseDeltaX) * deltaTime * g_mouseSensitivity);
    transformation.rotate(Transformation::LEFT, static_cast<float>(mouseDeltaY) * deltaTime * g_mouseSensitivity);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    {
        transformation.move(transformation.getForward() * deltaTime);
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    {
        transformation.move(-transformation.getForward() * deltaTime);
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    {
        transformation.move(transformation.getLeft() * deltaTime);
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    {
        transformation.move(-transformation.getLeft() * deltaTime);
    }
    if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
    {
        transformation.position = {0.0f, 0.5f, 3.0f};
        transformation.rotation = {0.0f, 0.0f, 0.0f};
    }
    transformation.updateModelMatrix();
}

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(c_width, c_height, "GL", NULL, NULL);
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

    glEnable(GL_DEPTH_TEST);
    glViewport(0, 0, c_width, c_height);

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
    shader.createProgram({"../shaders/shader.vert", "../shaders/shader.frag"});
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
