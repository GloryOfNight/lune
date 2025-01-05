#include "depth_image.hxx"

#include "log.hxx"
#include "view.hxx"
#include "vulkan_subsystem.hxx"

vk::Format findSupportedDepthFormat(const vk::PhysicalDevice physicalDevice) noexcept
{
	constexpr auto formatCandidates = std::array{vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint};
	const auto imageTiling = vk::ImageTiling::eOptimal;
	const auto formatFeatureFlags = vk::FormatFeatureFlagBits::eDepthStencilAttachment;

	for (const auto format : formatCandidates)
	{
		const auto formatProperties = physicalDevice.getFormatProperties(format);
		if (imageTiling == vk::ImageTiling::eLinear &&
			(formatProperties.linearTilingFeatures & formatFeatureFlags) == formatFeatureFlags)
		{
			return format;
		}
		else if (imageTiling == vk::ImageTiling::eOptimal &&
				 (formatProperties.optimalTilingFeatures & formatFeatureFlags) == formatFeatureFlags)
		{
			return format;
		}
	}
	return vk::Format::eUndefined;
}

std::unique_ptr<lune::vulkan::depth_image> lune::vulkan::depth_image::create()
{
	return std::make_unique<depth_image>();
}

void lune::vulkan::depth_image::init(class view* view)
{
	mFormat = findSupportedDepthFormat(gVulkanContext.physicalDevice);
	mExtent = view->getCurrentExtent();

	createImage();

	allocateMemory();

	createImageView();

	transitionImageLayout(vk::ImageLayout::eUndefined, vk::ImageLayout::eDepthStencilAttachmentOptimal);
}

void lune::vulkan::depth_image::destroy()
{
	gVulkanContext.device.destroyImageView(mImageView);
	vmaDestroyImage(gVulkanContext.vmaAllocator, mImage, mVmaAllocation);
	new (this) depth_image(); // reset the object
}

void lune::vulkan::depth_image::createImage()
{
	const vk::ImageCreateInfo imageCreateInfo =
		vk::ImageCreateInfo()
			.setImageType(vk::ImageType::e2D)
			.setFormat(mFormat)
			.setExtent(vk::Extent3D(mExtent))
			.setMipLevels(1)
			.setArrayLayers(1)
			.setSamples(mSampleCount)
			.setTiling(vk::ImageTiling::eOptimal)
			.setUsage(vk::ImageUsageFlagBits::eDepthStencilAttachment)
			.setInitialLayout(vk::ImageLayout::eUndefined)
			.setQueueFamilyIndices(gVulkanContext.queueFamilyIndices)
			.setSharingMode(vk::SharingMode::eExclusive);

	mImage = gVulkanContext.device.createImage(imageCreateInfo);
}

void lune::vulkan::depth_image::allocateMemory()
{
	vk::MemoryRequirements memoryRequirements = gVulkanContext.device.getImageMemoryRequirements(mImage);

	const VmaAllocationCreateInfo allocationCreateInfo = {};
	VmaAllocationInfo allocationInfo = {};
	vmaAllocateMemory(gVulkanContext.vmaAllocator, reinterpret_cast<VkMemoryRequirements*>(&memoryRequirements), &allocationCreateInfo, &mVmaAllocation, &allocationInfo);

	vmaBindImageMemory(gVulkanContext.vmaAllocator, mVmaAllocation, mImage);
}

void lune::vulkan::depth_image::createImageView()
{
	const vk::ImageViewCreateInfo imageViewCreateInfo =
		vk::ImageViewCreateInfo()
			.setImage(mImage)
			.setViewType(vk::ImageViewType::e2D)
			.setFormat(mFormat)
			.setComponents(vk::ComponentMapping())
			.setSubresourceRange(vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eDepth, 0, 1, 0, 1));

	mImageView = gVulkanContext.device.createImageView(imageViewCreateInfo);
}

void lune::vulkan::depth_image::transitionImageLayout(vk::ImageLayout oldLayout, vk::ImageLayout newLayout)
{
	const vk::CommandBufferAllocateInfo commandBufferAllocateInfo =
		vk::CommandBufferAllocateInfo()
			.setLevel(vk::CommandBufferLevel::ePrimary)
			.setCommandBufferCount(1)
			.setCommandPool(gVulkanContext.transferCommandPool);

	vk::CommandBuffer commandBuffer = gVulkanContext.device.allocateCommandBuffers(commandBufferAllocateInfo)[0];

	const vk::CommandBufferBeginInfo commandBufferBeginInfo =
		vk::CommandBufferBeginInfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
	commandBuffer.begin(commandBufferBeginInfo);

	const vk::ImageSubresourceRange imageSubresource =
		vk::ImageSubresourceRange()
			.setAspectMask(vk::ImageAspectFlagBits::eDepth)
			.setBaseMipLevel(0)
			.setBaseArrayLayer(0)
			.setLayerCount(1)
			.setLevelCount(1);

	const vk::ImageMemoryBarrier imageMemoryBarrier =
		vk::ImageMemoryBarrier()
			.setOldLayout(oldLayout)
			.setNewLayout(newLayout)
			.setImage(mImage)
			.setSubresourceRange(imageSubresource);

	commandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eTopOfPipe, vk::PipelineStageFlagBits::eEarlyFragmentTests, vk::DependencyFlagBits(), {}, {}, {imageMemoryBarrier});
	commandBuffer.end();

	const vk::SubmitInfo submitInfo =
		vk::SubmitInfo()
			.setCommandBuffers({commandBuffer});

	gVulkanContext.transferQueue.submit(submitInfo, nullptr);
}
