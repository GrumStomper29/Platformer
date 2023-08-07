#version 460 core

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inNorm;
layout (location = 2) in vec2 inTex;

uniform mat4 transform;

layout (location = 0) out vec3 outNorm;
layout (location = 1) out vec2 outTex;

void main()
{
	gl_Position = transform * vec4(inPos, 1.0f);
	outNorm = inNorm;
	outTex = inTex;
}