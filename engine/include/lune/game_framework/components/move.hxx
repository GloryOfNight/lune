#pragma once

#include "lune/core/math.hxx"

#include "component.hxx"

namespace lune
{
	struct MoveComponent : public ComponentBase
	{
		float speed{1.f};
	};
} // namespace lune