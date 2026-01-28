#include "lune/vulkan/view.hxx"

#include "backends/imgui_impl_sdl3.h"
#include "backends/imgui_impl_vulkan.h"
#include "lune/core/log.hxx"
#include "lune/vulkan/vulkan_core.hxx"
#include "lune/vulkan/vulkan_subsystem.hxx"

#include "imgui.h"

#include <SDL3/SDL_video.h>
#include <SDL3/SDL_vulkan.h>
#include <algorithm>
#include <thread>
#include <vulkan/vulkan_enums.hpp>
#include <vulkan/vulkan_handles.hpp>

vk::SurfaceFormatKHR findSurfaceFormat(vk::PhysicalDevice physicalDevice, vk::SurfaceKHR surface, vk::Format format)
{
	const auto surfaceFormats = physicalDevice.getSurfaceFormatsKHR(surface);
	const auto pred = [format](const vk::SurfaceFormatKHR surfaceFormat)
	{
		return surfaceFormat.format == format;
	};
	const auto result = std::find_if(surfaceFormats.begin(), surfaceFormats.end(), pred);
	return result != surfaceFormats.end() ? *result : vk::SurfaceFormatKHR();
}

vk::PresentModeKHR findPresentMode(vk::PhysicalDevice physicalDevice, vk::SurfaceKHR surface)
{
	// clang-format off
	constexpr auto modesPriority =	
		std::array
		{
			vk::PresentModeKHR::eMailbox,
			vk::PresentModeKHR::eFifo
		};
	// clang-format on
	const auto availableModes = physicalDevice.getSurfacePresentModesKHR(surface);
	const auto result = std::find_first_of(modesPriority.begin(), modesPriority.end(), availableModes.begin(), availableModes.end());
	return result != modesPriority.end() ? *result : vk::PresentModeKHR();
}

lune::vulkan::View::View(SDL_Window* window, vk::SurfaceKHR surface)
	: View()
{
	mWindow = window;
	mSurface = surface;
}

lune::vulkan::View::~View()
{
	SDL_DestroyWindow(mWindow);

	shutdownImGui();

	const auto cleanSwapchainLam = [framebuffers = mFramebuffers, imageViews = mSwapchainImageViews, swapchain = mSwapchain]() -> bool
	{
		for (auto framebuffer : framebuffers)
			getVulkanContext().device.destroyFramebuffer(framebuffer);
		for (auto imageView : imageViews)
			getVulkanContext().device.destroyImageView(imageView);
		getVulkanContext().device.destroySwapchainKHR(swapchain);
		return true;
	};

	const auto cleanSurfaceLam = [surface = mSurface]() -> bool
	{
		SDL_Vulkan_DestroySurface(getVulkanContext().instance, surface, nullptr);
		return true;
	};

	const auto cleanOtherLam = [semaphore1 = mSemaphoreImageAvailable, semaphore2 = mSemaphoreCopyComplete, semaphore3 = mSemaphoreRenderFinished, fences = mSubmitQueueFences]() -> bool
	{
		getVulkanContext().device.destroySemaphore(semaphore1);
		getVulkanContext().device.destroySemaphore(semaphore2);
		getVulkanContext().device.destroySemaphore(semaphore3);
		for (auto fence : fences)
			getVulkanContext().device.destroyFence(fence);
		return true;
	};

	getVulkanDeleteQueue().push(cleanSwapchainLam);
	getVulkanDeleteQueue().push(cleanSurfaceLam);
	getVulkanDeleteQueue().push(cleanOtherLam);

	mMsaaImage.reset();
	mDepthImage.reset();
}

lune::vulkan::UniqueView lune::vulkan::View::create(SDL_Window* window)
{
	VkSurfaceKHR surface{};
	SDL_Vulkan_CreateSurface(window, getVulkanContext().instance, nullptr, &surface);

	if (surface == VK_NULL_HANDLE)
	{
		return nullptr;
	}

	auto newView = std::make_unique<View>(window, surface);
	newView->init();
	return std::move(newView);
}

