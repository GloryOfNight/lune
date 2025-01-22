#include "lune/game_framework/systems/gizmo_system.hxx"

#include "lune/core/engine.hxx"
#include "lune/game_framework/scene.hxx"
#include "lune/game_framework/systems/camera_system.hxx"
#include "lune/vulkan/pipeline.hxx"
#include "lune/vulkan/primitive.hxx"
#include "lune/vulkan/vulkan_subsystem.hxx"

lune::GizmoSystem::GizmoSystem()
{
	addDependecy<CameraSystem>();
}

void lune::GizmoSystem::render(Scene* scene)
{
	auto cameraSystem = scene->findSystem<CameraSystem>();
	if (!cameraSystem)
		return;

	auto vkSubsystem = Engine::get()->findSubsystem<VulkanSubsystem>();
	auto [viewId, imageIndex, commandBuffer] = vkSubsystem->getFrameInfo();

	auto gizmoX = vkSubsystem->findPrimitive("lune::gizmoX");
	auto gizmoY = vkSubsystem->findPrimitive("lune::gizmoY");
	auto gizmoZ = vkSubsystem->findPrimitive("lune::gizmoZ");
	auto pipeline = vkSubsystem->findPipeline("lune::gizmo");

	if (!mDescriptorSets)
	{
		mDescriptorSets = vulkan::DescriptorSets::create(pipeline, 1);
		mDescriptorSets->setBufferInfo("viewProj", 0, cameraSystem->getViewProjectionBuffer()->getBuffer(), 0, sizeof(lnm::mat4));
		mDescriptorSets->updateSets(0);
	}

    pipeline->cmdBind(commandBuffer);

	mDescriptorSets->cmdBind(commandBuffer, 0);

	gizmoX->cmdBind(commandBuffer);
	gizmoX->cmdDraw(commandBuffer);

	gizmoY->cmdBind(commandBuffer);
	gizmoY->cmdDraw(commandBuffer);

	gizmoZ->cmdBind(commandBuffer);
	gizmoZ->cmdDraw(commandBuffer);
}