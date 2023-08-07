#pragma once

#include "pipeline.hpp"

#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include "glm/glm.hpp"

// Todo: remove if RENDERER_USE_IMGUI is not defined
#include "imgui/imgui.h"
#include "imgui/imgui_impl_opengl3.h"
#include "imgui/imgui_impl_glfw.h"

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
		glm::vec3 normal{};
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

	enum Pass
	{
		UBER,
		AABB,
	};

	struct MeshInstance
	{
		int mesh{};

		glm::mat4 transform{};

		Pass pass{ UBER };

		bool show{ true };
	};

	void init();
	void beginFrame();
	void render(const glm::mat4& transform, bool shadowpass, bool executeAABBPass);
	void cleanup();

	void loadModel(const std::string& path);
	void finalizeModels();

	bool windowShouldClose();

	GLFWwindow* window() const { return m_window; }

	std::vector<MeshInstance> meshInstances{};

private:

	void initWindow();
	void initOpenGL();

	void initPipelines();

	void initImgui();

	void renderpass(const glm::mat4& transform);
	void shadowpass(const glm::mat4& transform);
	void aabbpass(const glm::mat4& transform);

	GLFWwindow* m_window{};
	static constexpr int m_initialWindowWidth{ 1600 };
	static constexpr int m_initialWindowHeight{ 900 };

	GLuint m_shadowFBO{};
	GLuint m_shadowMap{};

	std::vector<Vertex> m_vertices{};
	GLuint m_vertexBuffer{};
	GLuint m_vertexArray{};
	std::vector<Mesh> m_meshes{};

	Pipeline m_uberPipeline{};
	Pipeline m_aabbPipeline{};
};