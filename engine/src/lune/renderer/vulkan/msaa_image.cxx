#include "lune/vulkan/msaa_image.hxx"

#include "lune/vulkan/view.hxx"
#include "lune/vulkan/vulkan_subsystem.hxx"

std::unique_ptr<lune::vulkan::MsaaImage> lune::vulkan::MsaaImage::create()
{
	return std::make_unique<MsaaImage>();
}

void lune::vulkan::MsaaImage::init(View* view)
{
	mFormat = getVulkanConfig().colorFormat;
	mExtent = view->getCurrentExtent();
	mSampleCount = getVulkanConfig().sampleCount;
}

void lune::vulkan::MsaaImage::destroy()
{
	getVulkanContext().device.destroyImageView(mImageView);
	vmaDestroyImage(getVulkanContext().vmaAllocator, mImage, mVmaAllocation);
	new (this) MsaaImage(); // reset the object
}

void lune::vulkan::MsaaImage::createImage()
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
			.setUsage(vk::ImageUsageFlagBits::eTransientAttachment | vk::ImageUsageFlagBits::eColorAttachment)
			.setInitialLayout(vk::ImageLayout::eUndefined)
			.setQueueFamilyIndices(getVulkanContext().queueFamilyIndices)
			.setSharingMode(vk::SharingMode::eExclusive);

	mImage = getVulkanContext().device.createImage(imageCreateInfo);
}

void lune::vulkan::MsaaImage::allocateMemory()
{
	const vk::MemoryRequirements memoryRequirements = getVulkanContext().device.getImageMemoryRequirements(mImage);

	const VmaAllocationCreateInfo allocationCreateInfo = {};
	VmaAllocationInfo allocationInfo = {};
	vmaAllocateMemory(getVulkanContext().vmaAllocator, reinterpret_cast<const VkMemoryRequirements*>(&memoryRequirements), &allocationCreateInfo, &mVmaAllocation, &allocationInfo);

	vmaBindImageMemory(getVulkanContext().vmaAllocator, mVmaAllocation, mImage);
}

void lune::vulkan::MsaaImage::createImageView()
{
	const vk::ImageSubresourceRange subresourceRange =
		vk::ImageSubresourceRange()
			.setAspectMask(vk::ImageAspectFlagBits::eColor)
			.setBaseMipLevel(0)
			.setLevelCount(1)
			.setBaseArrayLayer(0)
			.setLayerCount(1);

	const vk::ImageViewCreateInfo imageViewCreateInfo =
		vk::ImageViewCreateInfo()
			.setImage(mImage)
			.setViewType(vk::ImageViewType::e2D)
			.setFormat(mFormat)
			.setComponents(vk::ComponentMapping())
			.setSubresourceRange(subresourceRange);

	mImageView = getVulkanContext().device.createImageView(imageViewCreateInfo);
}
