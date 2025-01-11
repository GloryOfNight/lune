#pragma once

#include "lune/vulkan/depth_image.hxx"
#include "lune/vulkan/msaa_image.hxx"
#include "vulkan/vulkan.hpp"

#include <memory>
#include <vector>

struct SDL_Window;

namespace lune::vulkan
{
	using UniqueView = std::unique_ptr<class View>;

	class View final
	{
	public:
		View() = default;
		View(SDL_Window* window, vk::SurfaceKHR surface);

		View(View&) = delete;
		View(View&&) = default;
		~View();

		static UniqueView create(SDL_Window* window);

		bool beginNextFrame();
		void beginRenderPass();
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
		void init();

		void recreateSwapchain();

		bool updateExtent();

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

		UniqueDepthImage mDepthImage;

		UniqueMsaaImage mMsaaImage;

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
