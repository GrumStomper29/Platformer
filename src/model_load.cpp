#include "model_load.hpp"

#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "tiny_gltf.h"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/quaternion.hpp"

#include <cstdint>
#include <cstring>
#include <iostream>
#include <string>

namespace ModelLoader
{

	Material getPrimitiveMaterial(const tinygltf::Model& model, const tinygltf::Primitive& primitive)
	{
		Material outMaterial{};
		if (primitive.material != -1)
		{
			const tinygltf::Material& material{ model.materials[primitive.material] };
			const tinygltf::TextureInfo& textureInfo{ material.pbrMetallicRoughness.baseColorTexture };
			if (textureInfo.index != -1)
			{
				const tinygltf::Texture& texture{ model.textures[textureInfo.index] };
				const tinygltf::Image& image{ model.images[texture.source] };

				glGenTextures(1, &outMaterial.texture);
				glBindTexture(GL_TEXTURE_2D, outMaterial.texture);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image.width, image.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image.image.data());
				glGenerateMipmap(GL_TEXTURE_2D);

				outMaterial.hasTexture = true;
			}
			else
			{
				outMaterial.hasTexture = false;
			}
			
			outMaterial.color =
			{
				material.pbrMetallicRoughness.baseColorFactor[0],
				material.pbrMetallicRoughness.baseColorFactor[1],
				material.pbrMetallicRoughness.baseColorFactor[2]
			};
		}
		
