#include "renderer.hpp"

#include "model_load.hpp"

#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include "glm/glm.hpp"

//#ifdef RENDERER_USE_IMGUI
#include "imgui/imgui.h"
#include "imgui/imgui_impl_opengl3.h"
#include "imgui/imgui_impl_glfw.h"
//#endif

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

	initImgui();
}

void Renderer::beginFrame()
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClearColor(0.6f, 0.8f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	ImGui_ImplGlfw_NewFrame();
	ImGui_ImplOpenGL3_NewFrame();
	ImGui::NewFrame();
}

void Renderer::render(const glm::mat4& transform, bool shadowpass, bool executeAABBPass)
{
	if (shadowpass)
	{
		this->shadowpass(transform);
	}

	renderpass(transform);
	
	if (executeAABBPass)
	{
		aabbpass(transform);
	}

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

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

	ImGui_ImplOpenGL3_Shutdown();

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

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_TRUE, sizeof(Vertex), reinterpret_cast<void*>(sizeof(glm::vec3)));
	glEnableVertexAttribArray(1);

	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(sizeof(glm::vec3) + sizeof(glm::vec3)));
	glEnableVertexAttribArray(2);

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

	glfwWindowHint(GLFW_SAMPLES, 8);

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

			if (type != GL_DEBUG_TYPE_OTHER)
			{
				std::cerr << sourceStr << ", " << typeStr << ", " << severityStr << ", " << id << ": " << message << '\n';
			}
		},
		nullptr);

	glViewport(0, 0, m_initialWindowWidth, m_initialWindowHeight);
	glfwSetFramebufferSizeCallback(m_window, 
		[](GLFWwindow* window, int width, int height) {
			glViewport(0, 0, width, height);
		});

	glEnable(GL_DEPTH_TEST);

	glEnable(GL_MULTISAMPLE);

	glGenFramebuffers(1, &m_shadowFBO);

	glGenTextures(1, &m_shadowMap);
	glBindTexture(GL_TEXTURE_2D, m_shadowMap);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, 1024, 1024, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glBindFramebuffer(GL_FRAMEBUFFER, m_shadowFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_shadowMap, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Renderer::initPipelines()
{
	m_uberPipeline = { "shaders/uber.vert", "shaders/uber.frag" };

	m_aabbPipeline = { "shaders/aabb.vert", "shaders/aabb.frag" };
}

void Renderer::initImgui()
{
	ImGui::CreateContext();
	
	ImGui_ImplGlfw_InitForOpenGL(m_window, true);
	ImGui_ImplOpenGL3_Init();

	ImGui_ImplOpenGL3_CreateFontsTexture();

	ImGui::StyleColorsLight();
}

void Renderer::renderpass(const glm::mat4& transform)
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glViewport(0, 0, m_initialWindowWidth, m_initialWindowHeight);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	m_uberPipeline.bind();

	glBindVertexArray(m_vertexArray);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	for (const auto& meshInstance : meshInstances)
	{
		if (meshInstance.pass == UBER)
		{
			if (meshInstance.show)
			{
				for (const auto& primitive : m_meshes[meshInstance.mesh].primitives)
				{
					glUniform1i(glGetUniformLocation(m_uberPipeline.shaderProgram(), "textured"), primitive.material.hasTexture);
					glUniformMatrix4fv(glGetUniformLocation(m_uberPipeline.shaderProgram(), "transform"), 1, GL_FALSE, glm::value_ptr(glm::mat4{ transform* (meshInstance.transform* primitive.transform) }));
					glUniform3fv(glGetUniformLocation(m_uberPipeline.shaderProgram(), "color"), 1, glm::value_ptr(primitive.material.color));

					glActiveTexture(GL_TEXTURE0);
					glBindTexture(GL_TEXTURE_2D, primitive.material.texture);

					glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, primitive.indexBuffer);

					glDrawElements(GL_TRIANGLES, primitive.indexCount, GL_UNSIGNED_INT, nullptr);
				}
			}
		}
	}
}

void Renderer::shadowpass(const glm::mat4& transform)
{
	glViewport(0, 0, 1024, 1024);
	glBindFramebuffer(GL_FRAMEBUFFER, m_shadowFBO);
	glClear(GL_DEPTH_BUFFER_BIT);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	m_uberPipeline.bind();

	glBindVertexArray(m_vertexArray);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	for (const auto& meshInstance : meshInstances)
	{
		if (meshInstance.pass == UBER)
		{
			if (meshInstance.show)
			{
				for (const auto& primitive : m_meshes[meshInstance.mesh].primitives)
				{
					glUniform1i(glGetUniformLocation(m_uberPipeline.shaderProgram(), "textured"), primitive.material.hasTexture);
					glUniformMatrix4fv(glGetUniformLocation(m_uberPipeline.shaderProgram(), "transform"), 1, GL_FALSE, glm::value_ptr(glm::mat4{ transform* (meshInstance.transform* primitive.transform) }));
					glUniform3fv(glGetUniformLocation(m_uberPipeline.shaderProgram(), "color"), 1, glm::value_ptr(primitive.material.color));

					glActiveTexture(GL_TEXTURE0);
					glBindTexture(GL_TEXTURE_2D, primitive.material.texture);

					glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, primitive.indexBuffer);

					glDrawElements(GL_TRIANGLES, primitive.indexCount, GL_UNSIGNED_INT, nullptr);
				}
			}
		}
	}
}

void Renderer::aabbpass(const glm::mat4& transform)
{
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	m_aabbPipeline.bind();

	for (const auto& meshInstance : meshInstances)
	{
		if (meshInstance.pass == AABB)
		{
			for (const auto& primitive : m_meshes[meshInstance.mesh].primitives)
			{
				glUniformMatrix4fv(glGetUniformLocation(m_aabbPipeline.shaderProgram(), "transform"), 1, GL_FALSE, glm::value_ptr(glm::mat4{ transform* (meshInstance.transform* primitive.transform) }));

				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, primitive.indexBuffer);

				glDrawElements(GL_TRIANGLES, primitive.indexCount, GL_UNSIGNED_INT, nullptr);
			}
		}
	}
}