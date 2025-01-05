#include "texture_image.hxx"

#include "log.hxx"
#include "vulkan_subsystem.hxx"

vk::Format sdlFormatToVulkan(SDL_PixelFormat format)
{
	switch (format)
	{
	case SDL_PIXELFORMAT_ARGB8888:
		return vk::Format::eR8G8B8A8Srgb;
	case SDL_PIXELFORMAT_ABGR8888:
		return vk::Format::eB8G8R8A8Srgb;
	case SDL_PIXELFORMAT_RGBA8888:
		return vk::Format::eR8G8B8A8Srgb;
	case SDL_PIXELFORMAT_BGRA8888:
		return vk::Format::eB8G8R8A8Srgb;
	default:
		return vk::Format::eUndefined;
	}
}

void lune::vulkan::texture_image::init(const SDL_Surface& surface)
{
	mFormat = sdlFormatToVulkan(surface.format);
	if (mFormat == vk::Format::eUndefined)
	{
		LN_LOG(Fatal, Vulkan::TextureImage, "Unsupported SDL pixel format");
		return;
	}

	mExtent = vk::Extent2D(static_cast<uint32_t>(surface.w), static_cast<uint32_t>(surface.h));
	mSampleCount = vk::SampleCountFlagBits::e1;

	createImage();
	allocateMemory();
	createImageView();
	createSampler();

    copyPixelsToImage(surface);
}

void lune::vulkan::texture_image::destroy()
{
	gVulkanContext.device.destroySampler(mSampler);
	gVulkanContext.device.destroyImageView(mImageView);
	vmaDestroyImage(gVulkanContext.vmaAllocator, mImage, mVmaAllocation);
}

void lune::vulkan::texture_image::createImage()
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
			.setUsage(vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst)
			.setInitialLayout(vk::ImageLayout::eUndefined)
			.setQueueFamilyIndices(gVulkanContext.queueFamilyIndices)
			.setSharingMode(vk::SharingMode::eExclusive);

	mImage = gVulkanContext.device.createImage(imageCreateInfo);
}

void lune::vulkan::texture_image::allocateMemory()
{
	const vk::MemoryRequirements memoryRequirements = gVulkanContext.device.getImageMemoryRequirements(mImage);

	VmaAllocationCreateInfo allocationCreateInfo{};
	allocationCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;
	allocationCreateInfo.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;

	VmaAllocationInfo allocationInfo = {};
	vmaAllocateMemory(gVulkanContext.vmaAllocator, reinterpret_cast<const VkMemoryRequirements*>(&memoryRequirements), &allocationCreateInfo, &mVmaAllocation, &allocationInfo);

	vmaBindImageMemory(gVulkanContext.vmaAllocator, mVmaAllocation, mImage);
}

void lune::vulkan::texture_image::createImageView()
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
			.setSubresourceRange(subresourceRange);

	mImageView = gVulkanContext.device.createImageView(imageViewCreateInfo);
}

void lune::vulkan::texture_image::createSampler()
{
	const vk::SamplerCreateInfo samplerCreateInfo =
		vk::SamplerCreateInfo()
			.setMagFilter(vk::Filter::eLinear)
			.setMinFilter(vk::Filter::eLinear)
			.setAddressModeU(vk::SamplerAddressMode::eRepeat)
			.setAddressModeV(vk::SamplerAddressMode::eRepeat)
			.setAddressModeW(vk::SamplerAddressMode::eRepeat)
			.setAnisotropyEnable(VK_FALSE)
			.setMaxAnisotropy(1)
			.setBorderColor(vk::BorderColor::eIntOpaqueBlack)
			.setUnnormalizedCoordinates(VK_FALSE)
			.setCompareEnable(VK_FALSE)
			.setCompareOp(vk::CompareOp::eAlways)
			.setMipmapMode(vk::SamplerMipmapMode::eLinear)
			.setMipLodBias(0)
			.setMinLod(0)
			.setMaxLod(0);

	mSampler = gVulkanContext.device.createSampler(samplerCreateInfo);
}

