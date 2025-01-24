#include "lune/core/gltf.hxx"

#include "lune/core/log.hxx"
#include "lune/game_framework/components/mesh.hxx"
#include "lune/game_framework/components/parent_child.hxx"
#include "lune/game_framework/entities/entity.hxx"
#include "lune/game_framework/scene.hxx"

#include <format>
#include <string>

namespace lune
{
	void modelToScene(const tinygltf::Model& tinyModel, std::string_view alias, int32 tinySceneIndex, Scene* luneScene);
	uint64 processNode(const tinygltf::Model& tinyModel, std::string_view alias, uint32 nodeIndex, Scene* luneScene, EntityBase* parentEntity);
} // namespace lune

bool lune::gltf::loadInScene(std::filesystem::path gltfScene, std::string_view alias, class Scene* scene)
{
	if (scene == nullptr) [[unlikely]]
		return false;

	tinygltf::Model model;
	tinygltf::TinyGLTF loader;
	std::string err;
	std::string warn;

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
		auto meshComp = newEntity->addComponent<MeshComponent>();

		const std::string primitiveAlias = std::format("{}::{}::{}", alias, node.name, nodeMesh.name);
		for (size_t i = 0; i < nodeMesh.primitives.size(); ++i)
		{
            const std::string primitiveName = std::format("{}::{}", primitiveAlias, i);
			// not that simple
            // should be a struct with a bunch of stuff. like materials and shit
            meshComp->primitiveNames.emplace_back(primitiveName);
            // load primitive to gpu
            // load textures to gpu
            // ??? save somewhere materials for reuse
		}
	}

	return newEntity->getId();
}