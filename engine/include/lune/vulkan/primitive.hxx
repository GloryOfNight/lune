#pragma once

#include "lune/core/math.hxx"
#include "lune/lune.hxx"
#include "lune/vulkan/vulkan_core.hxx"

#include "buffer.hxx"

#include <span>
#include <vector>
#include <vulkan/vulkan_core.h>

namespace lune::vulkan
{
	class Primitive final
	{
	public:
		Primitive() = default;
		Primitive(const Primitive&) = delete;
		Primitive(Primitive&&) = default;
		~Primitive() = default;

		template <typename Vert, typename Indx = uint32>
		static SharedPrimitive create(std::span<Vert> verticies, std::span<Indx> indices = {})
		{
			return create(verticies.data(), verticies.size(), sizeof(typename decltype(verticies)::element_type), indices.data(), indices.size(), sizeof(typename decltype(indices)::element_type));
		}

		template <typename Vert, typename Indx = uint32>
		static SharedPrimitive createFromBuffers(std::span<const Vert> vertData, const std::span<const Indx> indxData, SharedBuffer vertBuffer, uint32 vertOffset, SharedBuffer indxBuffer, uint32 indxOffset)
		{
			auto newPrimitive = std::make_shared<Primitive>();
			newPrimitive->mVerticiesOffset = vertOffset;
			newPrimitive->mVertexBuffer = vertBuffer;
			newPrimitive->mIndicesOffset = indxOffset;
			newPrimitive->mIndexBuffer = indxBuffer;
			newPrimitive->init(vertData.data(), vertData.size(), sizeof(typename decltype(vertData)::element_type), indxData.data(), indxData.size(), sizeof(typename decltype(indxData)::element_type));
			return std::move(newPrimitive);
		}

		static SharedPrimitive create(const void* vertData, uint32 vertDataSize, uint32 vertSizeof, const void* indexData, uint32 indexDataSize, uint32 indexSizeof);

		void cmdBind(vk::CommandBuffer commandBuffer);

		void cmdDraw(vk::CommandBuffer commandBuffer, uint32 instanceCount = 1, uint32 firstInstance = 0);

		SharedBuffer getVertexBuffer() const { return mVertexBuffer; }
		std::vector<VkDeviceSize> getVertexOffsets() { return std::vector<VkDeviceSize>{0}; }

		SharedBuffer getIndexBuffer() const { return mIndexBuffer; }
		vk::IndexType getIndexType() const { return mIndicesType; }

	private:
		void init(const void* vertData, uint32 vertDataSize, uint32 vertSizeof, const void* indexData, uint32 indexDataSize, uint32 indexSizeof);

		uint32 mVerticiesSize{};
		uint32 mVerticiesCount{};
		uint32 mVerticiesOffset{};
		uint32 mVerticiesSizeof{};
		SharedBuffer mVertexBuffer{};

		uint32 mIndicesSize{};
		uint32 mIndicesCount{};
		uint32 mIndicesOffset{};
		uint32 mIndicesSizeof{};
		vk::IndexType mIndicesType{};
		SharedBuffer mIndexBuffer{};
	};
} // namespace lune::vulkan