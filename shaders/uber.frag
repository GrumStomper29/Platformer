#version 450 core

layout (location = 0) in vec3 inNorm;
layout (location = 1) in vec2 inTex;

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

	const float ambient = 0.7f;

	const vec3 lightDir = normalize(const vec3(-1.0f, 2.0f, 1.5f));
	float diffuse = max(dot(inNorm, lightDir), 0);

	outColor = vec4(outColor.rgb * (diffuse + ambient), outColor.a);
}