void lune::vulkan::View::init()
{
	updateExtent();
	createSwapchain();
	createImageViews();

	mDepthImage = DepthImage::create(getCurrentExtent());

	if (getVulkanConfig().sampleCount != vk::SampleCountFlagBits::e1)
		mMsaaImage = MsaaImage::create(getCurrentExtent());

	createFramebuffers();
	createFences();
	createSemaphores();
	createImGui();
}

void lune::vulkan::View::recreateSwapchain()
{
	if (0 == mCurrentExtent.height || 0 == mCurrentExtent.width)
	{
		return;
	}

	LN_LOG(Info, Vulkan::View, "Recreating swapchain with new extent w: {}, h:{}", mCurrentExtent.width, mCurrentExtent.height);

	createSwapchain();
	createImageViews();

	if (mDepthImage)
		mDepthImage = DepthImage::create(getCurrentExtent());

	if (mMsaaImage)
		mMsaaImage = MsaaImage::create(getCurrentExtent());

	createFramebuffers();

	shutdownImGui();
	createImGui();
}

bool lune::vulkan::View::updateExtent()
{
	const auto surfaceCapabilities = getVulkanContext().physicalDevice.getSurfaceCapabilitiesKHR(mSurface);

	vk::Extent2D newExtent = getVulkanContext().physicalDevice.getSurfaceCapabilitiesKHR(mSurface).currentExtent;
	if (mCurrentExtent != newExtent)
	{
		int w, h;
		SDL_GetWindowSizeInPixels(mWindow, &w, &h);

		if (newExtent.width == UINT32_MAX || newExtent.height == UINT32_MAX)
		{
			int w, h;
			SDL_GetWindowSizeInPixels(mWindow, &w, &h);
			newExtent.width = w;
			newExtent.height = h;
		}

		mCurrentExtent.width = std::clamp(newExtent.width, surfaceCapabilities.minImageExtent.width, surfaceCapabilities.maxImageExtent.width);
		mCurrentExtent.height = std::clamp(newExtent.height, surfaceCapabilities.minImageExtent.height, surfaceCapabilities.maxImageExtent.height);
		return true;
	}
	return false;
}

bool lune::vulkan::View::beginNextFrame()
{
	if (!acquireNextImageIndex())
		return false;

	ImGui::SetCurrentContext(mImGuiContext);
	ImGui_ImplVulkan_NewFrame();
	ImGui_ImplSDL3_NewFrame();
	ImGui::NewFrame();

	{
		const vk::CommandBufferAllocateInfo commandBufferAllocateInfo =
			vk::CommandBufferAllocateInfo()
				.setLevel(vk::CommandBufferLevel::ePrimary)
				.setCommandBufferCount(1)
				.setCommandPool(getVulkanContext().transferCommandPool);
		mCopyCommandBuffer = getVulkanContext().device.allocateCommandBuffers(commandBufferAllocateInfo)[0];
	}
	{
		const vk::CommandBufferAllocateInfo commandBufferAllocateInfo =
			vk::CommandBufferAllocateInfo()
				.setLevel(vk::CommandBufferLevel::ePrimary)
				.setCommandBufferCount(1)
				.setCommandPool(getVulkanContext().graphicsCommandPool);
		mImageCommandBuffer = getVulkanContext().device.allocateCommandBuffers(commandBufferAllocateInfo)[0];
	}

	const vk::CommandBufferBeginInfo commandBufferBeginInfo =
		vk::CommandBufferBeginInfo()
			.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);

	mCopyCommandBuffer.begin(commandBufferBeginInfo);
	mImageCommandBuffer.begin(commandBufferBeginInfo);

	return true;
}

