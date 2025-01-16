#pragma once

#include "lune/lune.hxx"

#include <memory>
#include <vector>

namespace lune
{
	class SystemBase
	{
	public:
		SystemBase() = default;
		SystemBase(const SystemBase&) = delete;
		SystemBase(SystemBase&&) = default;
		virtual ~SystemBase() = default;

		virtual void update(class Scene* scene, double deltaTime) = 0;
	};

	class PrepareRenderSystemInterface
	{
	public:
		virtual void prepareRender(class Scene* scene) = 0;
	};

	class RenderSystemInterface
	{
	public:
		virtual void render(class Scene* scene) = 0;
	};
} // namespace lune