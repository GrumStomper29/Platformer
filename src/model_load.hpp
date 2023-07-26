#pragma once

#include "renderer.hpp"

#include "glad/glad.h"
#include "glm/glm.hpp"

#include <cstdint>
#include <string>
#include <vector>

namespace ModelLoader
{

	struct Material
	{
		bool hasTexture{};
		GLuint texture{};
		glm::vec3 color{};
	};

	struct Primitive
	{
		std::vector<std::uint32_t> indices{};
		glm::mat4 transform{};
		Material material{};
	};

	struct Mesh
	{
		std::vector<Primitive> primitives{};
	};

	Mesh loadGLB(const std::string& path, std::vector<Renderer::Vertex>& vertices);

}