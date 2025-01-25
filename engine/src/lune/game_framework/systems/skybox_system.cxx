#include "lune/game_framework/systems/skybox_system.hxx"

#include "lune/core/engine.hxx"
#include "lune/game_framework/components/skybox.hxx"
#include "lune/game_framework/scene.hxx"
#include "lune/game_framework/systems/camera_system.hxx"
#include "lune/vulkan/pipeline.hxx"
#include "lune/vulkan/primitive.hxx"
#include "lune/vulkan/sampler.hxx"
#include "lune/vulkan/texture_image.hxx"
#include "lune/vulkan/vulkan_core.hxx"
#include "lune/vulkan/vulkan_subsystem.hxx"

lune::SkyboxSystem::SkyboxSystem()
{
	addDependecy<CameraSystem>();
}

void lune::SkyboxSystem::prepareRender(class Scene* scene)
{
	auto vkSubsystem = Engine::get()->findSubsystem<VulkanSubsystem>();
	auto cameraSystem = scene->findSystem<CameraSystem>();

	if (!mBox)
		mBox = vkSubsystem->findPrimitive("lune::skybox");

	if (!mPipeline)
		mPipeline = vkSubsystem->findPipeline("lune::skybox");

	if (!mSampler)
		mSampler = vkSubsystem->findSampler("lune::linear");

	const auto& eIds = scene->getComponentEntities<SkyboxComponent>();
	for (uint64 eId : eIds)
	{
		if (mSkyboxes.find(eId) == mSkyboxes.end())
		{
			auto entity = scene->findEntity(eId);
			auto skyboxComp = entity->findComponent<SkyboxComponent>();

			SkyboxResources resources;
			resources.mTextureImage = vkSubsystem->findTextureImage(skyboxComp->imageName);

			resources.mDescriptorSets = vulkan::DescriptorSets::create(mPipeline, 1);
			resources.mDescriptorSets->setBufferInfo("view", 0, cameraSystem->getViewBuffer()->getBuffer(), 0, sizeof(lnm::mat4));
			resources.mDescriptorSets->setBufferInfo("proj", 0, cameraSystem->getProjectionBuffer()->getBuffer(), 0, sizeof(lnm::mat4));
			resources.mDescriptorSets->setImageInfo("cubemap", 0, resources.mTextureImage->getImageView(), mSampler->getSampler());
			resources.mDescriptorSets->updateSets(0);

			mSkyboxes.emplace(eId, std::move(resources));
		}
	}
}

void lune::SkyboxSystem::render(class Scene* scene)
{
	auto vkSubsystem = Engine::get()->findSubsystem<VulkanSubsystem>();
	auto [viewId, imageIndex, commandBuffer] = vkSubsystem->getFrameInfo();

	for (const auto& [eId, skybox] : mSkyboxes)
	{
		mBox->cmdBind(commandBuffer);
		mPipeline->cmdBind(commandBuffer);
		skybox.mDescriptorSets->cmdBind(commandBuffer, 0);

		commandBuffer.setDepthTestEnableEXT(false, vulkan::getDynamicLoader());
		mBox->cmdDraw(commandBuffer);
		commandBuffer.setDepthTestEnableEXT(true, vulkan::getDynamicLoader());
	}
}