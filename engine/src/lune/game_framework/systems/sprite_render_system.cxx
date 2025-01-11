#include "lune/game_framework/systems/sprite_render_system.hxx"

#include "lune/game_framework/components/sprite.hxx"
#include "lune/game_framework/components/transform.hxx"
#include "lune/game_framework/entities/entity.hxx"
#include "lune/game_framework/scene.hxx"
#include "lune/game_framework/systems/camera_system.hxx"
#include "lune/vulkan/pipeline.hxx"
#include "lune/vulkan/primitive.hxx"
#include "lune/vulkan/texture_image.hxx"
#include "lune/vulkan/vulkan_subsystem.hxx"

void lune::SpriteRenderSystem::update(const std::vector<std::shared_ptr<Entity>>& entities, double deltaTime)
{
}

void lune::SpriteRenderSystem::beforeRender(vk::CommandBuffer commandBuffer, Scene* scene)
{
	const auto& entities = scene->getEntities();
	auto vkSubsystem = vulkan_subsystem::get();

	auto cameraSystem = scene->findSystem<CameraSystem>();
	if (!cameraSystem)
		return;

	for (auto& entity : entities)
	{
		auto spriteComp = entity->findComponent<SpriteComponent>();
		if (spriteComp)
		{
			SpriteResources* res{};
			if (auto findRes = mResources.find(spriteComp); findRes == mResources.end())
			{
				SpriteResources resources{};

				resources.primitive = vkSubsystem->findPrimitive("lune::plane");
				auto pipeline = vkSubsystem->findPipeline("lune::sprite");

				resources.texImage = vkSubsystem->findTextureImage(spriteComp->imageName);
				if (pipeline == nullptr || resources.texImage == nullptr)
					continue;

				resources.stagingModelBuffer = vulkan::Buffer::create(vk::BufferUsageFlagBits::eTransferSrc, sizeof(lnm::mat4), VMA_MEMORY_USAGE_AUTO, VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT);
				resources.modelBuffer = vulkan::Buffer::create(vk::BufferUsageFlagBits::eUniformBuffer | vk::BufferUsageFlagBits::eTransferDst, sizeof(lnm::mat4), VMA_MEMORY_USAGE_AUTO, VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT);

				resources.descSets = vulkan::DescriptorSets::create(pipeline, 3);
				resources.descSets->setBufferInfo("viewProj", 0, cameraSystem->getViewProjectionBuffer()->getBuffer(), 0, sizeof(lnm::mat4));
				resources.descSets->setBufferInfo("model", 0, resources.modelBuffer->getBuffer(), 0, sizeof(lnm::mat4));
				resources.descSets->setImageInfo("texSampler", 0, resources.texImage->getImageView(), resources.texImage->getSampler());
				resources.descSets->updateSets(0);

				const auto [it, result] = mResources.emplace(spriteComp, std::move(resources));
				res = &it->second;
			}
			else
			{
				res = &findRes->second;
			}

			lnm::mat4 model = lnm::translate(lnm::mat4(1.0f), spriteComp->position);
			auto transformComp = entity->findComponent<TransformComponent>();
			if (transformComp)
				model *= transformComp->getTransform();

			bool bUpdateModel = false;
			{
				uint8* stageBuffer = res->stagingModelBuffer->map();
				if (memcmp(stageBuffer, &model, sizeof(model)) != 0)
				{
					memcpy(stageBuffer, &model, sizeof(model));
					bUpdateModel = true;
				}
				res->stagingModelBuffer->unmap();
			}

			if (bUpdateModel)
			{
				const vk::BufferCopy bufferCopy = vk::BufferCopy().setSize(sizeof(model));
				commandBuffer.copyBuffer(res->stagingModelBuffer->getBuffer(), res->modelBuffer->getBuffer(), bufferCopy);
			}
		}
	}
}

void lune::SpriteRenderSystem::render(vk::CommandBuffer commandBuffer, Scene* scene)
{
	const auto& entities = scene->getEntities();
	auto vkSubsystem = vulkan_subsystem::get();

	auto cameraSystem = scene->findSystem<CameraSystem>();
	if (!cameraSystem)
		return;

	for (auto& entity : entities)
	{
		auto spriteComp = entity->findComponent<SpriteComponent>();
		if (spriteComp)
		{
			if (auto findRes = mResources.find(spriteComp); findRes != mResources.end())
			{
				const auto& [comp, res] = *findRes;

				res.descSets->getPipeline()->cmdBind(commandBuffer);
				res.descSets->cmdBind(commandBuffer, 0);
				res.primitive->cmdBind(commandBuffer);
				res.primitive->cmdDraw(commandBuffer);
			}
		}
	}
}
