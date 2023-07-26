#pragma once

#include "glad/glad.h"

class Pipeline
{
public:

	Pipeline() = default;
	Pipeline(const char* vertexShaderPath, const char* fragmentShaderPath);

	Pipeline(const Pipeline&) = delete;
	Pipeline& operator=(const Pipeline&) = delete;

	Pipeline(Pipeline&& p);
	Pipeline& operator=(Pipeline&& p);

	~Pipeline();

	void bind();

	GLuint shaderProgram() const
	{
		return m_shaderProgram;
	}

private:

	void move(Pipeline&& p);
	void destruct();

	bool m_shouldDestruct{ false };

	GLuint loadShader(const char* path, GLenum type);

	GLuint m_shaderProgram{};

};