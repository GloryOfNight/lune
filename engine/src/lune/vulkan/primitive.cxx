#include "lune/vulkan/primitive.hxx"

#include "lune/core/log.hxx"

lune::vulkan::SharedPrimitive lune::vulkan::Primitive::create(const void* vertData, uint32 vertDataSize, uint32 vertSizeof, const void* indexData, uint32 indexDataSize, uint32 indexSizeof)
{
	auto newPrimitive = std::make_shared<Primitive>();
	newPrimitive->init(vertData, vertDataSize, vertSizeof, indexData, indexDataSize, indexSizeof);
	return std::move(newPrimitive);
}

void lune::vulkan::Primitive::init(const void* vertData, uint32 vertDataSize, uint32 vertSize, const void* indexData, uint32 indexDataSize, uint32 indexSize)
{
	mVerticiesSize = vertDataSize * vertSize;
	mVerticiesCount = mVerticiesSize / vertSize;

	constexpr vk::BufferUsageFlags vertexBufferUsageBits = vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst;
	if (!mVertexBuffer)
		mVertexBuffer = Buffer::create(vertexBufferUsageBits, mVerticiesSize, VmaMemoryUsage::VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE, {});
	mVertexBuffer->copyTransfer(vertData, mVerticiesOffset, mVerticiesSize);

	mIndicesSize = indexDataSize * indexSize;
	if (mIndicesSize)
	{
		mIndicesCount = mIndicesSize / indexSize;
		mIndicesType = indexSize == sizeof(uint32) ? vk::IndexType::eUint32 : vk::IndexType::eUint16;

		constexpr vk::BufferUsageFlags indexBufferUsageBits = vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst;
		if (!mIndexBuffer)
			mIndexBuffer = Buffer::create(indexBufferUsageBits, mVerticiesSize, VmaMemoryUsage::VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE, {});
		mIndexBuffer->copyTransfer(indexData, mIndicesOffset, mIndicesSize);
	}
}

void lune::vulkan::Primitive::cmdBind(vk::CommandBuffer commandBuffer)
{
	const std::array<vk::DeviceSize, 1> vertOffsets{mVerticiesOffset};
	commandBuffer.bindVertexBuffers(0, mVertexBuffer->getBuffer(), vertOffsets);

	if (mIndexBuffer)
	{
		commandBuffer.bindIndexBuffer(mIndexBuffer->getBuffer(), mIndicesOffset, mIndicesType);
	}
}

void lune::vulkan::Primitive::cmdDraw(vk::CommandBuffer commandBuffer, uint32 instanceCount, uint32 firstInstance)
{
	if (mIndexBuffer)
	{
		uint32 indiciesSizeof = mIndicesType == vk::IndexType::eUint32 ? sizeof(uint32) : sizeof(uint16);
		commandBuffer.drawIndexed(mIndicesCount, instanceCount, 0, 0, firstInstance);
	}
	else
	{
		commandBuffer.draw(mVerticiesCount, instanceCount, 0, firstInstance);
	}
}