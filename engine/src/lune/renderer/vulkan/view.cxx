#include "view.hxx"

#include "log.hxx"
#include "vulkan_subsystem.hxx"

#include <SDL3/SDL_video.h>
#include <SDL3/SDL_vulkan.h>
#include <thread>

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
			vk::PresentModeKHR::eFifoRelaxed,
			vk::PresentModeKHR::eFifo
		};
	// clang-format on
	const auto availableModes = physicalDevice.getSurfacePresentModesKHR(surface);
	const auto result = std::find_first_of(modesPriority.begin(), modesPriority.end(), availableModes.begin(), availableModes.end());
	return result != modesPriority.end() ? *result : vk::PresentModeKHR();
}

std::unique_ptr<lune::vulkan::view> lune::vulkan::view::create(SDL_Window* window)
{
	VkSurfaceKHR surface{};
	SDL_Vulkan_CreateSurface(window, getVulkanContext().instance, nullptr, &surface);

	if (surface == VK_NULL_HANDLE)
	{
		return nullptr;
	}

	auto newView = std::make_unique<view>();
	newView->mWindow = window;
	newView->mSurface = surface;

	return std::move(newView);
}

void lune::vulkan::view::init()
{
	createSwapchain();

	createImageViews();

	createImageCommandBuffers();

	mDepthImage = depth_image::create();
	mDepthImage->init(this);

	if (getVulkanConfig().mSampleCount != vk::SampleCountFlagBits::e1)
	{
		mMsaaImage = msaa_image::create();
		mMsaaImage->init(this);
	}

	createRenderPass();
	createFramebuffers();
	createFences();
	createSemaphores();
}

void lune::vulkan::view::recreateSwapchain()
{
	if (0 == mCurrentExtent.height || 0 == mCurrentExtent.width)
	{
		return;
	}

	createSwapchain();
	createImageViews();

	createRenderPass();

	if (mDepthImage)
	{
		mDepthImage->destroy();
		mDepthImage->init(this);
	}

	if (mMsaaImage)
	{
		mMsaaImage->destroy();
		mMsaaImage->init(this);
	}

	createFramebuffers();
}

void lune::vulkan::view::destroy()
{
	[[maybe_unused]] const auto waitResult = getVulkanContext().device.waitForFences(mSubmitQueueFences, true, UINT32_MAX);
	for (auto& fence : mSubmitQueueFences)
	{
		getVulkanContext().device.destroyFence(fence);
	}

	cleanupSwapchain(mSwapchain);

	getVulkanContext().device.destroySemaphore(mSemaphoreImageAvailable);
	getVulkanContext().device.destroySemaphore(mSemaphoreRenderFinished);

	if (mDepthImage)
		mDepthImage->destroy();

	if (mMsaaImage)
		mMsaaImage->destroy();

	getVulkanContext().instance.destroy(mSurface);

	new (this) view(); // reset the object
}

bool lune::vulkan::view::updateExtent()
{
	const vk::Extent2D newExtent = getVulkanContext().physicalDevice.getSurfaceCapabilitiesKHR(mSurface).currentExtent;
	if (mCurrentExtent != newExtent)
	{
		mCurrentExtent = newExtent;
		return true;
	}
	return false;
}

uint32_t lune::vulkan::view::acquireNextImageIndex()
{
	vk::ResultValue<uint32_t> aquireNextImageResult = vk::ResultValue<uint32_t>(vk::Result{}, UINT32_MAX);
	try
	{
		aquireNextImageResult = getVulkanContext().device.acquireNextImageKHR(mSwapchain, UINT32_MAX, mSemaphoreImageAvailable, nullptr);
	}
	catch (vk::OutOfDateKHRError outOfDateKHRError)
	{
		return UINT32_MAX;
	}

	const uint32_t imageIndex = aquireNextImageResult.value;

	if (vk::Result::eSuccess != aquireNextImageResult.result && vk::Result::eSuboptimalKHR != aquireNextImageResult.result)
	{
		if (vk::Result::eErrorOutOfDateKHR == aquireNextImageResult.result)
		{
			recreateSwapchain();
		}
		return UINT32_MAX;
	}
	return aquireNextImageResult.value;
}

vk::CommandBuffer lune::vulkan::view::beginCommandBuffer(uint32_t imageIndex)
{
	const std::array<vk::Fence, 1> waitFences{mSubmitQueueFences[imageIndex]};
	const vk::Result waitFencesResult = getVulkanContext().device.waitForFences(waitFences, true, UINT32_MAX);
	if (waitFencesResult == vk::Result::eTimeout)
		return nullptr;

	getVulkanContext().device.resetFences(waitFences);

	vk::CommandBuffer commandBuffer = mImageCommandBuffers[imageIndex];

	const vk::CommandBufferBeginInfo commandBufferBeginInfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
	commandBuffer.begin(commandBufferBeginInfo);

	std::array<vk::ClearValue, 2> clearValues;
	clearValues[0].color = vk::ClearColorValue(std::array<float, 4>{0.0F, 0.0F, 0.0F, 1.0F});
	clearValues[1].depthStencil = vk::ClearDepthStencilValue(1.0F, 0U);

	const vk::RenderPassBeginInfo renderPassBeginInfo =
		vk::RenderPassBeginInfo()
			.setRenderPass(mRenderPass)
			.setFramebuffer(mFramebuffers[imageIndex])
			.setRenderArea(vk::Rect2D(vk::Offset2D(0, 0), mCurrentExtent))
			.setClearValues(clearValues);

	commandBuffer.beginRenderPass(renderPassBeginInfo, vk::SubpassContents::eInline);

	return commandBuffer;
}

