#pragma once

#include "lune/vulkan/vulkan_core.hxx"

#include <memory>

namespace lune::vulkan
{
	using UniqueBuffer = std::unique_ptr<class Buffer>;

	class Buffer
	{
	public:
		Buffer() = default;
		Buffer(const Buffer&) = delete;
		Buffer(Buffer&&) = default;
		~Buffer() = default;

		static UniqueBuffer create(vk::BufferUsageFlags usage, vk::DeviceSize size, VmaMemoryUsage vmaUsage, VmaAllocationCreateFlags vmaFlags);

		void destroy();

		vk::Buffer getBuffer() const { return mBuffer; }

		void write(const void* data, size_t offset, size_t size);

	private:
		void init(vk::BufferUsageFlags usage, vk::DeviceSize size, VmaMemoryUsage vmaUsage, VmaAllocationCreateFlags vmaFlags);

		vk::Buffer mBuffer{};
		vk::DeviceSize mSize{};

		VmaAllocation mVmaAllocation{};
	};
} // namespace lune::vulkan