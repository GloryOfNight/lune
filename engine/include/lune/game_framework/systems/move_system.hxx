#pragma once

#include "system.hxx"

namespace lune
{
	class MoveSystem : public SystemBase
	{
	public:
		virtual void update(const std::vector<std::shared_ptr<Entity>>& entities, double deltaTime) override;
	};
} // namespace lune