void lune::vulkan::View::beginRenderPass()
{
	std::array<vk::ClearValue, 2> clearValues{};
	clearValues[0].color = vk::ClearColorValue(std::array<float, 4>{0.0F, 0.0F, 0.0F, 1.0F});
	clearValues[1].depthStencil = vk::ClearDepthStencilValue(1.0F, 0U);

	const vk::RenderPassBeginInfo renderPassBeginInfo =
		vk::RenderPassBeginInfo()
			.setRenderPass(getVulkanContext().renderPass)
			.setFramebuffer(mFramebuffers[mImageIndex])
			.setRenderArea(vk::Rect2D(vk::Offset2D(0, 0), mCurrentExtent))
			.setClearValues(clearValues);

	mImageCommandBuffer.beginRenderPass(renderPassBeginInfo, vk::SubpassContents::eInline);

	mImageCommandBuffer.setViewport(0, vk::Viewport(0.f, 0.f, static_cast<float>(mCurrentExtent.width), static_cast<float>(mCurrentExtent.height), 0.f, 1.f));
	mImageCommandBuffer.setScissor(0, vk::Rect2D(vk::Offset2D(0, 0), mCurrentExtent));
}

void lune::vulkan::View::sumbit()
{
	ImGui::SetCurrentContext(mImGuiContext);
	ImGui::Render();
	ImGui::EndFrame();

	{ // submit copy command buffer
		mCopyCommandBuffer.end();
		const std::array<vk::Semaphore, 1> submitWaitSemaphores = {mSemaphoreImageAvailable};
		const std::array<vk::Semaphore, 1> submitSignalSemaphores = {mSemaphoreCopyComplete};
		const std::array<vk::PipelineStageFlags, 1> submitWaitDstStages = {vk::PipelineStageFlagBits::eTopOfPipe};
		const std::array<vk::CommandBuffer, 1> submitCommandBuffers = {mCopyCommandBuffer};
		const vk::SubmitInfo submitInfo =
			vk::SubmitInfo()
				.setWaitSemaphores(submitWaitSemaphores)
				.setSignalSemaphores(submitSignalSemaphores)
				.setWaitDstStageMask(submitWaitDstStages)
				.setCommandBuffers(submitCommandBuffers);
		getVulkanContext().transferQueue.submit(submitInfo);
		mCopyCommandBuffer = nullptr;
	}

	ImGui::SetCurrentContext(mImGuiContext);
	auto drawData = ImGui::GetDrawData();
	ImGui_ImplVulkan_RenderDrawData(drawData, mImageCommandBuffer);

	mImageCommandBuffer.endRenderPass();
	mImageCommandBuffer.end();

	const std::array<vk::Semaphore, 1> submitWaitSemaphores = {mSemaphoreCopyComplete};
	const std::array<vk::Semaphore, 1> submitSignalSemaphores = {mSemaphoreRenderFinished};
	const std::array<vk::PipelineStageFlags, 1> submitWaitDstStages = {vk::PipelineStageFlagBits::eColorAttachmentOutput};
	const std::array<vk::CommandBuffer, 1> submitCommandBuffers = {mImageCommandBuffer};

	const vk::SubmitInfo submitInfo =
		vk::SubmitInfo()
			.setWaitSemaphores(submitWaitSemaphores)
			.setSignalSemaphores(submitSignalSemaphores)
			.setWaitDstStageMask(submitWaitDstStages)
			.setCommandBuffers(submitCommandBuffers);
	getVulkanContext().graphicsQueue.submit(submitInfo, mSubmitQueueFences[mImageIndex]);
	mImageCommandBuffer = nullptr;

	const vk::PresentInfoKHR presentInfo =
		vk::PresentInfoKHR()
			.setWaitSemaphores(submitSignalSemaphores)
			.setSwapchains({1, &mSwapchain})
			.setImageIndices({1, &mImageIndex});

	try
	{
		const vk::Result presentResult = getVulkanContext().graphicsQueue.presentKHR(presentInfo);

		if (vk::Result::eSuboptimalKHR == presentResult)
		{
			recreateSwapchain();
		}
	}
	catch (vk::OutOfDateKHRError error)
	{
		LN_LOG(Warning, Vulkan::View, "OutOfDateKHRError");
	}

	const std::array<vk::Fence, 1> waitFences{mSubmitQueueFences[mImageIndex]};
	const vk::Result waitFencesResult = getVulkanContext().device.waitForFences(waitFences, true, UINT32_MAX);
	getVulkanContext().device.resetFences(waitFences);
}

