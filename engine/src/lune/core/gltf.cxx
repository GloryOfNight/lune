#include "lune/core/gltf.hxx"

#include "lune/core/engine.hxx"
#include "lune/core/log.hxx"
#include "lune/core/math.hxx"
#include "lune/game_framework/components/mesh.hxx"
#include "lune/game_framework/components/parent_child.hxx"
#include "lune/game_framework/components/transform.hxx"
#include "lune/game_framework/entities/entity.hxx"
#include "lune/game_framework/scene.hxx"
#include "lune/vulkan/primitive.hxx"
#include "lune/vulkan/vulkan_core.hxx"
#include "lune/vulkan/vulkan_subsystem.hxx"

#include <filesystem>
#include <format>
#include <glm/detail/qualifier.hpp>
#include <glm/ext/vector_float3.hpp>
#include <span>
#include <string>
#include <tinygltf/tiny_gltf.h>

bool imageLoad(tinygltf::Image*, const int, std::string*, std::string*, int, int, const unsigned char*, int, void*)
{
	return true;
}

namespace lune
{
	vk::PrimitiveTopology modeToVkTopology(int32 mode);
	void modelToScene(const tinygltf::Model& tinyModel, std::string_view alias, int32 tinySceneIndex, Scene* luneScene);
	uint64 processNode(const tinygltf::Model& tinyModel, std::string_view alias, uint32 nodeIndex, Scene* luneScene, EntityBase* parentEntity);
} // namespace lune

bool lune::gltf::loadInScene(std::filesystem::path gltfScene, std::string_view alias, class Scene* scene)
{
	if (scene == nullptr) [[unlikely]]
		return false;
	if (!std::filesystem::is_regular_file(gltfScene)) [[unlikely]]
		return false;

	tinygltf::Model model;

	tinygltf::TinyGLTF loader{};
	std::string err{};
	std::string warn{};

	loader.SetImageLoader(&imageLoad, nullptr);

	const bool result = loader.LoadASCIIFromFile(&model, &err, &warn, gltfScene.generic_string());
	if (!err.empty())
	{
		LN_LOG(Error, GLTF::Loader, err);
	}
	if (!warn.empty())
	{
		LN_LOG(Warning, GLTF::Loader, err);
	}

	if (result)
	{
		for (size_t i = 0; i < model.scenes.size(); ++i)
		{
			modelToScene(model, alias, i, scene);
		}
	}

	return result;
}

void lune::modelToScene(const tinygltf::Model& tinyModel, std::string_view alias, int32 tinySceneIndex, Scene* scene)
{
	auto& tinyScene = tinyModel.scenes[tinySceneIndex];
	const size_t tinySceneNodeSize = tinyScene.nodes.size();
	for (size_t i = 0; i < tinySceneNodeSize; ++i)
	{
		processNode(tinyModel, alias, tinyScene.nodes[i], scene, nullptr);
	}
}

