#pragma once

#include <glad/glad.h>

#include <string>
#include <vector>

class Shader
{
public:
    Shader(){};
    ~Shader();

    bool createProgram(const std::vector<std::string>& files);
    GLuint getProgram() const;

private:
    GLuint program = 0;
    std::vector<GLuint> shaders;

    bool attachShader(const std::string& filename);
    bool compileShader(GLuint shader, const std::string& filename);
    bool linkProgram();
    void deleteShaders();
};
