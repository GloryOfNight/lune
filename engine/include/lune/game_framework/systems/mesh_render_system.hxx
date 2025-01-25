#pragma once

#include "system.hxx"

#include "lune/game_framework/components/mesh.hxx"
#include "lune/vulkan/vulkan_core.hxx"
#include "lune/vulkan/buffer.hxx"
#include "lune/vulkan/descriptor_sets.hxx"

namespace lune
{
	class MeshRenderSystem : public SystemBase, public PrepareRenderSystemInterface, public RenderSystemInterface
	{
	public:
		MeshRenderSystem();

		virtual void update(class Scene* scene, double deltaTime) override {};
		virtual void prepareRender(class Scene* scene) override;
		virtual void render(class Scene* scene) override;

	private:
		vulkan::SharedGraphicsPipeline mPipeline{};
		struct MeshResources
		{
			std::vector<vulkan::SharedPrimitive> primitives{};
			vulkan::UniqueDescriptorSets descSets{};
			vulkan::UniqueBuffer stagingModelBuffer{};
			vulkan::UniqueBuffer modelBuffer{};
		};
		std::unordered_map<class MeshComponent*, MeshResources> mResources{};
	};
} // namespace lune