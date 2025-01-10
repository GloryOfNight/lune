#include "lune/vulkan/primitive.hxx"

#include "lune/core/log.hxx"

lune::vulkan::SharedPrimitive lune::vulkan::Primitive::create(const std::vector<Vertex>& vertexies, const std::vector<Index>& indices)
{
	if (vertexies.size() == 0) [[unlikely]]
	{
		LN_LOG(Error, Vulkan::Primitive, "Attempt to create primitive with 0 vertexies");
		return nullptr;
	}

	return SharedPrimitive();
}

void lune::vulkan::Primitive::init(const std::vector<Vertex>& vertexies, const std::vector<Index>& indices)
{
	mVertexiesSize = vertexies.size() * sizeof(Vertex);
	mIndeciesSize = indices.size() * sizeof(Index);

	mBuffer = Buffer::create(vk::BufferUsageFlagBits::eVertexBuffer, mVertexiesSize + mIndeciesSize, VmaMemoryUsage::VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE, {});

	mBuffer->copyTransfer(vertexies.data(), 0, mVertexiesSize);
	mBuffer->copyTransfer(indices.data(), mVertexiesSize, mIndeciesSize);
}

void lune::vulkan::Primitive::cmdBind(vk::CommandBuffer commandBuffer)
{
	const std::array<vk::DeviceSize, 1> vertOffsets{0};
	commandBuffer.bindVertexBuffers(0, mBuffer->getBuffer(), vertOffsets);

	if (mIndeciesSize)
	{
		commandBuffer.bindIndexBuffer(mBuffer->getBuffer(), mVertexiesSize, vk::IndexType::eUint32);
		static_assert(sizeof(Index) == sizeof(uint32), "Fix index buffer bind ^^^^^^^^^^^^^^^^^^^^");
	}
}

void lune::vulkan::Primitive::cmdDraw(vk::CommandBuffer commandBuffer, uint32 instanceCount, uint32 firstInstance)
{
	if (mIndeciesSize == 0)
	{
		commandBuffer.draw(mVertexiesSize / sizeof(Vertex), instanceCount, 0, firstInstance);
	}
	else
	{
		commandBuffer.drawIndexed(mIndeciesSize / sizeof(Index), instanceCount, 0, mVertexiesSize, firstInstance);
	}
}