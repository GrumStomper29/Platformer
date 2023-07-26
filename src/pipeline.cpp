#include "pipeline.hpp"

#include "glad/glad.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <utility>

Pipeline::Pipeline(const char* vertexShaderPath, const char* fragmentShaderPath)
{
	GLuint vertexShader{ loadShader(vertexShaderPath, GL_VERTEX_SHADER) };
	GLuint fragmentShader{ loadShader(fragmentShaderPath, GL_FRAGMENT_SHADER) };

	m_shaderProgram = glCreateProgram();
	glAttachShader(m_shaderProgram, vertexShader);
	glAttachShader(m_shaderProgram, fragmentShader);
	glLinkProgram(m_shaderProgram);

	GLint success{};
	glGetProgramiv(m_shaderProgram, GL_LINK_STATUS, &success);
	if (!success)
	{
		char log[1024]{};
		glGetProgramInfoLog(m_shaderProgram, 1024, nullptr, log);
		std::cerr << "ENGINE, ERROR, MEDIUM, OpenGL shader program linkage raised info log:\n"
			<< log << '\n';
	}

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	m_shouldDestruct = true;
}

Pipeline::Pipeline(Pipeline&& p)
{
	move(std::move(p));
}

Pipeline& Pipeline::operator=(Pipeline&& p)
{
	destruct();
	move(std::move(p));

	return *this;
}

Pipeline::~Pipeline()
{
	destruct();
}

void Pipeline::bind()
{
	glUseProgram(m_shaderProgram);
}

void Pipeline::move(Pipeline&& p)
{
	m_shaderProgram = p.m_shaderProgram;

	p.m_shouldDestruct = false;
}

void Pipeline::destruct()
{
	if (m_shouldDestruct)
	{
		glDeleteProgram(m_shaderProgram);
	}
}

GLuint Pipeline::loadShader(const char* path, GLenum type)
{
	std::ifstream shaderStream{};
	shaderStream.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	
	std::string shaderStr{};

	try
	{
		shaderStream.open(path);

		std::stringstream shaderStringStream{};
		shaderStringStream << shaderStream.rdbuf();

		shaderStream.close();

		shaderStr = shaderStringStream.str();
	}
	catch (std::ifstream::failure& e)
	{
		std::cerr << "ENGINE, ERROR, MEDIUM, loadShader(" << path << ", GLenum type)\n";
		std::cerr << e.what() << '\n';
	}

	const char* shaderCStr{ shaderStr.c_str() };

	GLuint shader{ glCreateShader(type) };
	glShaderSource(shader, 1, &shaderCStr, nullptr);
	glCompileShader(shader);

	GLint success{};
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		char log[1024]{};
		glGetShaderInfoLog(shader, 1024, nullptr, log);
		std::cerr << "ENGINE, ERROR, MEDIUM, OpenGL shader compilation raised info log:\n"
			<< log << '\n';
	}

	return shader;
}