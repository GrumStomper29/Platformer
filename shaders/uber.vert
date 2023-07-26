#version 460 core

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec2 inTex;

uniform mat4 transform;

layout (location = 0) out vec2 outTex;

void main()
{
	gl_Position = transform * vec4(inPos, 1.0f);
	outTex = inTex;
}