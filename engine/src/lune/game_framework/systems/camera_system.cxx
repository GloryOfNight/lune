#include "lune/game_framework/systems/camera_system.hxx"

#include "lune/game_framework/components/camera.hxx"
#include "lune/game_framework/components/transform.hxx"
#include "lune/game_framework/entities/entity.hxx"
#include "lune/game_framework/scene.hxx"
#include "lune/vulkan/vulkan_subsystem.hxx"

void lune::CameraSystem::update(const std::vector<std::shared_ptr<lune::Entity>>& entities, double deltaTime)
{
	for (const auto& e : entities)
	{
		auto isoCam = e->findComponent<IsometricCameraComponent>();
		if (isoCam)
		{
			lnm::vec3 position = isoCam->mPosition;
			auto transformComp = e->findComponent<TransformComponent>();
			if (transformComp)
				position += transformComp->mPosition;

			const auto& direction = isoCam->mDirection;
			const auto& up = isoCam->mUp;

			float left, right, bottom, top, near, far;
			isoCam->getOrtho(left, right, bottom, top, near, far);

			mView = lnm::lookAt(position, position - direction, up);
			mProjection = lnm::ortho(left, right, bottom, top, near, far);

			mViewProjection = mProjection * mView;

			break;
		}

		auto persCam = e->findComponent<PerspectiveCameraComponent>();
		if (persCam)
		{
			lnm::vec3 position = persCam->mPosition;
			auto transformComp = e->findComponent<TransformComponent>();
			if (transformComp)
				position += transformComp->mPosition;

			const auto& direction = persCam->mDirection;
			const auto& up = persCam->mUp;

			float fov, aspectRatio, nearPlane, farPlane;
			persCam->getPerspective(fov, aspectRatio, nearPlane, farPlane);

			mView = lnm::lookAt(position, position - direction, up);
			mProjection = lnm::perspective(lnm::radians(fov), aspectRatio, nearPlane, farPlane);

			mViewProjection = mProjection * mView;
		}
	}
}
void lune::CameraSystem::beforeRender(vk::CommandBuffer commandBuffer, Scene* scene)
{
	const auto& entities = scene->getEntities();
	auto vkSubsystem = vulkan_subsystem::get();

	if (!stagingCameraBuffer)
		stagingCameraBuffer = vulkan::Buffer::create(vk::BufferUsageFlagBits::eTransferSrc, sizeof(lnm::mat4), VMA_MEMORY_USAGE_AUTO, VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT);
	if (!cameraBuffer)
		cameraBuffer = vulkan::Buffer::create(vk::BufferUsageFlagBits::eUniformBuffer | vk::BufferUsageFlagBits::eTransferDst, sizeof(lnm::mat4), VMA_MEMORY_USAGE_AUTO, VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT);

	int diff{};
	uint8* stageBuffer = stagingCameraBuffer->map();
	if (diff = memcmp(stageBuffer, &mViewProjection, sizeof(mViewProjection)); diff != 0)
	{
		memcpy(stageBuffer, &mViewProjection, sizeof(mViewProjection));
	}
	stagingCameraBuffer->unmap();

	if (diff)
	{
		const vk::BufferCopy bufferCopy = vk::BufferCopy().setSize(sizeof(mViewProjection));
		commandBuffer.copyBuffer(stagingCameraBuffer->getBuffer(), cameraBuffer->getBuffer(), bufferCopy);
	}
}

void lune::CameraSystem::render(vk::CommandBuffer commandBuffer, Scene* scene)
{
}
