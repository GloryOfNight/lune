#pragma once

#include "glm/glm.hpp"
#include "glm/gtc/quaternion.hpp"
#include "glm/mat4x4.hpp"
#include "glm/vec2.hpp"
#include "glm/vec3.hpp"
#include "glm/vec4.hpp"
#include "lune/lune.hxx"

namespace lune
{
	namespace math = glm;
}

namespace lnm = lune::math;

namespace lune
{
	static constexpr lnm::vec3 forwardAxis = lnm::vec3(0.f, 0.f, 1.f);
	static constexpr lnm::vec3 rightAxis = lnm::vec3(1.f, 0.f, 0.f);
	static constexpr lnm::vec3 upAxis = lnm::vec3(0.f, 1.f, 0.f);

	struct Vertex3
	{
		lnm::vec3 position{};
	};

	struct Vertex32
	{
		lnm::vec3 position{};
		lnm::vec2 uv{};
	};

	struct Vertex33
	{
		lnm::vec3 position{};
		lnm::vec3 normal{};
	};

	struct Vertex34
	{
		lnm::vec3 position{};
		lnm::vec4 color{};
	};

	struct Vertex342
	{
		lnm::vec3 position{};
		lnm::vec4 color{};
		lnm::vec2 uv{};
	};

	struct Vertex343224
	{
		lnm::vec3 position{};
		lnm::vec4 tangent{};
		lnm::vec3 normal{};
		lnm::vec2 uv0{};
		lnm::vec2 uv1{};
		lnm::vec4 color0{};
	};

	using Index16 = uint16;
	using Index32 = uint32;

	using Vertex = Vertex342;
	using Index = Index32;
} // namespace lune