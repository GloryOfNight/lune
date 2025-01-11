#pragma once

#include "vulkan/vulkan.hpp"

#include "system.hxx"

namespace lune
{
	class RenderSystem : public SystemBase
	{
	public:
		virtual void prepareRender(class Scene* scene) = 0;
		virtual void render(class Scene* scene) = 0;
	};
} // namespace lune