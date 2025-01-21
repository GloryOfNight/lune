#include "lune/vulkan/buffer.hxx"

#include "lune/core/log.hxx"

lune::vulkan::Buffer::~Buffer()
{
	const auto cleanBufferLam = [buffer = mBuffer, allocation = mVmaAllocation]() -> bool
	{
		vmaDestroyBuffer(getVulkanContext().vmaAllocator, buffer, allocation);
		return true;
	};
	getVulkanDeleteQueue().push(cleanBufferLam);
}

lune::vulkan::UniqueBuffer lune::vulkan::Buffer::create(vk::BufferUsageFlags usage, vk::DeviceSize size, VmaMemoryUsage vmaUsage, VmaAllocationCreateFlags vmaFlags)
{
	auto newBuffer = std::make_unique<Buffer>();
	newBuffer->init(usage, size, vmaUsage, vmaFlags);
	return std::move(newBuffer);
}

void lune::vulkan::Buffer::init(vk::BufferUsageFlags usage, vk::DeviceSize size, VmaMemoryUsage vmaUsage, VmaAllocationCreateFlags vmaFlags)
{
	mSize = size;
	vk::BufferCreateInfo bufferCreateInfo = vk::BufferCreateInfo()
												.setSize(mSize)
												.setUsage(usage)
												.setQueueFamilyIndices(getVulkanContext().queueFamilyIndices)
												.setSharingMode(vk::SharingMode::eExclusive);

	VmaAllocationCreateInfo vmaCreateInfo{};
	vmaCreateInfo.usage = vmaUsage;
	vmaCreateInfo.flags = vmaFlags;

	VmaAllocationInfo info{};
	vmaCreateBuffer(getVulkanContext().vmaAllocator, reinterpret_cast<const VkBufferCreateInfo*>(&bufferCreateInfo), &vmaCreateInfo, reinterpret_cast<VkBuffer*>(&mBuffer), &mVmaAllocation, &info);
}

uint8* lune::vulkan::Buffer::map() const
{
	uint8* pBuffer{};
	VkResult mapRes = vmaMapMemory(getVulkanContext().vmaAllocator, mVmaAllocation, reinterpret_cast<void**>(&pBuffer));
	if (mapRes != VK_SUCCESS) [[unlikely]]
	{
		LN_LOG(Fatal, Vulkan::Buffer, "Failed to vmaMapMemory. Did you tried to map device memory?");
		return nullptr;
	}
	return pBuffer;
}

void lune::vulkan::Buffer::unmap() const
{
	vmaUnmapMemory(getVulkanContext().vmaAllocator, mVmaAllocation);
}

void lune::vulkan::Buffer::copyMap(const void* data, size_t offset, size_t size)
{
	uint8* pBuffer = map();
	if (pBuffer)
	{
		memcpy(pBuffer + offset, data, size);
		unmap();
	}
}

void lune::vulkan::Buffer::copyTransfer(const void* data, size_t offset, size_t size)
{
	const auto vmaUsage = VMA_MEMORY_USAGE_AUTO_PREFER_HOST;
	const auto vmaFlags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;
	auto stagingBuffer = create(vk::BufferUsageFlagBits::eTransferSrc, size, vmaUsage, vmaFlags);

	stagingBuffer->copyMap(data, 0, size);

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

	const vk::BufferCopy copyRegion = vk::BufferCopy()
										  .setSrcOffset(0)
										  .setDstOffset(offset)
										  .setSize(size);

	commandBuffer.copyBuffer(stagingBuffer->getBuffer(), getBuffer(), copyRegion);

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
