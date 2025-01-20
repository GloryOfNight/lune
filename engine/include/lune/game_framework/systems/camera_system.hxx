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

		const vulkan::UniqueBuffer& getViewProjectionBuffer() const { return mViewProjBuffer; }
		const vulkan::UniqueBuffer& getViewBuffer() const { return mViewBuffer; }
		const vulkan::UniqueBuffer& getProjectionBuffer() const { return mProjBuffer; }

		const lnm::mat4& getView(uint32 viewId) const { return mViewsProjs.at(viewId).view; }
		const lnm::mat4& getProjection(uint32 viewId) const { return mViewsProjs.at(viewId).proj; }
		const lnm::mat4& getViewProjection(uint32 viewId) const { return mViewsProjs.at(viewId).viewProj; }

	private:
		vulkan::UniqueBuffer mViewProjStagingBuffer{};

		vulkan::UniqueBuffer mViewProjBuffer{};
		vulkan::UniqueBuffer mViewBuffer{};
		vulkan::UniqueBuffer mProjBuffer{};

		struct ViewProj
		{
			lnm::mat4 viewProj{};
			lnm::mat4 view{};
			lnm::mat4 proj{};
		};
		std::map<uint32, ViewProj> mViewsProjs{};
	};
} // namespace lune