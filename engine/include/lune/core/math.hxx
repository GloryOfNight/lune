#pragma once

#include "glm/ext/matrix_clip_space.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "glm/ext/scalar_constants.hpp"
#include "glm/glm.hpp"
#include "glm/gtc/quaternion.hpp"
#include "glm/mat4x4.hpp"
#include "glm/vec3.hpp"
#include "glm/vec4.hpp"

namespace lune
{
	namespace math = glm;
}

namespace lnm = lune::math;

namespace lune
{
	static constexpr lnm::vec3 frontAxis = lnm::vec3(0.f, 0.f, 1.f);
	static constexpr lnm::vec3 rightAxis = lnm::vec3(1.f, 0.f, 0.f);
	static constexpr lnm::vec3 upAxis = lnm::vec3(0.f, -1.f, 0.f);
} // namespace lune