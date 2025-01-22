#pragma once

#include "lune/vulkan/descriptor_sets.hxx"

#include "system.hxx"

namespace lune
{
	class GizmoSystem : public SystemBase, public RenderSystemInterface
	{
	public:
		GizmoSystem();

		virtual void update(class Scene* scene, double deltaTime) override {};

		virtual void render(class Scene* scene) override;

	private:
		vulkan::UniqueDescriptorSets mDescriptorSets{};
	};
} // namespace lune