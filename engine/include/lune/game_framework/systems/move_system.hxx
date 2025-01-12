#pragma once

#include "system.hxx"

namespace lune
{
	class MoveSystem : public SystemBase
	{
	public:
		virtual void update(class Scene* scene, double deltaTime) override;
	};
} // namespace lune