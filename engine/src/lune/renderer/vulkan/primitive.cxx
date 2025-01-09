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

    
}