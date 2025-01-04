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
	auto subsystem = vulkan_subsystem::get();

	auto instance = vulkan_subsystem::get()->getInstance();
	auto physicalDevice = vulkan_subsystem::get()->getPhysicalDevice();
	auto device = vulkan_subsystem::get()->getDevice();

	VkSurfaceKHR surface{};
	SDL_Vulkan_CreateSurface(window, instance, nullptr, &surface);

	if (surface == VK_NULL_HANDLE)
	{
		return nullptr;
	}

	auto newView = std::make_unique<view>();
	newView->mInstance = instance;
	newView->mPhysicalDevice = physicalDevice;
	newView->mDevice = device;
	newView->mWindow = window;
	newView->mSurface = surface;

	return std::move(newView);
}

void lune::vulkan::view::init()
{
	createSwapchain();

	createImageViews();

	createImageCommandBuffers(vulkan_subsystem::get()->getTransferCommandPool());

	//_depthImage.create(viewIndex);
	//_msaaImage.create(viewIndex);

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

	//_depthImage.recreate();
	//_msaaImage.recreate();

	createFramebuffers();
}

void lune::vulkan::view::destroy()
{
	[[maybe_unused]] const auto waitResult = mDevice.waitForFences(mSubmitQueueFences, true, UINT32_MAX);
	for (auto& fence : mSubmitQueueFences)
	{
		mDevice.destroyFence(fence);
	}

	cleanupSwapchain(mSwapchain);

	mDevice.destroySemaphore(mSemaphoreImageAvailable);
	mDevice.destroySemaphore(mSemaphoreRenderFinished);

	//_depthImage.destroy();
	//_msaaImage.destroy();

	mInstance.destroy(mSurface);

	new (this) view(); // reset the object
}

bool lune::vulkan::view::updateExtent(vk::PhysicalDevice physicalDevice)
{
	const vk::Extent2D newExtent = physicalDevice.getSurfaceCapabilitiesKHR(mSurface).currentExtent;
	if (mCurrentExtent != newExtent)
	{
		mCurrentExtent = newExtent;
		return true;
	}
	return false;
}

// void lune::vulkan::view::setViewMatrix(const de::math::mat4& viewMatrix)
// {
// 	_viewMatrix = viewMatrix;
// }

// void lune::vulkan::view::applySettings(settings&& newSettings)
// {
// 	if (_settings != newSettings)
// 	{
// 		_settings = newSettings;

// 		recreateSwapchain();

// 		auto& mats = renderer::get()->getMaterials();
// 		for (auto& [name, mat] : mats)
// 		{
// 			mat->viewUpdated(_viewIndex);
// 		}
// 	}
// }

