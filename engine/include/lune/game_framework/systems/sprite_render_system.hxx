#pragma once
#include "lune/vulkan/buffer.hxx"
#include "lune/vulkan/descriptor_sets.hxx"
#include "lune/vulkan/vulkan_core.hxx"

#include "camera_system.hxx"
#include "system.hxx"

namespace lune
{
	class SpriteRenderSystem
		: public SystemBase,
		  public PrepareRenderSystemInterface,
		  public RenderSystemInterface
	{
	public:
		SpriteRenderSystem()
		{
			addDependecy<CameraSystem>();
		}

		virtual void update(class Scene* scene, double deltaTime) override;

		virtual void prepareRender(class Scene* scene) override;

		virtual void render(class Scene* scene) override;

	private:
		vulkan::SharedPrimitive mPrimitive{};
		vulkan::SharedGraphicsPipeline mPipeline{};
		vulkan::SharedSampler mSampler{};

		struct SpriteResources
		{
			vulkan::SharedTextureImage texImage{};
			vulkan::UniqueDescriptorSets descSets{};
			vulkan::UniqueBuffer stagingModelBuffer{};
			vulkan::UniqueBuffer modelBuffer{};
		};
		std::unordered_map<class SpriteComponent*, SpriteResources> mResources{};
	};
} // namespace lune