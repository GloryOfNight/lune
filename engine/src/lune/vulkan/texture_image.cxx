#include "lune/vulkan/texture_image.hxx"

#include "lune/core/log.hxx"
#include "lune/vulkan/vulkan_subsystem.hxx"

vk::Format sdlFormatToVulkan(SDL_PixelFormat format)
{
	switch (format)
	{
	case SDL_PIXELFORMAT_RGBA32:
		return vk::Format::eR8G8B8A8Unorm;
	case SDL_PIXELFORMAT_BGRA8888:
		return vk::Format::eB8G8R8A8Unorm;
	default:
		LN_LOG(Fatal, Vulkan::Image, "Unsupported format detected!");
		return vk::Format::eUndefined;
	}
}

lune::vulkan::TextureImage::~TextureImage()
{
	const auto cleanSampler = [sampler = mSampler]() -> bool
	{
		getVulkanContext().device.destroySampler(sampler);
		return true;
	};
	const auto cleanImageView = [imageView = mImageView, sampler = mSampler]() -> bool
	{
		getVulkanContext().device.destroyImageView(imageView);
		return true;
	};
	const auto cleanImageAlloc = [image = mImage, vmaAlloc = mVmaAllocation]() -> bool
	{
		vmaDestroyImage(getVulkanContext().vmaAllocator, image, vmaAlloc);
		return true;
	};
	getVulkanDeleteQueue().push(cleanSampler);
	getVulkanDeleteQueue().push(cleanImageView);
	getVulkanDeleteQueue().push(cleanImageAlloc);
}

lune::vulkan::UniqueTextureImage lune::vulkan::TextureImage::create(std::span<const SDL_Surface*, 6> cubeSurfaces)
{
	auto newTexImage = std::make_unique<TextureImage>();
	newTexImage->init(cubeSurfaces);
	return newTexImage;
}

lune::vulkan::UniqueTextureImage lune::vulkan::TextureImage::create(const SDL_Surface* surface)
{
	auto newTexImage = std::make_unique<TextureImage>();
	newTexImage->init(std::span<const SDL_Surface*>(&(surface), 1));
	return newTexImage;
}

void lune::vulkan::TextureImage::init(std::span<const SDL_Surface*> surfaces)
{
	const auto imageCreateFlags = surfaces.size() == 6 ? vk::ImageCreateFlagBits::eCubeCompatible : vk::ImageCreateFlagBits();
	const uint32 layerCount = surfaces.size();
	const auto extent = vk::Extent3D(static_cast<uint32>(surfaces[0]->w), static_cast<uint32>(surfaces[0]->h), 1);
	const auto imageViewType = surfaces.size() == 6 ? vk::ImageViewType::eCube : vk::ImageViewType::e2D;

	mFormat = sdlFormatToVulkan(surfaces[0]->format);
	createImage(imageCreateFlags, layerCount, extent);
	createImageView(imageViewType, layerCount);

	copyPixelsToImage(surfaces, layerCount, extent);
}

void lune::vulkan::TextureImage::createImage(vk::ImageCreateFlagBits flags, uint32 arrayLayers, vk::Extent3D extent)
{
	const vk::ImageCreateInfo imageCreateInfo =
		vk::ImageCreateInfo()
			.setFlags(flags)
			.setImageType(vk::ImageType::e2D)
			.setFormat(mFormat)
			.setExtent(extent)
			.setMipLevels(1)
			.setArrayLayers(arrayLayers)
			.setSamples(vk::SampleCountFlagBits::e1)
			.setTiling(vk::ImageTiling::eOptimal)
			.setUsage(vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst)
			.setInitialLayout(vk::ImageLayout::eUndefined)
			.setQueueFamilyIndices(getVulkanContext().queueFamilyIndices)
			.setSharingMode(vk::SharingMode::eExclusive);

	VmaAllocationCreateInfo allocationCreateInfo{};
	allocationCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;
	allocationCreateInfo.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;

	VmaAllocationInfo allocationInfo = {};
	vmaCreateImage(getVulkanContext().vmaAllocator, reinterpret_cast<const VkImageCreateInfo*>(&imageCreateInfo), &allocationCreateInfo, reinterpret_cast<VkImage*>(&mImage), &mVmaAllocation, &allocationInfo);
}

void lune::vulkan::TextureImage::createImageView(vk::ImageViewType type, uint32 layerCount)
{
	const vk::ImageSubresourceRange subresourceRange =
		vk::ImageSubresourceRange()
			.setAspectMask(vk::ImageAspectFlagBits::eColor)
			.setBaseMipLevel(0)
			.setLevelCount(1)
			.setBaseArrayLayer(0)
			.setLayerCount(layerCount);

	const vk::ImageViewCreateInfo imageViewCreateInfo =
		vk::ImageViewCreateInfo()
			.setImage(mImage)
			.setViewType(type)
			.setFormat(mFormat)
			.setSubresourceRange(subresourceRange);

	mImageView = getVulkanContext().device.createImageView(imageViewCreateInfo);
}

