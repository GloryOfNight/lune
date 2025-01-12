#include "lune/vulkan/primitive.hxx"

#include "lune/core/log.hxx"

lune::vulkan::SharedPrimitive lune::vulkan::Primitive::create(const std::vector<Vertex>& vertices, const std::vector<Index>& indices)
{
	if (vertices.size() == 0) [[unlikely]]
	{
		LN_LOG(Error, Vulkan::Primitive, "Attempt to create primitive with 0 vertexies");
		return nullptr;
	}

	auto newPrimitive = std::make_shared<Primitive>();
	newPrimitive->init(vertices, indices);

	return std::move(newPrimitive);
}

void lune::vulkan::Primitive::init(const std::vector<Vertex>& vertices, const std::vector<Index>& indices)
{
	mVerticesSize = vertices.size() * sizeof(Vertex);
	mIndeciesSize = indices.size() * sizeof(Index);

	vk::BufferUsageFlags bufferUsageBits = vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst;
	if (indices.size())
		bufferUsageBits |= vk::BufferUsageFlagBits::eIndexBuffer;

	mBuffer = Buffer::create(bufferUsageBits, mVerticesSize + mIndeciesSize, VmaMemoryUsage::VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE, {});

	mBuffer->copyTransfer(vertices.data(), 0, mVerticesSize);
	if (mIndeciesSize)
	{
		mBuffer->copyTransfer(indices.data(), mVerticesSize, mIndeciesSize);
	}
}

void lune::vulkan::Primitive::cmdBind(vk::CommandBuffer commandBuffer)
{
	const std::array<vk::DeviceSize, 1> vertOffsets{0};
	commandBuffer.bindVertexBuffers(0, mBuffer->getBuffer(), vertOffsets);

	if (mIndeciesSize)
	{
		commandBuffer.bindIndexBuffer(mBuffer->getBuffer(), mVerticesSize, vk::IndexType::eUint32);
		static_assert(sizeof(Index) == sizeof(uint32), "Fix index buffer bind ^^^^^^^^^^^^^^^^^^^^");
	}
}

void lune::vulkan::Primitive::cmdDraw(vk::CommandBuffer commandBuffer, uint32 instanceCount, uint32 firstInstance)
{
	if (mIndeciesSize == 0)
	{
		commandBuffer.draw(mVerticesSize / sizeof(Vertex), instanceCount, 0, firstInstance);
	}
	else
	{
		commandBuffer.drawIndexed(mIndeciesSize / sizeof(Index), instanceCount, 0, 0, firstInstance);
	}
}