uint64 lune::processNode(const tinygltf::Model& tinyModel, std::string_view alias, uint32 nodeIndex, Scene* scene, EntityBase* parentEntity)
{
	const auto& node = tinyModel.nodes[nodeIndex];

	auto newEntity = scene->addEntity<lune::EntityBase>();
	auto parentChildComp = newEntity->addComponent<ParentChildComponent>();

	if (parentEntity)
		parentChildComp->mParentId = parentEntity->getId();

	for (size_t i = 0; i < node.children.size(); ++i)
	{
		const uint64 childEntityId = processNode(tinyModel, alias, node.children[i], scene, newEntity);
		parentChildComp->mChildren.emplace(childEntityId);
	}

	if (node.mesh != -1 && tinyModel.meshes[node.mesh].primitives.size())
	{
		const auto& nodeMesh = tinyModel.meshes[node.mesh];
		auto transformComp = newEntity->addComponent<TransformComponent>();
		auto meshComp = newEntity->addComponent<MeshComponent>();

		const std::string primitiveAlias = std::format("{}::{}::{}", alias, node.name, nodeMesh.name);
		for (size_t i = 0; i < nodeMesh.primitives.size(); ++i)
		{
			const auto& primitive = nodeMesh.primitives[i];
			const std::string primitiveName = std::format("{}::primitive::{}", primitiveAlias, i);
			auto& meshCompPrimitive = meshComp->primitives.emplace_back();
			meshCompPrimitive.primitiveName = primitiveName;
			if (primitive.material != -1)
				meshCompPrimitive.materialName = std::format("{}::material::{}::{}", alias, tinyModel.materials[primitive.material].name, primitive.material);
			meshCompPrimitive.topology = modeToVkTopology(primitive.mode);

			std::vector<Vertex33224> vertexBuffer{};

			int32 positionAttrIndex{-1};
			int32 normalAttrIndex{-1};
			std::vector<int32> texCoordsAttrIndices{};
			std::vector<int32> colorAttrIndices{};

			for (const auto& [name, index] : primitive.attributes)
			{
				if (name == "POSITION")
				{
					positionAttrIndex = index;
				}
				else if (name == "NORMAL")
				{
					normalAttrIndex = index;
				}
				else if (name.find("TEXCOORD_") != std::string::npos)
				{
					texCoordsAttrIndices.emplace_back(index);
				}
				else if (name.find("COLOR_") != std::string::npos)
				{
					colorAttrIndices.emplace_back(index);
				}
			}

			const lnm::vec3* positionsData = nullptr;
			const lnm::vec3* normalData = nullptr;
			const lnm::uint8* uv0 = nullptr;
			const lnm::uint8* uv1 = nullptr;
			const lnm::uint8* color0 = nullptr;

			size_t positionBufferCount = 0;
			if (positionAttrIndex != -1)
			{
				const auto& positionAccessor = tinyModel.accessors[positionAttrIndex];
				const auto& positionsBufferView = tinyModel.bufferViews[positionAccessor.bufferView];
				positionsData = reinterpret_cast<const lnm::vec3*>(tinyModel.buffers[positionsBufferView.buffer].data.data() + positionsBufferView.byteOffset + positionAccessor.byteOffset);
				positionBufferCount = positionAccessor.count;
			}

			if (normalAttrIndex != -1)
			{
				const auto& normalAccessor = tinyModel.accessors[normalAttrIndex];
				const auto& normalBufferView = tinyModel.bufferViews[normalAccessor.bufferView];
				normalData = reinterpret_cast<const lnm::vec3*>(tinyModel.buffers[normalBufferView.buffer].data.data() + normalBufferView.byteOffset + normalAccessor.byteOffset);
			}

			for (size_t k = 0; k < texCoordsAttrIndices.size(); ++k)
			{
				const int32 attrIndex = texCoordsAttrIndices[k];
				const auto& accessor = tinyModel.accessors[attrIndex];
				const auto& bufferView = tinyModel.bufferViews[accessor.bufferView];

				const uint8* data = reinterpret_cast<const uint8*>(tinyModel.buffers[bufferView.buffer].data.data() + bufferView.byteOffset + accessor.byteOffset);
				if (k == 0)
					uv0 = data;
				else if (k == 1)
					uv1 = data;
				else
					break; // not supported
			}

			for (size_t k = 0; k < colorAttrIndices.size(); ++k)
			{
				const int32 attrIndex = colorAttrIndices[k];
				const auto& accessor = tinyModel.accessors[attrIndex];
				const auto& bufferView = tinyModel.bufferViews[accessor.bufferView];
				const uint8* data = reinterpret_cast<const uint8*>(tinyModel.buffers[bufferView.buffer].data.data() + bufferView.byteOffset + accessor.byteOffset);
				if (k == 0)
					color0 = data;
				else
					break; // not supported
			}

			const uint8* indicesData = nullptr;
			size_t indiciesCount = 0;
			int32 indiciesSizeof = 0;
			if (primitive.indices != -1)
			{
				const auto& indicesAccessor = tinyModel.accessors[primitive.indices];
				const auto& indicesBufferView = tinyModel.bufferViews[indicesAccessor.bufferView];
				indicesData = reinterpret_cast<const uint8*>(tinyModel.buffers[indicesBufferView.buffer].data.data() + indicesBufferView.byteOffset + indicesAccessor.byteOffset);
				indiciesCount = indicesAccessor.count;
				if (indicesAccessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT)
					indiciesSizeof = sizeof(uint16);
				else if (indicesAccessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT)
					indiciesSizeof = sizeof(uint32);
			}

			vertexBuffer.resize(positionBufferCount);
			for (size_t k = 0; k < positionBufferCount; ++k)
			{
				vertexBuffer[k].position = *(positionsData + k);
				if (normalData)
				{
					vertexBuffer[k].normal = *(normalData + k);
				}
				if (uv0)
				{
					const auto& accessor = tinyModel.accessors[texCoordsAttrIndices[0]];
					if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT)
						vertexBuffer[k].uv0 = *reinterpret_cast<const lnm::vec2*>(uv0 + (k * sizeof(lnm::vec2)));
					else if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT)
						vertexBuffer[k].uv0 = *reinterpret_cast<const lnm::u16vec2*>(uv0 + (k * sizeof(lnm::u16vec2)));
					else if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE)
						vertexBuffer[k].uv0 = *reinterpret_cast<const lnm::u8vec2*>(uv0 + (k * sizeof(lnm::u8vec2)));
				}
				if (uv1)
				{
					const auto& accessor = tinyModel.accessors[texCoordsAttrIndices[1]];
					if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT)
						vertexBuffer[k].uv1 = *reinterpret_cast<const lnm::vec2*>(uv1 + (k * sizeof(lnm::vec2)));
					else if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT)
						vertexBuffer[k].uv1 = *reinterpret_cast<const lnm::u16vec2*>(uv1 + (k * sizeof(lnm::u16vec2)));
					else if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE)
						vertexBuffer[k].uv1 = *reinterpret_cast<const lnm::u8vec2*>(uv1 + (k * sizeof(lnm::u8vec2)));
				}
				if (color0)
				{
					const auto& accessor = tinyModel.accessors[colorAttrIndices[0]];
					if (accessor.type == TINYGLTF_TYPE_VEC4)
					{
						if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT)
							vertexBuffer[k].color0 = lnm::vec4(*reinterpret_cast<const lnm::vec3*>(color0 + (k * sizeof(lnm::vec3))), 1.f);
						else if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT)
							vertexBuffer[k].color0 = lnm::u16vec4(*reinterpret_cast<const lnm::u16vec3*>(color0 + (k * sizeof(lnm::u16vec3))), 1);
						else if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE)
							vertexBuffer[k].color0 = lnm::u8vec4(*reinterpret_cast<const lnm::u8vec3*>(color0 + (k * sizeof(lnm::u8vec3))), 1);
					}
					else if (accessor.type == TINYGLTF_TYPE_VEC3)
					{
						if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT)
							vertexBuffer[k].color0 = *reinterpret_cast<const lnm::vec4*>(color0 + (k * sizeof(lnm::vec4)));
						else if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT)
							vertexBuffer[k].color0 = *reinterpret_cast<const lnm::u16vec4*>(color0 + (k * sizeof(lnm::u16vec4)));
						else if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE)
							vertexBuffer[k].color0 = *reinterpret_cast<const lnm::u8vec4*>(color0 + (k * sizeof(lnm::u8vec4)));
					}
				}
				else
				{
					vertexBuffer[k].color0 = lnm::vec4(1.f, 1.f, 1.f, 1.f);
				}
			}

			auto vkSubsystem = Engine::get()->findSubsystem<VulkanSubsystem>();
			vkSubsystem->addPrimitive(primitiveName, vulkan::Primitive::create(vertexBuffer.data(), vertexBuffer.size(), sizeof(Vertex33224), indicesData, indiciesCount, indiciesSizeof));

			// load textures to gpu
			// ??? save somewhere materials for reuse
		}
	}

	return newEntity->getId();
}

vk::PrimitiveTopology lune::modeToVkTopology(int32 mode)
{
	switch (mode)
	{
	case TINYGLTF_MODE_POINTS:
		return vk::PrimitiveTopology::ePointList;
	case TINYGLTF_MODE_LINE:
		return vk::PrimitiveTopology::eLineList;
	case TINYGLTF_MODE_LINE_LOOP:
		return vk::PrimitiveTopology::eLineList;
	case TINYGLTF_MODE_LINE_STRIP:
		return vk::PrimitiveTopology::eLineStrip;
	case TINYGLTF_MODE_TRIANGLES:
		return vk::PrimitiveTopology::eTriangleList;
	case TINYGLTF_MODE_TRIANGLE_STRIP:
		return vk::PrimitiveTopology::eTriangleStrip;
	case TINYGLTF_MODE_TRIANGLE_FAN:
		return vk::PrimitiveTopology::eTriangleFan;
	}
	return vk::PrimitiveTopology::eTriangleList;
}