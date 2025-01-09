#pragma once
#include "render_system.hxx"

namespace lune
{
	class SpriteRenderSystem : public RenderSystem
	{
	public:
        virtual void update(const std::vector<std::shared_ptr<Entity>>& entities, double deltaTime) override;

		virtual void render(const std::vector<std::shared_ptr<Entity>>& entities) override;
	};
} // namespace lune