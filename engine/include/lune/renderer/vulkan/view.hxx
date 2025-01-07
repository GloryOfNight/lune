#pragma once

#include "depth_image.hxx"
#include "msaa_image.hxx"

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

		bool updateExtent();

		bool beginNextFrame();
		void sumbit();

		vk::Extent2D getCurrentExtent() const { return mCurrentExtent; };
		vk::SurfaceKHR getSurface() const { return mSurface; }
		uint32 getImageCount() const { return mSwapchainImageViews.size(); }
		uint32 getImageIndex() const { return mImageIndex; }

		vk::CommandBuffer getCurrentImageCmdBuffer()
		{
			if (mImageIndex != UINT32_MAX) [[likely]]

				return mImageCommandBuffers[mImageIndex];
			else
				return vk::CommandBuffer();
		}

	private:
		void createSwapchain();

		void cleanupSwapchain(vk::SwapchainKHR swapchain);

		void createImageViews();

		void createFramebuffers();

		void createImageCommandBuffers();

		void createFences();

		void createSemaphores();

		bool acquireNextImageIndex();

		SDL_Window* mWindow{nullptr};

		uint32 mImageIndex{};

		std::unique_ptr<depth_image> mDepthImage;

		std::unique_ptr<msaa_image> mMsaaImage;

		vk::SurfaceKHR mSurface;

		vk::SwapchainKHR mSwapchain;

		vk::Extent2D mCurrentExtent;

		std::vector<vk::ImageView> mSwapchainImageViews;
		std::vector<vk::CommandBuffer> mImageCommandBuffers;

		std::vector<vk::Framebuffer> mFramebuffers;

		std::vector<vk::Fence> mSubmitQueueFences;

		vk::Semaphore mSemaphoreImageAvailable, mSemaphoreRenderFinished;
	};
} // namespace lune::vulkan
