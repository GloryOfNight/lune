#pragma once

#include "lune/core/math.hxx"
#include "lune/lune.hxx"

#include "component.hxx"

namespace lune
{
	struct IsometricCameraComponent : public ComponentBase
	{
		lnm::mat4 getProjection() const
		{
			return lnm::ortho(mLeft, mRight, mBottom, mTop, mNear, mFar);
		}

		void setProjection(float left, float right, float bottom, float top, float near, float far)
		{
			mLeft = left;
			mRight = right;
			mBottom = bottom;
			mTop = top;
			mNear = near;
			mFar = far;
		}

		lnm::vec3 mPosition = lnm::vec3(0.0f);
		lnm::vec3 mDirection = lnm::vec3(0.0f, 0.0f, -1.0f);
		lnm::vec3 mUp = lnm::vec3(0.0f, 1.0f, 0.0f);

		float mLeft = -1.0f;
		float mRight = 1.0f;
		float mBottom = -1.0f;
		float mTop = 1.0f;
		float mNear = 0.1f;
		float mFar = 100.0f;
	};
} // namespace lune