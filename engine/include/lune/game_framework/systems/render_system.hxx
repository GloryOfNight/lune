#pragma once

#include "system.hxx"

namespace lune
{
	class RenderSystem : public SystemBase
	{
	public:
		virtual void render(const std::vector<std::shared_ptr<Entity>>& entities) = 0;
	};
} // namespace lune