void lune::vulkan::view::endCommandBuffer(vk::CommandBuffer commandBuffer)
{
	commandBuffer.endRenderPass();
	commandBuffer.end();
}

void lune::vulkan::view::submitCommandBuffer(uint32_t imageIndex, vk::CommandBuffer commandBuffer)
{
	const std::array<vk::Semaphore, 1> submitWaitSemaphores = {mSemaphoreImageAvailable};
	const std::array<vk::Semaphore, 1> submitSignalSemaphores = {mSemaphoreRenderFinished};
	const std::array<vk::PipelineStageFlags, 1> submitWaitDstStages = {vk::PipelineStageFlagBits::eColorAttachmentOutput};
	const std::array<vk::CommandBuffer, 1> submitCommandBuffers = {commandBuffer};

	const vk::SubmitInfo submitInfo =
		vk::SubmitInfo()
			.setWaitSemaphores(submitWaitSemaphores)
			.setSignalSemaphores(submitSignalSemaphores)
			.setWaitDstStageMask(submitWaitDstStages)
			.setCommandBuffers(submitCommandBuffers);

	getVulkanContext().graphicsQueue.submit(submitInfo, mSubmitQueueFences[imageIndex]);

	const vk::PresentInfoKHR presentInfo =
		vk::PresentInfoKHR()
			.setWaitSemaphores(submitSignalSemaphores)
			.setSwapchains({1, &mSwapchain})
			.setImageIndices({1, &imageIndex});

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
		std::this_thread::sleep_for(std::chrono::milliseconds(16));
		LN_LOG(Warning, Vulkan::View, "OutOfDateKHRError");
	}
}

uint32_t lune::vulkan::view::getImageCount() const
{
	return mSwapchainImageViews.size();
}

void lune::vulkan::view::createSwapchain()
{
	const vk::SurfaceCapabilitiesKHR surfaceCapabilities = getVulkanContext().physicalDevice.getSurfaceCapabilitiesKHR(mSurface);
	mCurrentExtent = surfaceCapabilities.currentExtent;

	const auto surfaceFormat = findSurfaceFormat(getVulkanContext().physicalDevice, mSurface, getVulkanConfig().mColorFormat);
	if (surfaceFormat == vk::SurfaceFormatKHR())
	{
		LN_LOG(Fatal, Vulkan::View, "Failed to find preferred surface format!");
		return;
	}

	const auto presentMode = findPresentMode(getVulkanContext().physicalDevice, mSurface);
	if (presentMode == vk::PresentModeKHR())
		LN_LOG(Error, Vulkan::View, "Failed to to find preffered present mode!");

	const uint32_t minImageCount = surfaceCapabilities.maxImageCount >= 3 ? 3 : surfaceCapabilities.minImageCount;

	const vk::SwapchainCreateInfoKHR swapchainCreateInfo =
		vk::SwapchainCreateInfoKHR()
			.setSurface(mSurface)
			.setMinImageCount(minImageCount)
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

void lune::vulkan::view::cleanupSwapchain(vk::SwapchainKHR swapchain)
{
	getVulkanContext().device.waitIdle();

	getVulkanContext().device.destroyRenderPass(mRenderPass);

	for (auto frameBuffer : mFramebuffers)
	{
		getVulkanContext().device.destroyFramebuffer(frameBuffer);
	}
	mFramebuffers.clear();

	for (auto imageView : mSwapchainImageViews)
	{
		getVulkanContext().device.destroyImageView(imageView);
	}
	mSwapchainImageViews.clear();

	getVulkanContext().device.destroySwapchainKHR(swapchain);
}

void lune::vulkan::view::createImageViews()
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
				.setFormat(getVulkanConfig().mColorFormat)
				.setComponents(imageViewComponents)
				.setSubresourceRange(imageSubresourceRange);

		mSwapchainImageViews[i] = getVulkanContext().device.createImageView(imageViewCreateInfo);
	}
}

