#include "renderer.hpp"

#include "model_load.hpp"

#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include "glm/glm.hpp"

// temp
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#include <cstdint>
#include <iostream>
#include <string>

void Renderer::init()
{
	glfwInit();
	initWindow();
	initOpenGL();

	initPipelines();
}

void Renderer::render(const glm::mat4& transform)
{
	glClearColor(0.5f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	m_uberPipeline.bind();
	glBindVertexArray(m_vertexArray);

	for (const auto& meshInstance : meshInstances)
	{
		for (const auto& primitive : m_meshes[meshInstance.mesh].primitives)
		{
			glUniform1i(glGetUniformLocation(m_uberPipeline.shaderProgram(), "textured"), primitive.material.hasTexture);
			glUniformMatrix4fv(glGetUniformLocation(m_uberPipeline.shaderProgram(), "transform"), 1, GL_FALSE, glm::value_ptr(glm::mat4{ transform * (meshInstance.transform * primitive.transform) }));
			glUniform3fv(glGetUniformLocation(m_uberPipeline.shaderProgram(), "color"), 1, glm::value_ptr(primitive.material.color));

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, primitive.material.texture);

			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, primitive.indexBuffer);

			glDrawElements(GL_TRIANGLES, primitive.indexCount, GL_UNSIGNED_INT, nullptr);
		}
	}

	glfwPollEvents();
	glfwSwapBuffers(m_window);
}

void Renderer::cleanup()
{
	glDeleteVertexArrays(1, &m_vertexArray);
	glDeleteBuffers(1, &m_vertexBuffer);
	for (const auto& mesh : m_meshes)
	{
		for (const auto& primitive : mesh.primitives)
		{
			glDeleteBuffers(1, &primitive.indexBuffer);
			
			if (primitive.material.hasTexture)
			{
				glDeleteTextures(1, &primitive.material.texture);
			}
		}
	}

	glfwDestroyWindow(m_window);
	glfwTerminate();
}

void Renderer::loadModel(const std::string& path)
{
	ModelLoader::Mesh modelLoaderMesh{ ModelLoader::loadGLB(path, m_vertices) };

	Mesh mesh{};

	for (const auto& loaderPrimitive : modelLoaderMesh.primitives)
	{
		GLuint indexBuffer{};
		glGenBuffers(1, &indexBuffer);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(std::uint32_t) * loaderPrimitive.indices.size(), 
			loaderPrimitive.indices.data(), GL_STATIC_DRAW);

		mesh.primitives.push_back({
			.indexBuffer{ indexBuffer },
			.indexCount{ static_cast<GLsizei>(loaderPrimitive.indices.size()) },
			.transform{ loaderPrimitive.transform },
			.material
			{
				.texture{ loaderPrimitive.material.texture },
				.hasTexture{ loaderPrimitive.material.hasTexture },
				.color{ loaderPrimitive.material.color },
			},
			});
	}

	m_meshes.push_back(mesh);
}

void Renderer::finalizeModels()
{
	glGenBuffers(1, &m_vertexBuffer);
	glGenVertexArrays(1, &m_vertexArray);

	glBindBuffer(GL_ARRAY_BUFFER, m_vertexBuffer);
	glBindVertexArray(m_vertexArray);

	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * m_vertices.size(), m_vertices.data(), GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), nullptr);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(sizeof(glm::vec3)));
	glEnableVertexAttribArray(1);

	m_vertices.clear();
	m_vertices.shrink_to_fit();
}

bool Renderer::windowShouldClose()
{
	return glfwWindowShouldClose(m_window);
}

void Renderer::initWindow()
{
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	m_window = glfwCreateWindow(m_initialWindowWidth, m_initialWindowHeight, "Platformer", nullptr, nullptr);

	glfwMakeContextCurrent(m_window);
}

void Renderer::initOpenGL()
{
	gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress));

	glEnable(GL_DEBUG_OUTPUT);
	glDebugMessageCallback(
		[](GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, void const* userParam)
		{
			const char* sourceStr
			{ [source]() {
				switch (source)
				{
				case GL_DEBUG_SOURCE_API: return "API";
				case GL_DEBUG_SOURCE_WINDOW_SYSTEM: return "WINDOW SYSTEM";
				case GL_DEBUG_SOURCE_SHADER_COMPILER: return "SHADER COMPILER";
				case GL_DEBUG_SOURCE_THIRD_PARTY: return "THIRD PARTY";
				case GL_DEBUG_SOURCE_APPLICATION: return "APPLICATION";
				case GL_DEBUG_SOURCE_OTHER: return "OTHER";
				}
			}() };

			const char* typeStr
			{ [type]() {
				switch (type)
				{
				case GL_DEBUG_TYPE_ERROR: return "ERROR";
				case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: return "DEPRECATED_BEHAVIOR";
				case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR: return "UNDEFINED_BEHAVIOR";
				case GL_DEBUG_TYPE_PORTABILITY: return "PORTABILITY";
				case GL_DEBUG_TYPE_PERFORMANCE: return "PERFORMANCE";
				case GL_DEBUG_TYPE_MARKER: return "MARKER";
				case GL_DEBUG_TYPE_OTHER: return "OTHER";
				}
			}() };

			const char* severityStr
			{ [severity]() {
				switch (severity)
				{
				case GL_DEBUG_SEVERITY_NOTIFICATION: return "NOTIFICATION";
				case GL_DEBUG_SEVERITY_LOW: return "LOW";
				case GL_DEBUG_SEVERITY_MEDIUM: return "MEDIUM";
				case GL_DEBUG_SEVERITY_HIGH: return "HIGH";
				}
			}() };

			std::cerr << sourceStr << ", " << typeStr << ", " << typeStr << ", " << id << ": " << message << '\n';
		},
		nullptr);

	glViewport(0, 0, m_initialWindowWidth, m_initialWindowHeight);
	glfwSetFramebufferSizeCallback(m_window, 
		[](GLFWwindow* window, int width, int height) {
			glViewport(0, 0, width, height);
		});

	glEnable(GL_DEPTH_TEST);
}

void Renderer::initPipelines()
{
	m_uberPipeline = { "shaders/uber.vert", "shaders/uber.frag" };
}