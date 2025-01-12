#pragma once

#include "lune/core/math.hxx"

#include "component.hxx"

namespace lune
{
	struct TransformComponent : public ComponentBase
	{
		void translate(const lnm::vec3& translation) { mPosition += translation; }
		void move(const lnm::vec3& translation) { mPosition += mRotation * translation; }
		void rotate(const lnm::quat& rotation) { mRotation = rotation * mRotation; }
		void rotate(float rads, lnm::vec3& axis) { mRotation *= lnm::angleAxis(rads, axis);};
		void scale(const lnm::vec3& scaling) { mScale *= scaling; }

		void setTransform(const lnm::vec3& position, const lnm::quat& rotation, const lnm::vec3& scale)
		{
			mPosition = position;
			mRotation = rotation;
			mScale = scale;
		}

		lnm::mat4 getTransform() const
		{
			return lnm::translate(lnm::mat4(1.0f), mPosition) * lnm::mat4(mRotation) * lnm::scale(lnm::mat4(1.0f), mScale);
		}

		lnm::vec3 mPosition = lnm::vec3(0.0f);
		lnm::quat mRotation = lnm::quat(1.0f, 0.0f, 0.0f, 0.0f);
		lnm::vec3 mScale = lnm::vec3(1.0f);
	};
} // namespace lune