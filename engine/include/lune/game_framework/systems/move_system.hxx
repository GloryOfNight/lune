#pragma once

#include "system.hxx"

namespace lune
{
	// first-person like movement
	class MoveSystem : public SystemBase
	{
	public:
		virtual void update(class Scene* scene, double deltaTime) override;
	};
} // namespace lune