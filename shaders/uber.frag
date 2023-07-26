#version 450 core

layout (location = 0) in vec2 inTex;

uniform bool      textured;
uniform sampler2D inTexture;
uniform vec3      color;

out vec4 outColor;

void main()
{
	if (textured)
	{
		outColor = texture(inTexture, inTex);
	}
	else
	{
		outColor = vec4(color, 1.0f);
	}
}