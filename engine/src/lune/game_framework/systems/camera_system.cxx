#include "lune/game_framework/systems/camera_system.hxx"

#include "lune/core/engine.hxx"
#include "lune/game_framework/components/camera.hxx"
#include "lune/game_framework/components/transform.hxx"
#include "lune/game_framework/entities/entity.hxx"
#include "lune/game_framework/scene.hxx"
#include "lune/vulkan/vulkan_subsystem.hxx"

void lune::CameraSystem::update(Scene* scene, double deltaTime)
{
	mViewsProjs.clear();

	const auto& eIds = scene->getComponentEntities<PerspectiveCameraComponent>();
	for (auto eId : eIds)
	{
		auto entity = scene->findEntity(eId);
		if (!entity)
			continue;

		auto persCam = entity->findComponent<PerspectiveCameraComponent>();
		if (!persCam)
			continue;

		lnm::vec3 position = persCam->mPosition;
		lnm::quat rotation = glm::quat(1.f, 0.f, 0.f, 0.f);
		auto transformComp = entity->findComponent<TransformComponent>();
		if (transformComp)
		{
			position += transformComp->mPosition;
			rotation = transformComp->mOrientation;
		}

		auto vkSubsystem = Engine::get()->findSubsystem<VulkanSubsystem>();
		const auto& viewIds = Engine::get()->getViewIds();
		for (uint32 viewId : viewIds)
		{
			auto [it, result] = mViewsProjs.try_emplace(viewId, ViewProj());
			auto& viewProj = it->second;

			auto currentExtent = vkSubsystem->findView(viewId)->getCurrentExtent();
			float viewAspectRatio = static_cast<float>(currentExtent.width) / static_cast<float>(currentExtent.height);

			float fov, aspectRatio, nearPlane, farPlane;
			persCam->getPerspective(fov, aspectRatio, nearPlane, farPlane);

			const lnm::vec3 forward = rotation * forwardAxis;
			const lnm::vec3 up = rotation * -upAxis;

			viewProj.view = lnm::lookAtRH(position, (position + forward), up);
			//viewProj.view = glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, -1.0f, -1.0f)) * viewProj.view;
			viewProj.proj = lnm::perspective(lnm::radians(fov), aspectRatio * viewAspectRatio, nearPlane, farPlane);
			viewProj.viewProj = viewProj.proj * viewProj.view;
		}
	}
}

void lune::CameraSystem::prepareRender(Scene* scene)
{
	const auto& entities = scene->getEntities();
	auto vkSubsystem = Engine::get()->findSubsystem<VulkanSubsystem>();
	auto [viewId, imageIndex, commandBuffer] = vkSubsystem->getFrameInfo();

	if (!mViewProjStagingBuffer)
		mViewProjStagingBuffer = vulkan::Buffer::create(vk::BufferUsageFlagBits::eTransferSrc, sizeof(ViewProj), VMA_MEMORY_USAGE_AUTO, VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT);

	if (!mViewProjBuffer)
		mViewProjBuffer = vulkan::Buffer::create(vk::BufferUsageFlagBits::eUniformBuffer | vk::BufferUsageFlagBits::eTransferDst, sizeof(lnm::mat4), VMA_MEMORY_USAGE_AUTO, VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT);
	if (!mViewBuffer)
		mViewBuffer = vulkan::Buffer::create(vk::BufferUsageFlagBits::eUniformBuffer | vk::BufferUsageFlagBits::eTransferDst, sizeof(lnm::mat4), VMA_MEMORY_USAGE_AUTO, VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT);
	if (!mProjBuffer)
		mProjBuffer = vulkan::Buffer::create(vk::BufferUsageFlagBits::eUniformBuffer | vk::BufferUsageFlagBits::eTransferDst, sizeof(lnm::mat4), VMA_MEMORY_USAGE_AUTO, VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT);

	auto findRes = mViewsProjs.find(viewId);
	if (findRes == mViewsProjs.end())
		return;

	uint32 diff{};

	uint8* pStageBuffer = mViewProjStagingBuffer->map();

	if (uint32 memdiff = memcmp(pStageBuffer, &findRes->second.viewProj, sizeof(lnm::mat4)); memdiff != 0)
	{
		memcpy(pStageBuffer, &findRes->second.viewProj, sizeof(lnm::mat4));
		diff += memdiff;
	}
	if (uint32 memdiff = memcmp(pStageBuffer + offsetof(ViewProj, view), &findRes->second.view, sizeof(lnm::mat4)); memdiff != 0)
	{
		memcpy(pStageBuffer + offsetof(ViewProj, view), &findRes->second.view, sizeof(lnm::mat4));
		diff += memdiff;
	}
	if (uint32 memdiff = memcmp(pStageBuffer + offsetof(ViewProj, proj), &findRes->second.proj, sizeof(lnm::mat4)); memdiff != 0)
	{
		memcpy(pStageBuffer + offsetof(ViewProj, proj), &findRes->second.proj, sizeof(lnm::mat4));
		diff += memdiff;
	}

	mViewProjStagingBuffer->unmap();

	if (diff)
	{
		{
			const vk::BufferCopy bufferCopy = vk::BufferCopy()
												  .setSrcOffset(0)
												  .setSize(sizeof(lnm::mat4));
			commandBuffer.copyBuffer(mViewProjStagingBuffer->getBuffer(), mViewProjBuffer->getBuffer(), bufferCopy);
		}
		{
			const vk::BufferCopy bufferCopy = vk::BufferCopy()
												  .setSrcOffset(offsetof(ViewProj, view))
												  .setSize(sizeof(lnm::mat4));
			commandBuffer.copyBuffer(mViewProjStagingBuffer->getBuffer(), mViewBuffer->getBuffer(), bufferCopy);
		}
		{
			const vk::BufferCopy bufferCopy = vk::BufferCopy()
												  .setSrcOffset(offsetof(ViewProj, proj))
												  .setSize(sizeof(lnm::mat4));
			commandBuffer.copyBuffer(mViewProjStagingBuffer->getBuffer(), mProjBuffer->getBuffer(), bufferCopy);
		}
	}
}