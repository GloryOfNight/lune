#pragma once

#include "lune/math.hxx"

#include "component.hxx"

namespace lune
{
	struct transform_component : public component
	{
		lnm::vec3& getPosition() { return mPosition; }
		const lnm::vec3& getPosition() const { return mPosition; }

		lnm::quat& getRotation() { return mRotation; }
		const lnm::quat& getRotation() const { return mRotation; }

		lnm::vec3& getScale() { return mScale; }
		const lnm::vec3& getScale() const { return mScale; }

		void translate(const lnm::vec3& translation) { mPosition += translation; }
		void rotate(const lnm::quat& rotation) { mRotation = rotation * mRotation; }
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

	private:
		lnm::vec3 mPosition = lnm::vec3(0.0f);
		lnm::quat mRotation = lnm::quat(1.0f, 0.0f, 0.0f, 0.0f);
		lnm::vec3 mScale = lnm::vec3(1.0f);
	};
} // namespace lune