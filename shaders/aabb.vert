#version 460 core

layout (location = 0) in vec3 inPos;

uniform mat4 transform;

void main()
{
	gl_Position = transform * vec4(inPos, 1.0f);
}