uint32_t lune::vulkan::view::acquireNextImageIndex()
{
	vk::ResultValue<uint32_t> aquireNextImageResult = vk::ResultValue<uint32_t>(vk::Result{}, UINT32_MAX);
	try
	{
		aquireNextImageResult = mDevice.acquireNextImageKHR(mSwapchain, UINT32_MAX, mSemaphoreImageAvailable, nullptr);
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
	const vk::Result waitFencesResult = mDevice.waitForFences(waitFences, true, UINT32_MAX);
	if (waitFencesResult == vk::Result::eTimeout)
		return nullptr;

	mDevice.resetFences(waitFences);

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
	auto graphicsQueue = vulkan_subsystem::get()->getGraphicsQueue();

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

	graphicsQueue.submit(submitInfo, mSubmitQueueFences[imageIndex]);

	const vk::PresentInfoKHR presentInfo =
		vk::PresentInfoKHR()
			.setWaitSemaphores(submitSignalSemaphores)
			.setSwapchains({1, &mSwapchain})
			.setImageIndices({1, &imageIndex});

	try
	{
		const vk::Result presentResult = graphicsQueue.presentKHR(presentInfo);

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

vk::SharingMode lune::vulkan::view::getSharingMode() const
{
	return vk::SharingMode::eExclusive;
}

uint32_t lune::vulkan::view::getImageCount() const
{
	return mSwapchainImageViews.size();
}

void lune::vulkan::view::createSwapchain()
{
	const vk::SurfaceCapabilitiesKHR surfaceCapabilities = mPhysicalDevice.getSurfaceCapabilitiesKHR(mSurface);
	mCurrentExtent = surfaceCapabilities.currentExtent;

	const auto surfaceFormat = findSurfaceFormat(mPhysicalDevice, mSurface, getFormat());
	if (surfaceFormat == vk::SurfaceFormatKHR())
	{
		LN_LOG(Fatal, Vulkan::View, "Failed to find preferred surface format!");
		return;
	}

	const auto presentMode = findPresentMode(mPhysicalDevice, mSurface);
	if (presentMode == vk::PresentModeKHR())
		LN_LOG(Error, Vulkan::View, "Failed to to find preffered present mode!");

	const auto sharingMode{getSharingMode()};
	const auto queueFamilyIndexes{vulkan_subsystem::get()->getQueueFamilyIndices()};

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
			.setImageSharingMode(static_cast<vk::SharingMode>(sharingMode))
			.setQueueFamilyIndices(queueFamilyIndexes)
			.setPreTransform(surfaceCapabilities.currentTransform)
			.setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque)
			.setPresentMode(presentMode)
			.setClipped(VK_TRUE)
			.setOldSwapchain(mSwapchain);

	mSwapchain = mDevice.createSwapchainKHR(swapchainCreateInfo);
	if (swapchainCreateInfo.oldSwapchain)
	{
		cleanupSwapchain(swapchainCreateInfo.oldSwapchain);
	}
}

void lune::vulkan::view::cleanupSwapchain(vk::SwapchainKHR swapchain)
{
	mDevice.waitIdle();

	mDevice.destroyRenderPass(mRenderPass);

	for (auto frameBuffer : mFramebuffers)
	{
		mDevice.destroyFramebuffer(frameBuffer);
	}
	mFramebuffers.clear();

	for (auto imageView : mSwapchainImageViews)
	{
		mDevice.destroyImageView(imageView);
	}
	mSwapchainImageViews.clear();

	mDevice.destroySwapchainKHR(swapchain);
}

void lune::vulkan::view::createImageViews()
{
	const auto swapchainImages = mDevice.getSwapchainImagesKHR(mSwapchain);
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
				.setFormat(getFormat())
				.setComponents(imageViewComponents)
				.setSubresourceRange(imageSubresourceRange);

		mSwapchainImageViews[i] = mDevice.createImageView(imageViewCreateInfo);
	}
}

void lune::vulkan::view::createRenderPass()
{
	const vk::SampleCountFlagBits sampleCount = vk::SampleCountFlagBits::e1;
	const bool isMultisamplingSupported = false;

	std::vector<vk::AttachmentDescription> attachmentsDescriptions;

	std::vector<vk::AttachmentReference> attachmentReferences;
	std::vector<vk::AttachmentReference> resolveAttachmentReferences;

	attachmentsDescriptions.emplace_back() // color
		.setFormat(getFormat())
		.setSamples(sampleCount)
		.setLoadOp(vk::AttachmentLoadOp::eClear)
		.setStoreOp(isMultisamplingSupported ? vk::AttachmentStoreOp::eDontCare : vk::AttachmentStoreOp::eStore)
		.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
		.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
		.setInitialLayout(vk::ImageLayout::eUndefined)
		.setFinalLayout(isMultisamplingSupported ? vk::ImageLayout::eColorAttachmentOptimal : vk::ImageLayout::ePresentSrcKHR);
	attachmentReferences.push_back(vk::AttachmentReference(0, vk::ImageLayout::eColorAttachmentOptimal));

	// attachmentsDescriptions.emplace_back() // depth
	// 	.setFormat(_depthImage.getFormat())
	// 	.setSamples(sampleCount)
	// 	.setLoadOp(vk::AttachmentLoadOp::eClear)
	// 	.setStoreOp(vk::AttachmentStoreOp::eDontCare)
	// 	.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
	// 	.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
	// 	.setInitialLayout(vk::ImageLayout::eUndefined)
	// 	.setFinalLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);
	// attachmentReferences.push_back(vk::AttachmentReference(1, vk::ImageLayout::eDepthStencilAttachmentOptimal));

	// if (isMultisamplingSupported)
	// {
	// 	attachmentsDescriptions.emplace_back() // color msaa
	// 		.setFormat(getFormat())
	// 		.setSamples(vk::SampleCountFlagBits::e1)
	// 		.setLoadOp(vk::AttachmentLoadOp::eDontCare)
	// 		.setStoreOp(vk::AttachmentStoreOp::eDontCare)
	// 		.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
	// 		.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
	// 		.setInitialLayout(vk::ImageLayout::eUndefined)
	// 		.setFinalLayout(vk::ImageLayout::ePresentSrcKHR);
	// 	resolveAttachmentReferences.push_back(vk::AttachmentReference(2, vk::ImageLayout::eColorAttachmentOptimal));
	// }

	const vk::SubpassDescription subpassDescription =
		vk::SubpassDescription()
			.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics)
			.setColorAttachments({1, &attachmentReferences[0]})
			//.setPDepthStencilAttachment(&attachmentReferences[1])
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

	mRenderPass = mDevice.createRenderPass(renderPassCreateInfo);
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
		//attachments.push_back(_depthImage.getImageView());

		const vk::FramebufferCreateInfo framebufferCreateInfo =
			vk::FramebufferCreateInfo()
				.setRenderPass(mRenderPass)
				.setAttachments(attachments)
				.setWidth(mCurrentExtent.width)
				.setHeight(mCurrentExtent.height)
				.setLayers(1);

		mFramebuffers[i] = mDevice.createFramebuffer(framebufferCreateInfo);
	}
}

void lune::vulkan::view::createImageCommandBuffers(vk::CommandPool transferCommandPool)
{
	const vk::CommandBufferAllocateInfo commandBufferAllocateInfo =
		vk::CommandBufferAllocateInfo()
			.setLevel(vk::CommandBufferLevel::ePrimary)
			.setCommandBufferCount(getImageCount())
			.setCommandPool(transferCommandPool);
	mImageCommandBuffers = mDevice.allocateCommandBuffers(commandBufferAllocateInfo);
}

void lune::vulkan::view::createFences()
{
	mSubmitQueueFences.resize(getImageCount());
	for (auto& fence : mSubmitQueueFences)
	{
		const vk::FenceCreateInfo fenceCreateInfo(vk::FenceCreateFlagBits::eSignaled);
		fence = mDevice.createFence(fenceCreateInfo);
	}
}

void lune::vulkan::view::createSemaphores()
{
	mSemaphoreImageAvailable = mDevice.createSemaphore(vk::SemaphoreCreateInfo());
	mSemaphoreRenderFinished = mDevice.createSemaphore(vk::SemaphoreCreateInfo());
}
