#pragma once

#include "lune/core/math.hxx"
#include "lune/lune.hxx"

#include "component.hxx"

namespace lune
{
	struct PerspectiveCameraComponent : public ComponentBase
	{
		void getPerspective(float& fov, float& aspectRatio, float& near, float& far) const
		{
			fov = mFov;
			aspectRatio = mAspectRatio;
			near = mNear;
			far = mFar;
		}

		lnm::vec3 mPosition = lnm::vec3(0.f, 0.f, 1.f);

		float mFov{45.f};
		float mAspectRatio{1.f};
		float mNear{0.1f};
		float mFar{10000.f};
	};

	struct IsometricCameraComponent : public ComponentBase
	{
		void getOrtho(float& left, float& right, float& bottom, float& top, float& near, float& far) const
		{
			left = mLeft;
			right = mRight;
			bottom = mBottom;
			top = mTop;
			near = mNear;
			far = mFar;
		}

		lnm::vec3 mPosition = lnm::vec3(0.f, 0.f, 1.f);
		lnm::vec3 mDirection = lnm::vec3(0.f, 0.f, 1.f);
		lnm::vec3 mUp = lnm::vec3(0.f, 1.f, 0.f);

		float mLeft = -1.f;
		float mRight = 1.f;
		float mBottom = -1.f;
		float mTop = 1.0f;
		float mNear = 0.1f;
		float mFar = 100.0f;
	};
} // namespace lune