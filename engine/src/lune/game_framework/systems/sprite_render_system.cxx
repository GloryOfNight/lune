#include "lune/game_framework/systems/sprite_render_system.hxx"

#include "lune/core/engine.hxx"
#include "lune/game_framework/components/sprite.hxx"
#include "lune/game_framework/components/transform.hxx"
#include "lune/game_framework/entities/entity.hxx"
#include "lune/game_framework/scene.hxx"
#include "lune/game_framework/systems/camera_system.hxx"
#include "lune/vulkan/pipeline.hxx"
#include "lune/vulkan/primitive.hxx"
#include "lune/vulkan/sampler.hxx"
#include "lune/vulkan/texture_image.hxx"
#include "lune/vulkan/vulkan_subsystem.hxx"
#include <vulkan/vulkan_handles.hpp>

void lune::SpriteRenderSystem::update(Scene* scene, double deltaTime)
{
}

void lune::SpriteRenderSystem::prepareRender(Scene* scene)
{
	const auto& entities = scene->getEntities();
	auto vkSubsystem = Engine::get()->findSubsystem<VulkanSubsystem>();
	vk::CommandBuffer commandBuffer = vkSubsystem->getFrameInfo().copyCommandBuffer;

	auto cameraSystem = scene->findSystem<CameraSystem>();
	if (!cameraSystem)
		return;

	if (!mPrimitive)
		mPrimitive = vkSubsystem->findPrimitive("lune::plane");

	if (!mPipeline)
		mPipeline = vkSubsystem->findPipeline("lune::sprite");

	if (!mSampler)
		mSampler = vkSubsystem->findSampler("lune::nearest");

	const auto& eIds = scene->getComponentEntities<SpriteComponent>();
	for (auto eId : eIds)
	{
		auto entity = scene->findEntity(eId);
		if (!entity)
			continue;

		auto spriteComp = entity->findComponent<SpriteComponent>();
		if (!spriteComp)
			continue;

		SpriteResources* res{};
		if (auto findRes = mResources.find(spriteComp); findRes == mResources.end())
		{
			SpriteResources resources{};

			auto pipeline = vkSubsystem->findPipeline("lune::sprite");

			resources.texImage = vkSubsystem->findTextureImage(spriteComp->imageName);
			if (!resources.texImage)
				continue;

			resources.stagingModelBuffer = vulkan::Buffer::create(vk::BufferUsageFlagBits::eTransferSrc, sizeof(lnm::mat4), VMA_MEMORY_USAGE_AUTO, VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT);
			resources.modelBuffer = vulkan::Buffer::create(vk::BufferUsageFlagBits::eUniformBuffer | vk::BufferUsageFlagBits::eTransferDst, sizeof(lnm::mat4), VMA_MEMORY_USAGE_AUTO, VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT);

			resources.descSets = vulkan::DescriptorSets::create(pipeline, 1);
			resources.descSets->setBufferInfo("viewProj", 0, cameraSystem->getViewProjectionBuffer()->getBuffer(), 0, sizeof(lnm::mat4));
			resources.descSets->setBufferInfo("model", 0, resources.modelBuffer->getBuffer(), 0, sizeof(lnm::mat4));
			resources.descSets->setImageInfo("texSampler", 0, resources.texImage->getImageView(), mSampler->getSampler());
			resources.descSets->updateSets(0);

			const auto [it, result] = mResources.emplace(spriteComp, std::move(resources));
			res = &it->second;
		}
		else
		{
			res = &findRes->second;
		}

		lnm::mat4 model = lnm::mat4(1.f);
		auto transformComp = entity->findComponent<TransformComponent>();
		if (transformComp)
			model = lnm::translate(model, transformComp->mPosition) * lnm::mat4(transformComp->mOrientation) * lnm::scale(model, transformComp->mScale);

		model = lnm::translate(model, spriteComp->position);

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

void lune::SpriteRenderSystem::render(Scene* scene)
{
	const auto& entities = scene->getEntities();
	auto vkSubsystem = Engine::get()->findSubsystem<VulkanSubsystem>();
	vk::CommandBuffer commandBuffer = vkSubsystem->getFrameInfo().renderCommandBuffer;

	auto cameraSystem = scene->findSystem<CameraSystem>();
	if (!cameraSystem)
		return;

	const auto& eIds = scene->getComponentEntities<SpriteComponent>();
	for (auto eId : eIds)
	{
		auto entity = scene->findEntity(eId);
		if (!entity)
			continue;

		auto spriteComp = entity->findComponent<SpriteComponent>();
		if (!spriteComp)
			continue;
		if (auto findRes = mResources.find(spriteComp); findRes != mResources.end())
		{
			const auto& [comp, res] = *findRes;

			res.descSets->getPipeline()->cmdBind(commandBuffer);
			res.descSets->cmdBind(commandBuffer, 0);
			mPrimitive->cmdBind(commandBuffer);
			mPrimitive->cmdDraw(commandBuffer);
		}
	}
}
