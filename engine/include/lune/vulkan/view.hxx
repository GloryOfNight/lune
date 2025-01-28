#pragma once

#include "lune/vulkan/depth_image.hxx"
#include "lune/vulkan/msaa_image.hxx"

#include "vulkan_core.hxx"

#include <memory>
#include <vector>
#include <vulkan/vulkan_handles.hpp>

struct SDL_Window;
struct ImGui_ImplVulkanH_Window;
struct ImGuiContext;

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
		SDL_Window* getWindow() const { return mWindow; }
		ImGui_ImplVulkanH_Window* getImGuiWindow() { return mImGuiWindow; }

		vk::CommandBuffer getCurrentImageCmdBuffer() const { return mImageCommandBuffer; }
		vk::CommandBuffer getCurrentImageCopyCmdBuffer() const { return mCopyCommandBuffer; }

	private:
		void init();

		void recreateSwapchain();

		bool updateExtent();

		void createSwapchain();

		void cleanupSwapchain(vk::SwapchainKHR swapchain);

		void createImageViews();

		void createFramebuffers();

		void createFences();

		void createSemaphores();

		void createImGui();
		void shutdownImGui();

		bool acquireNextImageIndex();

		SDL_Window* mWindow{nullptr};

		ImGuiContext* mImGuiContext{};

		ImGui_ImplVulkanH_Window* mImGuiWindow{};

		uint32 mImageIndex{};

		vk::CommandBuffer mImageCommandBuffer{};
		vk::CommandBuffer mCopyCommandBuffer{};

		UniqueDepthImage mDepthImage;

		UniqueMsaaImage mMsaaImage;

		vk::SurfaceKHR mSurface;

		vk::SwapchainKHR mSwapchain;

		vk::Extent2D mCurrentExtent;

		std::vector<vk::ImageView> mSwapchainImageViews;
		uint32 mMinImageCount{};

		std::vector<vk::Framebuffer> mFramebuffers;

		std::vector<vk::Fence> mSubmitQueueFences;

		vk::Semaphore mSemaphoreImageAvailable, mSemaphoreCopyComplete, mSemaphoreRenderFinished;
	};
} // namespace lune::vulkan