bool lune::vulkan::View::acquireNextImageIndex()
{
	constexpr uint64 timeout = std::chrono::nanoseconds(std::chrono::milliseconds(1)).count();
	const VkResult aquireRes = vkAcquireNextImageKHR(getVulkanContext().device, mSwapchain, timeout, mSemaphoreImageAvailable, VK_NULL_HANDLE, &mImageIndex);

	if (aquireRes == VK_SUCCESS || aquireRes == VK_SUBOPTIMAL_KHR) [[likely]]
		return true;

	if (aquireRes == VK_ERROR_OUT_OF_DATE_KHR)
	{
		recreateSwapchain();
	}
	return false;
}

void lune::vulkan::View::createSwapchain()
{
	const auto surfaceFormat = findSurfaceFormat(getVulkanContext().physicalDevice, mSurface, getVulkanConfig().colorFormat);
	if (surfaceFormat == vk::SurfaceFormatKHR())
	{
		LN_LOG(Fatal, Vulkan::View, "Failed to find preferred surface format!");
		return;
	}

	const auto presentMode = findPresentMode(getVulkanContext().physicalDevice, mSurface);
	if (presentMode == vk::PresentModeKHR())
		LN_LOG(Error, Vulkan::View, "Failed to to find preffered present mode!");

	const vk::SurfaceCapabilitiesKHR surfaceCapabilities = getVulkanContext().physicalDevice.getSurfaceCapabilitiesKHR(mSurface);
	mMinImageCount = surfaceCapabilities.maxImageCount >= 3 ? 3 : surfaceCapabilities.minImageCount;

	const vk::SwapchainCreateInfoKHR swapchainCreateInfo =
		vk::SwapchainCreateInfoKHR()
			.setSurface(mSurface)
			.setMinImageCount(mMinImageCount)
			.setImageFormat(surfaceFormat.format)
			.setImageColorSpace(surfaceFormat.colorSpace)
			.setImageExtent(mCurrentExtent)
			.setImageArrayLayers(1)
			.setImageUsage(vk::ImageUsageFlagBits::eColorAttachment)
			.setImageSharingMode(vk::SharingMode::eExclusive)
			.setQueueFamilyIndices(getVulkanContext().queueFamilyIndices)
			.setPreTransform(surfaceCapabilities.currentTransform)
			.setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque)
			.setPresentMode(presentMode)
			.setClipped(VK_TRUE)
			.setOldSwapchain(mSwapchain);

	mSwapchain = getVulkanContext().device.createSwapchainKHR(swapchainCreateInfo);
	if (swapchainCreateInfo.oldSwapchain)
	{
		cleanupSwapchain(swapchainCreateInfo.oldSwapchain);
	}
}

void lune::vulkan::View::cleanupSwapchain(vk::SwapchainKHR swapchain)
{
	for (auto frameBuffer : mFramebuffers)
		getVulkanContext().device.destroyFramebuffer(frameBuffer);
	mFramebuffers.clear();

	for (auto imageView : mSwapchainImageViews)
		getVulkanContext().device.destroyImageView(imageView);
	mSwapchainImageViews.clear();

	getVulkanContext().device.destroySwapchainKHR(swapchain);
}

void lune::vulkan::View::createImageViews()
{
	const auto swapchainImages = getVulkanContext().device.getSwapchainImagesKHR(mSwapchain);
	const size_t imageCount = swapchainImages.size();

	mSwapchainImageViews.resize(imageCount);
	for (size_t i = 0; i < imageCount; ++i)
	{
		const vk::ComponentMapping imageViewComponents =
			vk::ComponentMapping()
				.setR(vk::ComponentSwizzle::eIdentity)
				.setG(vk::ComponentSwizzle::eIdentity)
				.setB(vk::ComponentSwizzle::eIdentity)
				.setA(vk::ComponentSwizzle::eIdentity);

		const vk::ImageSubresourceRange imageSubresourceRange =
			vk::ImageSubresourceRange()
				.setAspectMask(vk::ImageAspectFlagBits::eColor)
				.setBaseArrayLayer(0)
				.setBaseMipLevel(0)
				.setLayerCount(1)
				.setLevelCount(1);

		const vk::ImageViewCreateInfo imageViewCreateInfo =
			vk::ImageViewCreateInfo()
				.setImage(swapchainImages[i])
				.setViewType(vk::ImageViewType::e2D)
				.setFormat(getVulkanConfig().colorFormat)
				.setComponents(imageViewComponents)
				.setSubresourceRange(imageSubresourceRange);

		mSwapchainImageViews[i] = getVulkanContext().device.createImageView(imageViewCreateInfo);
	}
}

