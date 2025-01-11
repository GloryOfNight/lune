#pragma once

#include "lune/core/math.hxx"
#include "lune/vulkan/buffer.hxx"

#include "render_system.hxx"

namespace lune
{
	class CameraSystem : public RenderSystem
	{
	public:
		virtual void update(const std::vector<std::shared_ptr<Entity>>& entities, double deltaTime) override;

		virtual void beforeRender(vk::CommandBuffer commandBuffer, class Scene* scene) override;
		virtual void render(vk::CommandBuffer commandBuffer, class Scene* scene) override;

		const vulkan::UniqueBuffer& getViewProjectionBuffer() const { return cameraBuffer; }

		const lnm::mat4& getView() const { return mView; }
		const lnm::mat4& getProjection() const { return mProjection; }
		const lnm::mat4& getViewProjection() const { return mViewProjection; }

	private:
		vulkan::UniqueBuffer stagingCameraBuffer{};
		vulkan::UniqueBuffer cameraBuffer{};

		lnm::mat4 mView{};
		lnm::mat4 mProjection{};
		lnm::mat4 mViewProjection{};
	};
} // namespace lune