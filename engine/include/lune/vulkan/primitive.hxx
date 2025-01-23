#pragma once

#include "lune/core/math.hxx"
#include "lune/lune.hxx"
#include "lune/vulkan/vulkan_core.hxx"

#include "buffer.hxx"

#include <span>

namespace lune::vulkan
{
	class Primitive final
	{
	public:
		Primitive() = default;
		Primitive(const Primitive&) = delete;
		Primitive(Primitive&&) = default;
		~Primitive() = default;

		template <typename Vert, typename Indx = Index>
		static SharedPrimitive create(std::span<Vert> verticies, std::span<Indx> indices = {})
		{
			return create(verticies.data(), verticies.size(), sizeof(typename decltype(verticies)::element_type), indices.data(), indices.size(), sizeof(typename decltype(indices)::element_type));
		}

		static SharedPrimitive create(const void* vertData, uint32 vertDataSize, uint32 vertSizeof, const void* indexData, uint32 indexDataSize, uint32 indexSizeof);

		void cmdBind(vk::CommandBuffer commandBuffer);

		void cmdDraw(vk::CommandBuffer commandBuffer, uint32 instanceCount = 1, uint32 firstInstance = 0);

	private:
		void init(const void* vertData, uint32 vertDataSize, uint32 vertSizeof, const void* indexData, uint32 indexDataSize, uint32 indexSizeof);

		uint32 mVerticiesSize{};
		uint32 mVerticiesCount{};

		uint32 mIndicesSize{};
		uint32 mIndicesCount{};
		vk::IndexType mIndicesType{};

		UniqueBuffer mBuffer{};
	};
} // namespace lune::vulkan