#pragma once

#include "lune/vulkan/descriptor_sets.hxx"
#include "lune/vulkan/vulkan_core.hxx"

#include "system.hxx"

namespace lune
{
	class SkyboxSystem
		: public SystemBase,
		  public PrepareRenderSystemInterface,
		  public RenderSystemInterface
	{
	public:
		SkyboxSystem();

        virtual void update(class Scene* scene, double deltaTime) override{};

		virtual void prepareRender(class Scene* scene) override;

		virtual void render(class Scene* scene) override;

	private:
		vulkan::SharedPrimitive mBox{};
		vulkan::SharedGraphicsPipeline mPipeline{};
		struct SkyboxResources
		{
			vulkan::SharedTextureImage mTextureImage{};
			vulkan::UniqueDescriptorSets mDescriptorSets{};
		};
        std::map<uint64, SkyboxResources> mSkyboxes{};
	};
} // namespace lune