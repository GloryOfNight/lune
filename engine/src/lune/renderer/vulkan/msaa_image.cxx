#include "msaa_image.hxx"

#include "view.hxx"
#include "vulkan_subsystem.hxx"

std::unique_ptr<lune::vulkan::msaa_image> lune::vulkan::msaa_image::create()
{
	return std::make_unique<msaa_image>();
}

void lune::vulkan::msaa_image::init(view* view)
{
	mFormat = getVulkanConfig().mColorFormat;
	mExtent = view->getCurrentExtent();
	mSampleCount = getVulkanConfig().mSampleCount;
}

void lune::vulkan::msaa_image::destroy()
{
	getVulkanContext().device.destroyImageView(mImageView);
	vmaDestroyImage(getVulkanContext().vmaAllocator, mImage, mVmaAllocation);
	new (this) msaa_image(); // reset the object
}

void lune::vulkan::msaa_image::createImage()
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

void lune::vulkan::msaa_image::allocateMemory()
{
	const vk::MemoryRequirements memoryRequirements = getVulkanContext().device.getImageMemoryRequirements(mImage);

	const VmaAllocationCreateInfo allocationCreateInfo = {};
	VmaAllocationInfo allocationInfo = {};
	vmaAllocateMemory(getVulkanContext().vmaAllocator, reinterpret_cast<const VkMemoryRequirements*>(&memoryRequirements), &allocationCreateInfo, &mVmaAllocation, &allocationInfo);

	vmaBindImageMemory(getVulkanContext().vmaAllocator, mVmaAllocation, mImage);
}

void lune::vulkan::msaa_image::createImageView()
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
