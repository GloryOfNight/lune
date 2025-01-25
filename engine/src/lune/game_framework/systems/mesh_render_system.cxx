#include "lune/game_framework/systems/mesh_render_system.hxx"

#include "lune/core/engine.hxx"
#include "lune/game_framework/components/mesh.hxx"
#include "lune/game_framework/components/transform.hxx"
#include "lune/game_framework/systems/camera_system.hxx"
#include "lune/vulkan/buffer.hxx"
#include "lune/vulkan/descriptor_sets.hxx"
#include "lune/vulkan/pipeline.hxx"
#include "lune/vulkan/primitive.hxx"
#include "lune/vulkan/vulkan_subsystem.hxx"

#include <vulkan/vulkan_enums.hpp>


lune::MeshRenderSystem::MeshRenderSystem()
{
	addDependecy<CameraSystem>();
}

void lune::MeshRenderSystem::prepareRender(class Scene* scene)
{
	const auto& entities = scene->getEntities();
	auto vkSubsystem = Engine::get()->findSubsystem<VulkanSubsystem>();
	auto [viewId, imageIndex, commandBuffer] = vkSubsystem->getFrameInfo();

	auto cameraSystem = scene->findSystem<CameraSystem>();
	if (!cameraSystem)
		return;

	if (!mPipeline)
		mPipeline = vkSubsystem->findPipeline("lune::gltf::primitive");

	auto eIds = scene->getComponentEntities<MeshComponent>();
	for (uint64 eId : eIds)
	{
		auto entity = scene->findEntity(eId);
		auto meshComponent = entity->findComponent<MeshComponent>();

		MeshResources* res{};
		if (auto it = mResources.find(meshComponent); it == mResources.end())
		{
			MeshResources resources{};
			for (auto& primitive : meshComponent->primitives)
				resources.primitives.emplace_back(vkSubsystem->findPrimitive(primitive.primitiveName));

			resources.stagingModelBuffer = vulkan::Buffer::create(vk::BufferUsageFlagBits::eTransferSrc, sizeof(lnm::mat4), VMA_MEMORY_USAGE_AUTO, VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT);
			resources.modelBuffer = vulkan::Buffer::create(vk::BufferUsageFlagBits::eUniformBuffer | vk::BufferUsageFlagBits::eTransferDst, sizeof(lnm::mat4), VMA_MEMORY_USAGE_AUTO, VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT);

			resources.descSets = std::move(vulkan::DescriptorSets::create(mPipeline, 1));
			resources.descSets->setBufferInfo("viewProj", 0, cameraSystem->getViewProjectionBuffer()->getBuffer(), 0, cameraSystem->getViewProjectionBuffer()->getSize());
			resources.descSets->setBufferInfo("model", 0, resources.modelBuffer->getBuffer(), 0, resources.modelBuffer->getSize());
			resources.descSets->updateSets(0);

			it = mResources.emplace(meshComponent, std::move(resources)).first;
			res = &it->second;
		}
		else
		{
			res = &it->second;
		}

		auto& resources = mResources.find(meshComponent)->second;

		lnm::mat4 model = lnm::mat4(1.f);
		auto transformComp = entity->findComponent<TransformComponent>();
		if (transformComp)
			model = lnm::translate(model, transformComp->mPosition) * lnm::mat4(transformComp->mOrientation) * lnm::scale(model, transformComp->mScale);

		int diff{};
		uint8* pStageBuffer = res->stagingModelBuffer->map();
		if (diff = memcmp(pStageBuffer, &model, sizeof(model)); diff != 0)
			memcpy(pStageBuffer, &model, sizeof(model));
		res->stagingModelBuffer->unmap();

		if (diff)
		{
			const vk::BufferCopy bufferCopy = vk::BufferCopy().setSize(sizeof(model));
			commandBuffer.copyBuffer(res->stagingModelBuffer->getBuffer(), res->modelBuffer->getBuffer(), bufferCopy);
		}
	}
}

void lune::MeshRenderSystem::render(class Scene* scene)
{
	const auto& entities = scene->getEntities();
	auto vkSubsystem = Engine::get()->findSubsystem<VulkanSubsystem>();
	auto [viewId, imageIndex, commandBuffer] = vkSubsystem->getFrameInfo();

	auto cameraSystem = scene->findSystem<CameraSystem>();
	if (!cameraSystem)
		return;

	const auto& eIds = scene->getComponentEntities<MeshComponent>();
	for (uint64 eId : eIds)
	{
		auto entity = scene->findEntity(eId);
		if (!entity)
			continue;

		auto comp = entity->findComponent<MeshComponent>();
		if (!comp)
			continue;

		if (auto findRes = mResources.find(comp); findRes != mResources.end())
		{
			const auto& [comp, res] = *findRes;

			res.descSets->getPipeline()->cmdBind(commandBuffer);

			commandBuffer.setPrimitiveTopology(vk::PrimitiveTopology::eTriangleList);

			res.descSets->cmdBind(commandBuffer, 0);

			for (auto& primitive : res.primitives)
			{
				primitive->cmdBind(commandBuffer);
				primitive->cmdDraw(commandBuffer);
			}
		}
	}
}