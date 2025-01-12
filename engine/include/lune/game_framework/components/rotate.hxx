#pragma once

#include "lune/core/math.hxx"

#include "component.hxx"

namespace lune
{
	struct RotateComponent : public ComponentBase
	{
		float speed{45.f};

		lnm::vec3 roll = lnm::vec3(0.f, 0.f, 1.f);
		lnm::vec3 pitch = lnm::vec3(1.f, 0.f, 0.f);
		lnm::vec3 yaw = lnm::vec3(0.f, 1.f, 0.f);
	};
} // namespace lune