void lune::vulkan::view::createRenderPass()
{
	const bool msaaEnabled = getVulkanConfig().mSampleCount != vk::SampleCountFlagBits::e1;

	std::vector<vk::AttachmentDescription> attachmentsDescriptions;

	std::vector<vk::AttachmentReference> attachmentReferences;
	std::vector<vk::AttachmentReference> resolveAttachmentReferences;

	attachmentsDescriptions.emplace_back() // color
		.setFormat(getVulkanConfig().mColorFormat)
		.setSamples(getVulkanConfig().mSampleCount)
		.setLoadOp(vk::AttachmentLoadOp::eClear)
		.setStoreOp(msaaEnabled ? vk::AttachmentStoreOp::eDontCare : vk::AttachmentStoreOp::eStore)
		.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
		.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
		.setInitialLayout(vk::ImageLayout::eUndefined)
		.setFinalLayout(msaaEnabled ? vk::ImageLayout::eColorAttachmentOptimal : vk::ImageLayout::ePresentSrcKHR);
	attachmentReferences.push_back(vk::AttachmentReference(0, vk::ImageLayout::eColorAttachmentOptimal));

	attachmentsDescriptions.emplace_back() // depth
		.setFormat(mDepthImage->getFormat())
		.setSamples(getVulkanConfig().mSampleCount)
		.setLoadOp(vk::AttachmentLoadOp::eClear)
		.setStoreOp(vk::AttachmentStoreOp::eDontCare)
		.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
		.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
		.setInitialLayout(vk::ImageLayout::eUndefined)
		.setFinalLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);
	attachmentReferences.push_back(vk::AttachmentReference(1, vk::ImageLayout::eDepthStencilAttachmentOptimal));

	if (msaaEnabled)
	{
		attachmentsDescriptions.emplace_back() // color msaa
			.setFormat(getVulkanConfig().mColorFormat)
			.setSamples(vk::SampleCountFlagBits::e1)
			.setLoadOp(vk::AttachmentLoadOp::eDontCare)
			.setStoreOp(vk::AttachmentStoreOp::eDontCare)
			.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
			.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
			.setInitialLayout(vk::ImageLayout::eUndefined)
			.setFinalLayout(vk::ImageLayout::ePresentSrcKHR);
		resolveAttachmentReferences.push_back(vk::AttachmentReference(2, vk::ImageLayout::eColorAttachmentOptimal));
	}

	const vk::SubpassDescription subpassDescription =
		vk::SubpassDescription()
			.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics)
			.setColorAttachments({1, &attachmentReferences[0]})
			.setPDepthStencilAttachment(&attachmentReferences[1])
			.setPResolveAttachments(resolveAttachmentReferences.data());

	const vk::SubpassDependency subpassDependecy =
		vk::SubpassDependency()
			.setSrcSubpass(VK_SUBPASS_EXTERNAL)
			.setDstSubpass(0)
			.setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests)
			.setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests)
			.setSrcAccessMask(vk::AccessFlagBits(0))
			.setDstAccessMask(vk::AccessFlagBits::eColorAttachmentWrite | vk::AccessFlagBits::eDepthStencilAttachmentWrite);

	const vk::RenderPassCreateInfo renderPassCreateInfo =
		vk::RenderPassCreateInfo()
			.setAttachments(attachmentsDescriptions)
			.setSubpasses({1, &subpassDescription})
			.setDependencies({1, &subpassDependecy});

	mRenderPass = getVulkanContext().device.createRenderPass(renderPassCreateInfo);
}

void lune::vulkan::view::createFramebuffers()
{
	const size_t imageCount = getImageCount();
	mFramebuffers.resize(imageCount);

	for (size_t i = 0; i < imageCount; ++i)
	{
		std::vector<vk::ImageView> attachments;
		attachments.reserve(3);

		attachments.push_back(mSwapchainImageViews[i]);
		attachments.push_back(mDepthImage->getImageView());

		const vk::FramebufferCreateInfo framebufferCreateInfo =
			vk::FramebufferCreateInfo()
				.setRenderPass(mRenderPass)
				.setAttachments(attachments)
				.setWidth(mCurrentExtent.width)
				.setHeight(mCurrentExtent.height)
				.setLayers(1);

		mFramebuffers[i] = getVulkanContext().device.createFramebuffer(framebufferCreateInfo);
	}
}

void lune::vulkan::view::createImageCommandBuffers()
{
	const vk::CommandBufferAllocateInfo commandBufferAllocateInfo =
		vk::CommandBufferAllocateInfo()
			.setLevel(vk::CommandBufferLevel::ePrimary)
			.setCommandBufferCount(getImageCount())
			.setCommandPool(getVulkanContext().transferCommandPool);
	mImageCommandBuffers = getVulkanContext().device.allocateCommandBuffers(commandBufferAllocateInfo);
}

void lune::vulkan::view::createFences()
{
	mSubmitQueueFences.resize(getImageCount());
	for (auto& fence : mSubmitQueueFences)
	{
		const vk::FenceCreateInfo fenceCreateInfo(vk::FenceCreateFlagBits::eSignaled);
		fence = getVulkanContext().device.createFence(fenceCreateInfo);
	}
}

void lune::vulkan::view::createSemaphores()
{
	mSemaphoreImageAvailable = getVulkanContext().device.createSemaphore(vk::SemaphoreCreateInfo());
	mSemaphoreRenderFinished = getVulkanContext().device.createSemaphore(vk::SemaphoreCreateInfo());
}
