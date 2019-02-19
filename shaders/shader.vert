#version 450 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;

layout (location = 0) uniform mat4 MVP;

layout (location = 0) out vec3 outNormal;

void main()
{
	gl_Position = MVP * vec4(position, 1.0);
	outNormal = normal;
}