void lune::vulkan::texture_image::copyPixelsToImage(const SDL_Surface& surface)
{
	const auto size = surface.w * surface.h * 4;

	VkBuffer stagingBuffer;
	VmaAllocation stagingAllocation;
	{ // Create staging buffer
		vk::BufferCreateInfo bufferCreateInfo =
			vk::BufferCreateInfo()
				.setSize(size)
				.setUsage(vk::BufferUsageFlagBits::eTransferSrc)
				.setSharingMode(vk::SharingMode::eExclusive);

		VmaAllocationCreateInfo allocationCreateInfo{};
		allocationCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;
		allocationCreateInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;

		VmaAllocationInfo allocationInfo = {};

		vmaCreateBuffer(gVulkanContext.vmaAllocator, reinterpret_cast<const VkBufferCreateInfo*>(&bufferCreateInfo), &allocationCreateInfo, &stagingBuffer, &stagingAllocation, &allocationInfo);
	}

	{ // Copy pixels to staging buffer
		const auto pixels = static_cast<const uint8_t*>(surface.pixels);
		void* data;
		vmaMapMemory(gVulkanContext.vmaAllocator, mVmaAllocation, &data);
		std::memcpy(data, pixels, size);
		vmaUnmapMemory(gVulkanContext.vmaAllocator, mVmaAllocation);
	}

	{ // Copy staging buffer to image
		const vk::CommandBufferAllocateInfo commandBufferAllocateInfo =
			vk::CommandBufferAllocateInfo()
				.setCommandPool(gVulkanContext.transferCommandPool)
				.setLevel(vk::CommandBufferLevel::ePrimary)
				.setCommandBufferCount(1);

		const vk::CommandBuffer commandBuffer = gVulkanContext.device.allocateCommandBuffers(commandBufferAllocateInfo)[0];

		const vk::CommandBufferBeginInfo commandBufferBeginInfo =
			vk::CommandBufferBeginInfo()
				.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);

		commandBuffer.begin(commandBufferBeginInfo);

		const vk::ImageSubresourceRange subresourceRange =
			vk::ImageSubresourceRange()
				.setAspectMask(vk::ImageAspectFlagBits::eColor)
				.setBaseMipLevel(0)
				.setLevelCount(1)
				.setBaseArrayLayer(0)
				.setLayerCount(1);

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

		const vk::BufferImageCopy bufferImageCopy =
			vk::BufferImageCopy()
				.setBufferOffset(0)
				.setBufferRowLength(0)
				.setBufferImageHeight(0)
				.setImageSubresource(vk::ImageSubresourceLayers(vk::ImageAspectFlagBits::eColor, 0, 0, 1))
				.setImageOffset(vk::Offset3D(0, 0, 0))
				.setImageExtent(vk::Extent3D(mExtent, 1));

		commandBuffer.copyBufferToImage(stagingBuffer, mImage, vk::ImageLayout::eTransferDstOptimal, bufferImageCopy);
	}

	{ // Transition image layout
		const vk::ImageSubresourceRange subresourceRange =
			vk::ImageSubresourceRange()
				.setAspectMask(vk::ImageAspectFlagBits::eColor)
				.setBaseMipLevel(0)
				.setLevelCount(1)
				.setBaseArrayLayer(0)
				.setLayerCount(1);

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

		const vk::CommandBufferAllocateInfo commandBufferAllocateInfo =
			vk::CommandBufferAllocateInfo()
				.setCommandPool(gVulkanContext.transferCommandPool)
				.setLevel(vk::CommandBufferLevel::ePrimary)
				.setCommandBufferCount(1);

		const vk::CommandBuffer commandBuffer = gVulkanContext.device.allocateCommandBuffers(commandBufferAllocateInfo)[0];

		const vk::CommandBufferBeginInfo commandBufferBeginInfo =
			vk::CommandBufferBeginInfo()
				.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);

		commandBuffer.begin(commandBufferBeginInfo);

		commandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eFragmentShader, {}, {}, {}, imageMemoryBarrier);

		commandBuffer.end();

		const vk::SubmitInfo submitInfo =
			vk::SubmitInfo()
				.setCommandBufferCount(1)
				.setPCommandBuffers(&commandBuffer);

		gVulkanContext.transferQueue.submit(submitInfo, nullptr);
		gVulkanContext.transferQueue.waitIdle();
	}

	vmaDestroyBuffer(gVulkanContext.vmaAllocator, stagingBuffer, stagingAllocation);
}
