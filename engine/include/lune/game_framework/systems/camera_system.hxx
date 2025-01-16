#pragma once

#include "lune/core/math.hxx"
#include "lune/vulkan/buffer.hxx"

#include "system.hxx"

#include <map>

namespace lune
{
	class CameraSystem
		: public SystemBase,
		  public PrepareRenderSystemInterface
	{
	public:
		virtual void update(class Scene* scene, double deltaTime) override;

		virtual void prepareRender(class Scene* scene) override;

		const vulkan::UniqueBuffer& getViewProjectionBuffer() const { return mCameraBuffer; }

		const lnm::mat4& getView(uint32 viewId) const { return mViewsProjs.at(viewId).view; }
		const lnm::mat4& getProjection(uint32 viewId) const { return mViewsProjs.at(viewId).proj; }
		const lnm::mat4& getViewProjection(uint32 viewId) const { return mViewsProjs.at(viewId).viewProj; }

	private:
		vulkan::UniqueBuffer mStagingCameraBuffer{};
		vulkan::UniqueBuffer mCameraBuffer{};

		struct ViewProj
		{
			lnm::mat4 view{};
			lnm::mat4 proj{};
			lnm::mat4 viewProj{};
		};
		std::map<uint32, ViewProj> mViewsProjs{};
	};
} // namespace lune