void lune::vulkan::TextureImage::copyPixelsToImage(std::span<const SDL_Surface*> surfaces, uint32 layerCount, vk::Extent3D extent)
{
	VkBuffer stagingBuffer;
	VmaAllocation stagingAllocation;
	{ // Create staging buffer
		vk::BufferCreateInfo bufferCreateInfo =
			vk::BufferCreateInfo()
				.setSize(vma::getAllocationSize(mVmaAllocation))
				.setUsage(vk::BufferUsageFlagBits::eTransferSrc)
				.setSharingMode(vk::SharingMode::eExclusive);

		VmaAllocationCreateInfo allocationCreateInfo{};
		allocationCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;
		allocationCreateInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;

		VmaAllocationInfo allocationInfo = {};

		vmaCreateBuffer(getVulkanContext().vmaAllocator, reinterpret_cast<const VkBufferCreateInfo*>(&bufferCreateInfo), &allocationCreateInfo, &stagingBuffer, &stagingAllocation, &allocationInfo);
	}

	{ // Copy pixels to staging buffer
		void* data{};
		vmaMapMemory(getVulkanContext().vmaAllocator, stagingAllocation, &data);
		const uint32 size = surfaces[0]->w * surfaces[0]->pitch;
		uint32 offset = 0;
		for (const SDL_Surface* surface : surfaces)
		{
			const uint8* pixels = static_cast<const uint8*>(surface->pixels);
			std::memcpy(static_cast<uint8*>(data) + offset, pixels, size);
			offset += size;
		}
		vmaUnmapMemory(getVulkanContext().vmaAllocator, stagingAllocation);
	}

	const vk::CommandBufferAllocateInfo commandBufferAllocateInfo =
		vk::CommandBufferAllocateInfo()
			.setCommandPool(getVulkanContext().transferCommandPool)
			.setLevel(vk::CommandBufferLevel::ePrimary)
			.setCommandBufferCount(1);

	const vk::CommandBuffer commandBuffer = getVulkanContext().device.allocateCommandBuffers(commandBufferAllocateInfo)[0];

	const vk::CommandBufferBeginInfo commandBufferBeginInfo =
		vk::CommandBufferBeginInfo()
			.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);

	commandBuffer.begin(commandBufferBeginInfo);

	{ // Copy staging buffer to image

		const vk::ImageSubresourceRange subresourceRange =
			vk::ImageSubresourceRange()
				.setAspectMask(vk::ImageAspectFlagBits::eColor)
				.setBaseMipLevel(0)
				.setLevelCount(1)
				.setBaseArrayLayer(0)
				.setLayerCount(layerCount);

		const vk::ImageMemoryBarrier imageMemoryBarrier =
			vk::ImageMemoryBarrier()
				.setSrcAccessMask({})
				.setDstAccessMask(vk::AccessFlagBits::eTransferWrite)
				.setOldLayout(vk::ImageLayout::eUndefined)
				.setNewLayout(vk::ImageLayout::eTransferDstOptimal)
				.setSrcQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
				.setDstQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
				.setImage(mImage)
				.setSubresourceRange(subresourceRange);

		commandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eTopOfPipe, vk::PipelineStageFlagBits::eTransfer, {}, {}, {}, imageMemoryBarrier);

		const vk::ImageSubresourceLayers imageSubresourceLayers =
			vk::ImageSubresourceLayers()
				.setAspectMask(vk::ImageAspectFlagBits::eColor)
				.setMipLevel(0)
				.setBaseArrayLayer(0)
				.setLayerCount(layerCount);

		const vk::BufferImageCopy bufferImageCopy =
			vk::BufferImageCopy()
				.setBufferOffset(0)
				.setBufferRowLength(0)
				.setBufferImageHeight(0)
				.setImageSubresource(imageSubresourceLayers)
				.setImageOffset(vk::Offset3D(0, 0, 0))
				.setImageExtent(extent);

		commandBuffer.copyBufferToImage(stagingBuffer, mImage, vk::ImageLayout::eTransferDstOptimal, bufferImageCopy);
	}

	{ // Transition image layout
		const vk::ImageSubresourceRange subresourceRange =
			vk::ImageSubresourceRange()
				.setAspectMask(vk::ImageAspectFlagBits::eColor)
				.setBaseMipLevel(0)
				.setLevelCount(1)
				.setBaseArrayLayer(0)
				.setLayerCount(layerCount);

		const vk::ImageMemoryBarrier imageMemoryBarrier =
			vk::ImageMemoryBarrier()
				.setSrcAccessMask(vk::AccessFlagBits::eTransferWrite)
				.setDstAccessMask(vk::AccessFlagBits::eShaderRead)
				.setOldLayout(vk::ImageLayout::eTransferDstOptimal)
				.setNewLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
				.setSrcQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
				.setDstQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
				.setImage(mImage)
				.setSubresourceRange(subresourceRange);

		commandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eFragmentShader, {}, {}, {}, imageMemoryBarrier);

		commandBuffer.end();

		const vk::Fence transferFence = getVulkanContext().device.createFence(vk::FenceCreateInfo());
		const vk::SubmitInfo submitInfo =
			vk::SubmitInfo()
				.setCommandBufferCount(1)
				.setPCommandBuffers(&commandBuffer);

		getVulkanContext().transferQueue.submit(submitInfo, transferFence);
		[[maybe_unused]] vk::Result waitRes = getVulkanContext().device.waitForFences(transferFence, true, UINT64_MAX);
		getVulkanContext().device.destroyFence(transferFence);
	}

	vmaDestroyBuffer(getVulkanContext().vmaAllocator, stagingBuffer, stagingAllocation);
}
