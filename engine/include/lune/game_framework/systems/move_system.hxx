#pragma once

#include "input_system.hxx"
#include "system.hxx"

namespace lune
{
	// first-person like movement
	class MoveSystem : public SystemBase
	{
	public:
		MoveSystem()
		{
			addDependecy<InputSystem>();
		}
		
		virtual void update(class Scene* scene, double deltaTime) override;
	};
} // namespace lune