#pragma once

#include "pipeline.hpp"

#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include "glm/glm.hpp"

#include <string>
#include <vector>

class Renderer final
{
public:

	Renderer() = default;
	Renderer(const Renderer&) = delete;
	Renderer& operator=(const Renderer&) = delete;

	struct Vertex
	{
		glm::vec3 position{};
		glm::vec2 texCoord{};
	};

	struct Material
	{
		GLuint texture{};
		bool hasTexture{};
		glm::vec3 color{};
	};

	struct Primitive
	{
		GLuint indexBuffer{};
		GLsizei indexCount{};

		glm::mat4 transform{};

		Material material{};
	};

	struct Mesh
	{
		std::vector<Primitive> primitives{};
	};

	struct MeshInstance
	{
		int mesh{};

		glm::mat4 transform{};
	};

	void init();
	void render(const glm::mat4& transform);
	void cleanup();

	void loadModel(const std::string& path);
	void finalizeModels();

	bool windowShouldClose();

	GLFWwindow* window() { return m_window; }

	std::vector<MeshInstance> meshInstances{};

private:

	void initWindow();
	void initOpenGL();

	void initPipelines();

	GLFWwindow* m_window{};
	static constexpr int m_initialWindowWidth{ 1600 };
	static constexpr int m_initialWindowHeight{ 900 };

	std::vector<Vertex> m_vertices{};
	GLuint m_vertexBuffer{};
	GLuint m_vertexArray{};
	std::vector<Mesh> m_meshes{};

	Pipeline m_uberPipeline{};
};