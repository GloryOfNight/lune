#pragma once

#include "lune/core/math.hxx"

#include "component.hxx"

namespace lune
{
	struct TransformComponent : public ComponentBase
	{
		void translate(const lnm::vec3& translation) { mPosition += translation; }
		void move(const lnm::vec3& translation) { mPosition += mOrientation * translation; }
		void rotate(const lnm::quat& rotation)
		{
			mOrientation = rotation * mOrientation;
			mOrientation = lnm::normalize(mOrientation);
		}
		void rotate(float rads, const lnm::vec3& axis) { rotate(lnm::angleAxis(rads, glm::normalize(axis))); };
		void scale(const lnm::vec3& scaling) { mScale *= scaling; }

		void setTransform(const lnm::vec3& position, const lnm::quat& rotation, const lnm::vec3& scale)
		{
			mPosition = position;
			mOrientation = rotation;
			mScale = scale;
		}

		lnm::mat4 getTransform() const
		{
			return lnm::translate(lnm::mat4(1.0f), mPosition) * lnm::mat4(mOrientation) * lnm::scale(lnm::mat4(1.0f), mScale);
		}

		lnm::vec3 mPosition = lnm::vec3(0.0f);
		lnm::quat mOrientation = lnm::quat(1.0f, lnm::vec3());
		lnm::vec3 mScale = lnm::vec3(1.0f);
	};
} // namespace lune