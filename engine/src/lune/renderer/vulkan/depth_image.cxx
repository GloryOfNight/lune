#include "lune/vulkan/depth_image.hxx"

#include "lune/core/log.hxx"
#include "lune/vulkan/view.hxx"
#include "lune/vulkan/vulkan_subsystem.hxx"

std::unique_ptr<lune::vulkan::DepthImage> lune::vulkan::DepthImage::create()
{
	return std::make_unique<DepthImage>();
}

void lune::vulkan::DepthImage::init(View* view)
{
	mFormat = getVulkanConfig().depthFormat;
	mExtent = view->getCurrentExtent();
	mSampleCount = getVulkanConfig().sampleCount;

	createImage();

	allocateMemory();

	createImageView();

	transitionImageLayout(vk::ImageLayout::eUndefined, vk::ImageLayout::eDepthStencilAttachmentOptimal);
}

void lune::vulkan::DepthImage::destroy()
{
	getVulkanContext().device.destroyImageView(mImageView);
	vmaDestroyImage(getVulkanContext().vmaAllocator, mImage, mVmaAllocation);
	new (this) DepthImage(); // reset the object
}

void lune::vulkan::DepthImage::createImage()
{
	const vk::ImageCreateInfo imageCreateInfo =
		vk::ImageCreateInfo()
			.setImageType(vk::ImageType::e2D)
			.setFormat(mFormat)
			.setExtent(vk::Extent3D(mExtent, 1))
			.setMipLevels(1)
			.setArrayLayers(1)
			.setSamples(mSampleCount)
			.setTiling(vk::ImageTiling::eOptimal)
			.setUsage(vk::ImageUsageFlagBits::eDepthStencilAttachment)
			.setInitialLayout(vk::ImageLayout::eUndefined)
			.setQueueFamilyIndices(getVulkanContext().queueFamilyIndices)
			.setSharingMode(vk::SharingMode::eExclusive);

	mImage = getVulkanContext().device.createImage(imageCreateInfo);
}

void lune::vulkan::DepthImage::allocateMemory()
{
	const vk::MemoryRequirements memoryRequirements = getVulkanContext().device.getImageMemoryRequirements(mImage);

	const VmaAllocationCreateInfo allocationCreateInfo = {};
	VmaAllocationInfo allocationInfo = {};
	vmaAllocateMemory(getVulkanContext().vmaAllocator, reinterpret_cast<const VkMemoryRequirements*>(&memoryRequirements), &allocationCreateInfo, &mVmaAllocation, &allocationInfo);

	vmaBindImageMemory(getVulkanContext().vmaAllocator, mVmaAllocation, mImage);
}

void lune::vulkan::DepthImage::createImageView()
{
	const vk::ImageViewCreateInfo imageViewCreateInfo =
		vk::ImageViewCreateInfo()
			.setImage(mImage)
			.setViewType(vk::ImageViewType::e2D)
			.setFormat(mFormat)
			.setComponents(vk::ComponentMapping())
			.setSubresourceRange(vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eDepth, 0, 1, 0, 1));

	mImageView = getVulkanContext().device.createImageView(imageViewCreateInfo);
}

void lune::vulkan::DepthImage::transitionImageLayout(vk::ImageLayout oldLayout, vk::ImageLayout newLayout)
{
	const vk::CommandBufferAllocateInfo commandBufferAllocateInfo =
		vk::CommandBufferAllocateInfo()
			.setLevel(vk::CommandBufferLevel::ePrimary)
			.setCommandBufferCount(1)
			.setCommandPool(getVulkanContext().transferCommandPool);

	vk::CommandBuffer commandBuffer = getVulkanContext().device.allocateCommandBuffers(commandBufferAllocateInfo)[0];

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

	getVulkanContext().transferQueue.submit(submitInfo, nullptr);
}
