#pragma once

#include "lune/vulkan/vulkan_core.hxx"

#include <memory>

namespace lune::vulkan
{
	using UniqueBuffer = std::unique_ptr<class Buffer>;

	class Buffer final
	{
	public:
		Buffer() = default;
		Buffer(const Buffer&) = delete;
		Buffer(Buffer&&) = default;
		~Buffer();

		static UniqueBuffer create(vk::BufferUsageFlags usage, vk::DeviceSize size, VmaMemoryUsage vmaUsage, VmaAllocationCreateFlags vmaFlags);

		vk::Buffer getBuffer() const { return mBuffer; }
		vk::DeviceSize getSize() const { return mSize; }

		uint8* map() const;

		void unmap() const;

		// copies data to allocation with VkMapMemory (if possible)
		void copyMap(const void* data, size_t offset, size_t size);

		// copies data to allocation using stagin buffer and transfer queue
		void copyTransfer(const void* data, size_t offset, size_t size);

	private:
		void init(vk::BufferUsageFlags usage, vk::DeviceSize size, VmaMemoryUsage vmaUsage, VmaAllocationCreateFlags vmaFlags);

		vk::Buffer mBuffer{};
		vk::DeviceSize mSize{};

		VmaAllocation mVmaAllocation{};
	};
} // namespace lune::vulkan