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
			const std::string primitiveName = std::format("{}::{}", primitiveAlias, i);
			meshComp->primitiveNames.emplace_back(primitiveName);

			std::vector<Vertex33> vertexBuffer{};

			int32 positionAttrIndex{-1};
			int32 normalAttrIndex{-1};
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
			}

			const lnm::vec3* positionsData = nullptr;
			size_t positionBufferCount = 0;
			if (positionAttrIndex != -1)
			{
				const auto& positionAccessor = tinyModel.accessors[positionAttrIndex];
				const auto& positionsBufferView = tinyModel.bufferViews[positionAccessor.bufferView];
				positionsData = reinterpret_cast<const lnm::vec3*>(tinyModel.buffers[positionsBufferView.buffer].data.data() + positionsBufferView.byteOffset + positionAccessor.byteOffset);
				positionBufferCount = positionAccessor.count;
			}

			const lnm::vec3* normalData = nullptr;
			if (normalAttrIndex != -1)
			{
				const auto& normalAccessor = tinyModel.accessors[normalAttrIndex];
				const auto& normalBufferView = tinyModel.bufferViews[normalAccessor.bufferView];
				normalData = reinterpret_cast<const lnm::vec3*>(tinyModel.buffers[normalBufferView.buffer].data.data() + normalBufferView.byteOffset + normalAccessor.byteOffset);
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
			}

			auto vkSubsystem = Engine::get()->findSubsystem<VulkanSubsystem>();
			vkSubsystem->addPrimitive(primitiveName, vulkan::Primitive::create(vertexBuffer.data(), vertexBuffer.size(), sizeof(Vertex33), indicesData, indiciesCount, indiciesSizeof));

			// load textures to gpu
			// ??? save somewhere materials for reuse
		}
	}

	return newEntity->getId();
}