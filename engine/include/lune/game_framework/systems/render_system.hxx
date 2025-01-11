#pragma once

#include "vulkan/vulkan.hpp"

#include "system.hxx"

namespace lune
{
	class RenderSystem : public SystemBase
	{
	public:
		virtual void beforeRender(vk::CommandBuffer commandBuffer, class Scene* scene) = 0;
		virtual void render(vk::CommandBuffer commandBuffer, class Scene* scene) = 0;
	};
} // namespace lune