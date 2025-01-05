#pragma once

#include "depth_image.hxx"

#include <memory>
#include <vector>
#include <vulkan/vulkan.hpp>

struct SDL_Window;

namespace lune::vulkan
{
	class view final
	{
	public:
		view() = default;
		view(view&) = delete;
		view(view&&) = default;
		~view() = default;

		static std::unique_ptr<view> create(SDL_Window* window);

		void init();
		void recreateSwapchain();
		void destroy();

		bool updateExtent(vk::PhysicalDevice physicalDevice);

		//void setViewMatrix(const de::math::mat4& viewMatrix);
		//const de::math::mat4& getViewMatrix() const { return _viewMatrix; };

		//const settings& getSettings() const { return _settings; }
		//void applySettings(settings&& newSettings);

		uint32_t acquireNextImageIndex();

		vk::CommandBuffer beginCommandBuffer(uint32_t imageIndex);
		void endCommandBuffer(vk::CommandBuffer commandBuffer);
		void submitCommandBuffer(uint32_t imageIndex, vk::CommandBuffer commandBuffer);

		vk::RenderPass getRenderPass() const { return mRenderPass; };
		vk::Extent2D getCurrentExtent() const { return mCurrentExtent; };
		vk::SurfaceKHR getSurface() const { return mSurface; }

		uint32_t getImageCount() const;

		inline vk::Format getFormat() const { return vk::Format::eB8G8R8A8Srgb; }

	private:
		void createSwapchain();

		void cleanupSwapchain(vk::SwapchainKHR swapchain);

		void createImageViews();

		void createRenderPass();

		void createFramebuffers();

		void createImageCommandBuffers();

		void createFences();

		void createSemaphores();

		SDL_Window* mWindow{nullptr};

		std::unique_ptr<depth_image> mDepthImage;

		vk::SurfaceKHR mSurface;

		vk::SwapchainKHR mSwapchain;

		vk::Extent2D mCurrentExtent;

		std::vector<vk::ImageView> mSwapchainImageViews;
		std::vector<vk::CommandBuffer> mImageCommandBuffers;

		vk::RenderPass mRenderPass;

		std::vector<vk::Framebuffer> mFramebuffers;

		std::vector<vk::Fence> mSubmitQueueFences;

		vk::Semaphore mSemaphoreImageAvailable, mSemaphoreRenderFinished;
	};
} // namespace lune::vulkan