		return outMaterial;
	}

	glm::mat4 getNodeTransform(const tinygltf::Node& node)
	{
		if (node.matrix.size() != 0)
		{
			std::cerr << "Transformation found\n";
		}

		glm::mat4 t{ 1.0f };
		if (node.translation.size() != 0)
		{
			glm::vec3 translation
			{
				static_cast<float>(node.translation[0]),
				static_cast<float>(node.translation[1]),
				static_cast<float>(node.translation[2]),
			};
			t = glm::translate(t, translation);
		}

		glm::mat4 r{ 1.0f };
		if (node.rotation.size() != 0)
		{
			// glm quaternions are in WXYZ
			glm::quat rotation
			{
				static_cast<float>(node.rotation[3]),
				static_cast<float>(node.rotation[0]),
				static_cast<float>(node.rotation[1]),
				static_cast<float>(node.rotation[2])
			};
			r = glm::toMat4(rotation);
		}

		glm::mat4 s{ 1.0f };
		if (node.scale.size() != 0)
		{
			glm::vec3 scale
			{
				static_cast<float>(node.scale[0]),
				static_cast<float>(node.scale[1]),
				static_cast<float>(node.scale[2])
			};
			s = glm::scale(s, scale);
		}

		return t * r * s;
	}

	Primitive loadPrimitive(const tinygltf::Model& model, const tinygltf::Primitive& primitive, std::vector<Renderer::Vertex>& vertices)
	{
		Primitive outPrimitive{};
		outPrimitive.material = getPrimitiveMaterial(model, primitive);

		const auto& accessor{ model.accessors[primitive.indices] };
		const auto& bufferView{ model.bufferViews[accessor.bufferView] };
		const auto& buffer{ model.buffers[bufferView.buffer] };

		const unsigned char* indexData{ buffer.data.data() };
		indexData += bufferView.byteOffset;

		const std::uint32_t offset{ static_cast<std::uint32_t>(vertices.size()) };

		switch (accessor.componentType)
		{
		case (TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT):
		{
			for (int i{ 0 }; i < (bufferView.byteLength / sizeof(std::uint16_t)); ++i)
			{
				std::uint16_t index{};
				std::memcpy(&index, indexData, sizeof(std::uint16_t));
				indexData += sizeof(std::uint16_t);
				
				outPrimitive.indices.push_back(static_cast<std::uint32_t>(index) + offset);
			}
		}
		break;
		case (TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT):
		{
			for (int i{ 0 }; i < (bufferView.byteLength / sizeof(std::uint32_t)); ++i)
			{
				std::uint32_t index{};
				std::memcpy(&index, indexData, sizeof(std::uint32_t));
				indexData += sizeof(std::uint32_t);
				
				outPrimitive.indices.push_back(index + offset);
			}
		}
		break;
		default:
		std::cerr << "MODEL LOADER, ERROR: Unrecognized index accessor component type " << accessor.componentType << '\n';
		break;
		}

		const unsigned char* positionData{ nullptr };
		const unsigned char* normalData  { nullptr };
		const unsigned char* texCoordData{ nullptr };
		const unsigned char* colorData   { nullptr };

		int vertexCount{};
		
		for (const auto& attribute : primitive.attributes)
		{
			const auto& accessor{ model.accessors[attribute.second] };
			const auto& bufferView{ model.bufferViews[accessor.bufferView] };
			const auto& buffer{ model.buffers[bufferView.buffer] };

			if (attribute.first == "POSITION")
			{
				vertexCount = static_cast<int>(bufferView.byteLength) / sizeof(glm::vec3);
				positionData = buffer.data.data() + bufferView.byteOffset;
			}
			else if (attribute.first == "NORMAL") normalData = buffer.data.data() + bufferView.byteOffset;
			else if (attribute.first == "TEXCOORD_0") texCoordData = buffer.data.data() + bufferView.byteOffset;
			else if (attribute.first == "COLOR_0") std::cerr << "Color\n";
		}

		for (int i{ 0 }; i < vertexCount; ++i)
		{
			glm::vec3 position{};
			std::memcpy(&position, positionData, sizeof(glm::vec3));
			positionData += sizeof(glm::vec3);

			glm::vec3 normal{ 0.0f, 1.0f, 0.0f };
			if (normalData)
			{
				std::memcpy(&normal, normalData, sizeof(glm::vec3));
				normalData += sizeof(glm::vec3);
			}

			glm::vec2 texCoord{ 0.0f, 0.0f };
			if (texCoordData)
			{
				std::memcpy(&texCoord, texCoordData, sizeof(glm::vec2));
				texCoordData += sizeof(glm::vec2);
			}

			vertices.push_back({ position, normal, texCoord });
		}

		return outPrimitive;
	}

	void loadNode(const tinygltf::Model& model, const tinygltf::Node& node, const glm::mat4& inheritedTransform, 
		Mesh& outMesh, std::vector<Renderer::Vertex>& vertices)
	{
		glm::mat4 transform{ getNodeTransform(node) };
		transform = inheritedTransform * transform;

		if (node.mesh != -1)
		{
			const auto mesh{ model.meshes[node.mesh] };

			for (const auto& primitive : mesh.primitives)
			{
				outMesh.primitives.push_back(loadPrimitive(model, primitive, vertices));
				outMesh.primitives.back().transform = transform;
			}
		}

		for (const auto& nodeIndex : node.children)
		{
			loadNode(model, model.nodes[nodeIndex], transform, outMesh, vertices);
		}
	}

	Mesh loadGLB(const std::string& path, std::vector<Renderer::Vertex>& vertices)
	{
		Mesh outMesh{};

		tinygltf::Model model{};
		tinygltf::TinyGLTF loader{};
		std::string warning{};
		std::string error{};

		loader.LoadBinaryFromFile(&model, &error, &warning, path);
		if (!error.empty())
		{
			std::cerr << "MODEL LOADER, ERROR: " << error << '\n';
		}
		if (!warning.empty())
		{
			std::cerr << "MODEL LOADER, WARNING: " << warning << '\n';
		}

		for (const auto& scene : model.scenes)
		{
			for (const auto& nodeIndex : scene.nodes)
			{
				loadNode(model, model.nodes[nodeIndex], glm::mat4{ 1.0f }, outMesh, vertices);
			}
		}

		return outMesh;
	}

}