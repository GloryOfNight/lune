#pragma once

#include "lune/core/math.hxx"

#include "component.hxx"

namespace lune
{
	struct TransformComponent : public ComponentBase
	{
		void translate(const lnm::vec3& translation) { mPosition += translation; }
		void move(const lnm::vec3& translation) { mPosition += mOrientation * translation; }
		void rotate(const lnm::quat& rotation) { mOrientation = lnm::normalize(rotation * mOrientation); }
		void rotate(float rads, const lnm::vec3& axis) { rotate(lnm::angleAxis(rads, glm::normalize(axis))); };
		void scale(const lnm::vec3& scaling) { mScale *= scaling; }

		lnm::vec3 mPosition = lnm::vec3(0.0f);
		lnm::quat mOrientation = lnm::quat(1.0f, lnm::vec3());
		lnm::vec3 mScale = lnm::vec3(1.0f);
	};
} // namespace lune