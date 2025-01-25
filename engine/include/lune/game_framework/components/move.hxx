#pragma once

#include "lune/core/math.hxx"

#include "component.hxx"

namespace lune
{
	struct MoveComponent : public ComponentBase
	{
		float speed{10.f};
	};
} // namespace lune