#version 450 core

layout (location = 0) in vec3 normal;

out vec4 color;

const vec3 lightDir = normalize(vec3(-1.0, -4.0, -1.0));

void main()
{
    float NdotL = max(0.0, dot(normal, -lightDir));
	color.rgb = vec3(1.0) * NdotL;
	color.a = 1.0;
} 