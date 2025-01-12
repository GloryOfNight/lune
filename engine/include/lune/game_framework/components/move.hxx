#pragma once

#include "lune/core/math.hxx"

#include "component.hxx"

namespace lune
{
	struct MoveComponent : public ComponentBase
	{
		float speed{1.f};
		float rotSpeed{45.f};

		lnm::vec3 front = lnm::vec3(0.f, 0.f, 1.f);
		lnm::vec3 left = lnm::vec3(1.f, 0.f, 0.f);
		lnm::vec3 up = lnm::vec3(0.f, 1.f, 0.f);

        lnm::vec3 roll = lnm::vec3(0.f, 0.f, 1.f);
        lnm::vec3 pitch = lnm::vec3(1.f, 0.f, 0.f);
        lnm::vec3 yaw = lnm::vec3(0.f, 1.f, 0.f);
	};
} // namespace lune