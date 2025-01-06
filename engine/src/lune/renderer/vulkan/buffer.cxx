#include "buffer.hxx"

std::unique_ptr<lune::vulkan::buffer> lune::vulkan::buffer::create()
{
	return std::make_unique<buffer>();
}

void lune::vulkan::buffer::init(vk::BufferUsageFlags usage, vk::DeviceSize size)
{
	mSize = size;
	vk::BufferCreateInfo bufferCreateInfo = vk::BufferCreateInfo()
												.setSize(mSize)
												.setUsage(usage)
												.setQueueFamilyIndices(getVulkanContext().queueFamilyIndices)
												.setSharingMode(vk::SharingMode::eExclusive);

	VmaAllocationCreateInfo vmaCreateInfo{};
	vmaCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;

	VkBuffer buffer = mBuffer; // compatability with C

	VmaAllocationInfo info{};
	vmaCreateBuffer(getVulkanContext().vmaAllocator, reinterpret_cast<const VkBufferCreateInfo*>(&bufferCreateInfo), &vmaCreateInfo, &buffer, &mVmaAllocation, &info);
}

void lune::vulkan::buffer::destroy()
{
	if (mBuffer)
		vmaDestroyBuffer(getVulkanContext().vmaAllocator, mBuffer, mVmaAllocation);
	new (this) buffer();
}

void lune::vulkan::buffer::write(const void* data, size_t offset, size_t size)
{
	void* pBuffer{};
	vmaMapMemory(getVulkanContext().vmaAllocator, mVmaAllocation, &pBuffer);

	memcpy(pBuffer + offset, data, size);

	vmaUnmapMemory(getVulkanContext().vmaAllocator, mVmaAllocation);
}
