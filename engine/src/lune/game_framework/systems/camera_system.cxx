#include "lune/game_framework/systems/camera_system.hxx"

#include "lune/game_framework/components/isometric_camera.hxx"
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
			lnm::vec3 position{};

			auto transformComp = e->findComponent<TransformComponent>();
			if (transformComp)
			{
				position = transformComp->mPosition;
			}

			position += isoCam->mPosition;
			const auto& direction = isoCam->mDirection;
			const auto& up = isoCam->mUp;

			mView = isoCam->getView();
			mProjection = isoCam->getProjection();

			const lnm::mat4 clip = lnm::mat4{
				1.0f, 0.0f, 0.0f, 0.0f,
				0.0f, -1.0f, 0.0f, 0.0f,
				0.0f, 0.0f, 0.5f, 0.0f,
				0.0f, 0.0f, 0.5f, 1.0f};

			mViewProjection = clip * mProjection * mView;

			break;
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

	bool wasCopy = false;

	uint8* stageBuffer = stagingCameraBuffer->map();
	if (memcmp(stageBuffer, &mViewProjection, sizeof(mViewProjection)) != 0)
	{
		memcpy(stageBuffer, &mViewProjection, sizeof(mViewProjection));
		wasCopy = true;
	}
	stagingCameraBuffer->unmap();

	if (wasCopy)
	{
		const vk::BufferCopy bufferCopy = vk::BufferCopy().setSize(sizeof(mViewProjection));
		commandBuffer.copyBuffer(stagingCameraBuffer->getBuffer(), cameraBuffer->getBuffer(), bufferCopy);
	}
}

void lune::CameraSystem::render(vk::CommandBuffer commandBuffer, Scene* scene)
{
}
