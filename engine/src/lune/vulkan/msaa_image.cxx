#include "lune/vulkan/msaa_image.hxx"

#include "lune/vulkan/view.hxx"
#include "lune/vulkan/vulkan_subsystem.hxx"

lune::vulkan::UniqueMsaaImage lune::vulkan::MsaaImage::create(View* view)
{
	auto newMsaaImage = std::make_unique<MsaaImage>();
	newMsaaImage->init(view);
	return std::move(newMsaaImage);
}

lune::vulkan::MsaaImage::~MsaaImage()
{
	const auto cleanImageView = [imageView = mImageView]() -> bool
	{
		getVulkanContext().device.destroyImageView(imageView);
		return true;
	};
	const auto cleanImageAlloc = [image = mImage, vmaAlloc = mVmaAllocation]() -> bool
	{
		vmaDestroyImage(getVulkanContext().vmaAllocator, image, vmaAlloc);
		return true;
	};
	getVulkanDeleteQueue().push(cleanImageView);
	getVulkanDeleteQueue().push(cleanImageAlloc);
}

void lune::vulkan::MsaaImage::init(View* view)
{
	mFormat = getVulkanConfig().colorFormat;
	mExtent = view->getCurrentExtent();
	mSampleCount = getVulkanConfig().sampleCount;

	createImage();
	createImageView();
}

void lune::vulkan::MsaaImage::createImage()
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
			.setUsage(vk::ImageUsageFlagBits::eTransientAttachment | vk::ImageUsageFlagBits::eColorAttachment)
			.setInitialLayout(vk::ImageLayout::eUndefined)
			.setQueueFamilyIndices(getVulkanContext().queueFamilyIndices)
			.setSharingMode(vk::SharingMode::eExclusive);

	VmaAllocationCreateInfo vmaAllocCreateInfo{};
	vmaAllocCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;
	VmaAllocationInfo allocInfo{};
	vmaCreateImage(getVulkanContext().vmaAllocator, reinterpret_cast<const VkImageCreateInfo*>(&imageCreateInfo), &vmaAllocCreateInfo, reinterpret_cast<VkImage*>(&mImage), &mVmaAllocation, &allocInfo);
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
