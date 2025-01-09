#pragma once

#include "lune/vulkan/vulkan_core.hxx"

#include <memory>

namespace lune::vulkan
{
	class Buffer
	{
	public:
		Buffer() = default;
		Buffer(const Buffer&) = delete;
		Buffer(Buffer&&) = delete;
		~Buffer() = default;

		static std::unique_ptr<Buffer> create();

		void init(vk::BufferUsageFlags usage, vk::DeviceSize size);
		void destroy();

		vk::Buffer getBuffer() const { return mBuffer; }

		void write(const void* data, size_t offset, size_t size);

	private:
		vk::Buffer mBuffer{};
		vk::DeviceSize mSize{};

		VmaAllocation mVmaAllocation{};
	};
} // namespace lune::vulkan