void lune::vulkan::View::createFramebuffers()
{
	const size_t imageCount = getImageCount();
	mFramebuffers.resize(imageCount);

	for (size_t i = 0; i < imageCount; ++i)
	{
		std::vector<vk::ImageView> attachments;
		attachments.reserve(3);

		if (mMsaaImage)
		{
			attachments.push_back(mMsaaImage->getImageView());
			attachments.push_back(mDepthImage->getImageView());
			attachments.push_back(mSwapchainImageViews[i]);
		}
		else
		{
			attachments.push_back(mSwapchainImageViews[i]);
			attachments.push_back(mDepthImage->getImageView());
		}

		const vk::FramebufferCreateInfo framebufferCreateInfo =
			vk::FramebufferCreateInfo()
				.setRenderPass(getVulkanContext().renderPass)
				.setAttachments(attachments)
				.setWidth(mCurrentExtent.width)
				.setHeight(mCurrentExtent.height)
				.setLayers(1);

		mFramebuffers[i] = getVulkanContext().device.createFramebuffer(framebufferCreateInfo);
	}
}

void lune::vulkan::View::createFences()
{
	mSubmitQueueFences.resize(getImageCount());
	for (auto& fence : mSubmitQueueFences)
	{
		const vk::FenceCreateInfo fenceCreateInfo{};
		fence = getVulkanContext().device.createFence(fenceCreateInfo);
	}
}

void lune::vulkan::View::createSemaphores()
{
	mSemaphoreImageAvailable = getVulkanContext().device.createSemaphore(vk::SemaphoreCreateInfo());
	mSemaphoreCopyComplete = getVulkanContext().device.createSemaphore(vk::SemaphoreCreateInfo());
	mSemaphoreRenderFinished = getVulkanContext().device.createSemaphore(vk::SemaphoreCreateInfo());
}

void lune::vulkan::View::createImGui()
{
	mImGuiContext = ImGui::CreateContext();
	ImGui::SetCurrentContext(mImGuiContext);
	ImGui::StyleColorsDark();

	ImGui_ImplSDL3_InitForVulkan(mWindow);

	auto& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;
	// io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable; maybe later

	auto& context = getVulkanContext();

	ImGui_ImplVulkan_InitInfo Info{};
	Info.Instance = context.instance;
	Info.PhysicalDevice = context.physicalDevice;
	Info.Device = context.device;
	Info.QueueFamily = context.graphicsQueueIndex;
	Info.Queue = context.graphicsQueue;
	Info.RenderPass = context.renderPass;
	Info.MinImageCount = mMinImageCount;
	Info.ImageCount = mSwapchainImageViews.size();
	Info.MSAASamples = static_cast<VkSampleCountFlagBits>(getVulkanConfig().sampleCount);
	Info.DescriptorPoolSize = 16;
	ImGui_ImplVulkan_Init(&Info);

	ImGui_ImplVulkan_CreateFontsTexture();
}

void lune::vulkan::View::shutdownImGui()
{
	ImGui::SetCurrentContext(mImGuiContext);
	if (mImGuiContext)
	{
		ImGui_ImplVulkan_Shutdown();
		ImGui_ImplSDL3_Shutdown();
		ImGui::DestroyContext(mImGuiContext);
	}
}

void lune::vulkan::View::updateViewSize()
{
	if (updateExtent())
	{
		recreateSwapchain();
	}
}
