#include "lune/core/gltf.hxx"

#include "lune/core/assets.hxx"
#include "lune/core/engine.hxx"
#include "lune/core/log.hxx"
#include "lune/core/math.hxx"
#include "lune/core/sdl.hxx"
#include "lune/game_framework/components/mesh.hxx"
#include "lune/game_framework/components/parent_child.hxx"
#include "lune/game_framework/components/transform.hxx"
#include "lune/game_framework/entities/entity.hxx"
#include "lune/game_framework/scene.hxx"
#include "lune/vulkan/buffer.hxx"
#include "lune/vulkan/pipeline.hxx"
#include "lune/vulkan/primitive.hxx"
#include "lune/vulkan/sampler.hxx"
#include "lune/vulkan/shader.hxx"
#include "lune/vulkan/texture_image.hxx"
#include "lune/vulkan/vulkan_core.hxx"
#include "lune/vulkan/vulkan_subsystem.hxx"

#include <SDL3_image/SDL_image.h>
#include <cstddef>
#include <filesystem>
#include <format>
#include <glm/detail/qualifier.hpp>
#include <glm/ext/vector_float3.hpp>
#include <glm/fwd.hpp>
#include <memory>
#include <span>
#include <string>
#include <tinygltf/tiny_gltf.h>
#include <vulkan/vulkan_enums.hpp>

bool imageLoad(tinygltf::Image*, const int, std::string*, std::string*, int, int, const unsigned char*, int, void*)
{
	return true;
}

namespace lune
{
	vk::PrimitiveTopology makeTopology(int32 mode);
	vk::Filter makeFilter(int32 tinyFilter);

	void modelToScene(std::filesystem::path sceneRoot, const tinygltf::Model& tinyModel, std::string_view alias, int32 tinySceneIndex, Scene* luneScene);
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
			modelToScene(gltfScene.parent_path(), model, alias, i, scene);
		}
	}

	return result;
}

