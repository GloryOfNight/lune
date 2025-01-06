#pragma once

#include "vulkan_core.hxx"

#include <memory>

namespace lune::vulkan
{
	class buffer
	{
	public:
		buffer() = default;
		buffer(const buffer&) = delete;
		buffer(buffer&&) = delete;
		~buffer() = default;

		static std::unique_ptr<buffer> create();

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