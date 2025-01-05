#include "msaa_image.hxx"

#include "view.hxx"
#include "vulkan_subsystem.hxx"

std::unique_ptr<lune::vulkan::msaa_image> lune::vulkan::msaa_image::create()
{
	return std::make_unique<msaa_image>();
}

void lune::vulkan::msaa_image::init(view* view)
{
	mFormat = view->getFormat();
	mExtent = view->getCurrentExtent();
	mSampleCount = view->getSampleCount();
}

void lune::vulkan::msaa_image::destroy()
{
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
			.setQueueFamilyIndices(gVulkanContext.queueFamilyIndices)
			.setSharingMode(vk::SharingMode::eExclusive);

	mImage = gVulkanContext.device.createImage(imageCreateInfo);
}

void lune::vulkan::msaa_image::allocateMemory()
{
	const vk::MemoryRequirements memoryRequirements = gVulkanContext.device.getImageMemoryRequirements(mImage);

	const VmaAllocationCreateInfo allocationCreateInfo = {};
	VmaAllocationInfo allocationInfo = {};
	vmaAllocateMemory(gVulkanContext.vmaAllocator, reinterpret_cast<const VkMemoryRequirements*>(&memoryRequirements), &allocationCreateInfo, &mVmaAllocation, &allocationInfo);

	vmaBindImageMemory(gVulkanContext.vmaAllocator, mVmaAllocation, mImage);
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

    mImageView = gVulkanContext.device.createImageView(imageViewCreateInfo);
}