void lune::modelToScene(std::filesystem::path sceneRoot, const tinygltf::Model& tinyModel, std::string_view alias, int32 tinySceneIndex, Scene* scene)
{
	auto& tinyScene = tinyModel.scenes[tinySceneIndex];
	const size_t tinySceneNodeSize = tinyScene.nodes.size();
	for (size_t i = 0; i < tinySceneNodeSize; ++i)
	{
		processNode(tinyModel, alias, tinyScene.nodes[i], scene, nullptr);
	}

	auto vkSubsystem = Engine::get()->findSubsystem<VulkanSubsystem>();
	for (size_t i = 0; i < tinyModel.samplers.size(); ++i)
	{
		const auto& tinySampler = tinyModel.samplers[i];
		vk::SamplerCreateInfo createInfo = vulkan::Sampler::defaultCreateInfo();
		createInfo.setMagFilter(makeFilter(tinySampler.magFilter)).setMinFilter(makeFilter(tinySampler.minFilter));
		vkSubsystem->addSampler(std::format("{}::sampler::{}", alias, i), vulkan::Sampler::create(createInfo));
	}
	for (size_t i = 0; i < tinyModel.textures.size(); ++i)
	{
		const auto& tinyTexture = tinyModel.textures[i];
		const auto& tinyImage = tinyModel.images[tinyTexture.source];
		const auto imagePath = sceneRoot / tinyImage.uri;
		auto newSurface = UniqueSDLSurface(IMG_Load(imagePath.generic_string().c_str()));
		vkSubsystem->addTextureImage(std::format("{}::texture::{}", alias, i), vulkan::TextureImage::create(newSurface.get()));
	}
	for (size_t i = 0; i < tinyModel.materials.size(); ++i)
	{
		vulkan::gltf::SharedMaterial newMaterial = std::make_shared<vulkan::gltf::Material>();
		newMaterial->init(&tinyModel, &tinyModel.materials[i], alias);
		vkSubsystem->addCustomResource(std::format("{}::material::{}", alias, i), std::move(newMaterial));
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
				meshCompPrimitive.materialName = std::format("{}::material::{}", alias, primitive.material);
			meshCompPrimitive.topology = makeTopology(primitive.mode);

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

vk::PrimitiveTopology lune::makeTopology(int32 mode)
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

vk::Filter lune::makeFilter(int32 tinyFilter)
{
	switch (tinyFilter)
	{
	case TINYGLTF_TEXTURE_FILTER_LINEAR:
		return vk::Filter::eLinear;
	case TINYGLTF_TEXTURE_FILTER_NEAREST:
		return vk::Filter::eNearest;
	}
	return vk::Filter::eLinear;
}

struct ShaderMaterialData
{
	lnm::vec3 emissiveFactor{};
	lnm::vec4 baseColorFactor{};
	float metallicFactor{};
	float roughnessFactor{};
	float normalScale{};

	int32 baseColorTextureUVSet{-1};
	int32 metallicRoughnessTextureUVSet{-1};
	int32 NormalTextureUVSet{-1};
	int32 emissiveTextureUVSet{-1};
};

void lune::vulkan::gltf::Material::init(const tinygltf::Model* tinyModel, const tinygltf::Material* tinyMaterial, const std::string_view alias)
{
	auto vkSubsystem = Engine::get()->findSubsystem<VulkanSubsystem>();

	auto shVert = vkSubsystem->loadShader(*EngineShaderPath("gltf/primitive.vert.spv"));
	auto shFrag = vkSubsystem->loadShader(*EngineShaderPath("gltf/primitive.frag.spv"));

	auto rasterizationState = GraphicsPipeline::defaultRasterizationState();
	rasterizationState.setCullMode(tinyMaterial->doubleSided ? vk::CullModeFlagBits::eNone : vk::CullModeFlagBits::eBack);

	vulkan::GraphicsPipeline::StatesOverride statesOverride{};
	statesOverride.rasterization = &rasterizationState;
	statesOverride.dynamicStates = vulkan::GraphicsPipeline::defaultDynamicStates();
	statesOverride.dynamicStates.emplace_back(vk::DynamicState::ePrimitiveTopology);

	mPipeline = GraphicsPipeline::create(shVert, shFrag, statesOverride);

	ShaderMaterialData shaderMat{};
	shaderMat.emissiveFactor = *reinterpret_cast<const lnm::dvec3*>(tinyMaterial->emissiveFactor.data());
	shaderMat.baseColorFactor = *reinterpret_cast<const lnm::dvec4*>(tinyMaterial->pbrMetallicRoughness.baseColorFactor.data());
	shaderMat.metallicFactor = tinyMaterial->pbrMetallicRoughness.metallicFactor;
	shaderMat.roughnessFactor = tinyMaterial->pbrMetallicRoughness.roughnessFactor;
	shaderMat.normalScale = tinyMaterial->normalTexture.scale;

	uint8 texCounter = 0;

	mTextures = std::vector<SharedTextureImage>(4, vkSubsystem->findTextureImage("lune::default"));
	mSamplers = std::vector<SharedSampler>(4, vkSubsystem->findSampler("lune::default"));

	if (tinyMaterial->pbrMetallicRoughness.baseColorTexture.index != -1)
	{
		shaderMat.baseColorTextureUVSet = tinyMaterial->pbrMetallicRoughness.baseColorTexture.texCoord;
		mTextures[0] = (vkSubsystem->findTextureImage(std::format("{}::texture::{}", alias, tinyMaterial->pbrMetallicRoughness.baseColorTexture.index)));
		mSamplers[0] = (vkSubsystem->findSampler(std::format("{}::sampler::{}", alias, tinyModel->textures[tinyMaterial->pbrMetallicRoughness.baseColorTexture.index].sampler)));
	}

	if (tinyMaterial->pbrMetallicRoughness.metallicRoughnessTexture.index != -1)
	{
		shaderMat.metallicRoughnessTextureUVSet = tinyMaterial->pbrMetallicRoughness.metallicRoughnessTexture.texCoord;
		mTextures[1] = (vkSubsystem->findTextureImage(std::format("{}::texture::{}", alias, tinyMaterial->pbrMetallicRoughness.metallicRoughnessTexture.index)));
		mSamplers[1] = (vkSubsystem->findSampler(std::format("{}::sampler::{}", alias, tinyModel->textures[tinyMaterial->pbrMetallicRoughness.metallicRoughnessTexture.index].sampler)));
	}

	if (tinyMaterial->normalTexture.index != -1)
	{
		shaderMat.NormalTextureUVSet = tinyMaterial->normalTexture.texCoord;
		mTextures[2] = (vkSubsystem->findTextureImage(std::format("{}::texture::{}", alias, tinyMaterial->normalTexture.index)));
		mSamplers[2] = (vkSubsystem->findSampler(std::format("{}::sampler::{}", alias, tinyModel->textures[tinyMaterial->normalTexture.index].sampler)));
	}

	if (tinyMaterial->emissiveTexture.index != -1)
	{
		shaderMat.emissiveTextureUVSet = tinyMaterial->emissiveTexture.texCoord;
		mTextures[3] = (vkSubsystem->findTextureImage(std::format("{}::texture::{}", alias, tinyMaterial->emissiveTexture.index)));
		mSamplers[3] = (vkSubsystem->findSampler(std::format("{}::sampler::{}", alias, tinyModel->textures[tinyMaterial->emissiveTexture.index].sampler)));
	}

	mBuffer = Buffer::create(vk::BufferUsageFlagBits::eUniformBuffer | vk::BufferUsageFlagBits::eTransferDst, sizeof(shaderMat), VMA_MEMORY_USAGE_AUTO, VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT);
	mBuffer->copyTransfer(&shaderMat, 0, sizeof(shaderMat));
}