#include "lune/vulkan/buffer.hxx"

std::unique_ptr<lune::vulkan::Buffer> lune::vulkan::Buffer::create()
{
	return std::make_unique<Buffer>();
}

void lune::vulkan::Buffer::init(vk::BufferUsageFlags usage, vk::DeviceSize size)
{
	mSize = size;
	vk::BufferCreateInfo bufferCreateInfo = vk::BufferCreateInfo()
												.setSize(mSize)
												.setUsage(usage)
												.setQueueFamilyIndices(getVulkanContext().queueFamilyIndices)
												.setSharingMode(vk::SharingMode::eExclusive);

	VmaAllocationCreateInfo vmaCreateInfo{};
	vmaCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;
	vmaCreateInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;

	VmaAllocationInfo info{};
	vmaCreateBuffer(getVulkanContext().vmaAllocator, reinterpret_cast<const VkBufferCreateInfo*>(&bufferCreateInfo), &vmaCreateInfo, reinterpret_cast<VkBuffer*>(&mBuffer), &mVmaAllocation, &info);
}

void lune::vulkan::Buffer::destroy()
{
	if (mBuffer)
		vmaDestroyBuffer(getVulkanContext().vmaAllocator, mBuffer, mVmaAllocation);
	new (this) Buffer();
}

void lune::vulkan::Buffer::write(const void* data, size_t offset, size_t size)
{
	uint8* pBuffer{};
	vmaMapMemory(getVulkanContext().vmaAllocator, mVmaAllocation, reinterpret_cast<void**>(&pBuffer));

	memcpy(pBuffer + offset, data, size);

	vmaUnmapMemory(getVulkanContext().vmaAllocator, mVmaAllocation);
}
