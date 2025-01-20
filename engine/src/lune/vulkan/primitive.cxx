#include "lune/vulkan/primitive.hxx"

#include "lune/core/log.hxx"

lune::vulkan::SharedPrimitive lune::vulkan::Primitive::create(const void* vertData, uint32 vertDataSize, uint32 vertSizeof, const void* indexData, uint32 indexDataSize, uint32 indexSizeof)
{
	if (vertDataSize == 0) [[unlikely]]
	{
		LN_LOG(Error, Vulkan::Primitive, "Attempt to create primitive with 0 vertexies");
		return nullptr;
	}

	auto newPrimitive = std::make_shared<Primitive>();
	newPrimitive->init(vertData, vertDataSize, vertSizeof, indexData, indexDataSize, indexSizeof);
	return std::move(newPrimitive);
}

void lune::vulkan::Primitive::init(const void* vertData, uint32 vertDataSize, uint32 vertSize, const void* indexData, uint32 indexDataSize, uint32 indexSize)
{
	mVerticiesSize = vertDataSize * vertSize;
	mVerticiesCount = mVerticiesSize / vertSize;

	mIndicesSize = indexDataSize * indexSize;
	mIndicesCount = mIndicesSize / indexSize;
	mIndicesType = indexSize == sizeof(Index32) ? vk::IndexType::eUint32 : vk::IndexType::eUint16;

	vk::BufferUsageFlags bufferUsageBits = vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst;
	if (mIndicesSize)
		bufferUsageBits |= vk::BufferUsageFlagBits::eIndexBuffer;

	mBuffer = Buffer::create(bufferUsageBits, mVerticiesSize + mIndicesSize, VmaMemoryUsage::VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE, {});

	// double wait for copy. not good;
	mBuffer->copyTransfer(vertData, 0, mVerticiesSize);
	if (mIndicesSize)
	{
		mBuffer->copyTransfer(indexData, mVerticiesSize, mIndicesSize);
	}
}

void lune::vulkan::Primitive::cmdBind(vk::CommandBuffer commandBuffer)
{
	const std::array<vk::DeviceSize, 1> vertOffsets{0};
	commandBuffer.bindVertexBuffers(0, mBuffer->getBuffer(), vertOffsets);

	if (mIndicesSize)
	{
		commandBuffer.bindIndexBuffer(mBuffer->getBuffer(), mVerticiesSize, mIndicesType);
		static_assert(sizeof(Index32) == sizeof(uint32), "Fix index buffer bind ^^^^^^^^^^^^^^^^^^^^");
	}
}

void lune::vulkan::Primitive::cmdDraw(vk::CommandBuffer commandBuffer, uint32 instanceCount, uint32 firstInstance)
{
	if (mIndicesSize == 0)
	{
		commandBuffer.draw(mVerticiesCount, instanceCount, 0, firstInstance);
	}
	else
	{
		commandBuffer.drawIndexed(mIndicesCount, instanceCount, 0, 0, firstInstance);
	}
}