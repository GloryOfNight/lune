#pragma once

#include "lune/core/math.hxx"
#include "lune/lune.hxx"
#include "lune/vulkan/vulkan_core.hxx"
#include "vulkan/vulkan.hpp"

#include "buffer.hxx"

#include <memory>

namespace lune::vulkan
{
	struct Vertex
	{
		lnm::vec3 position;
		lnm::vec4 color;
		lnm::vec2 uv;
	};

	using Index = uint32;

	class Primitive final
	{
	public:
		Primitive() = default;
		Primitive(const Primitive&) = delete;
		Primitive(Primitive&&) = default;
		~Primitive() = default;

		static SharedPrimitive create(const std::vector<Vertex>& vertices, const std::vector<Index>& indices);

		void destroy() {};

		void cmdBind(vk::CommandBuffer commandBuffer);

		void cmdDraw(vk::CommandBuffer commandBuffer, uint32 instanceCount = 1, uint32 firstInstance = 0);

	private:
		void init(const std::vector<Vertex>& vertexies, const std::vector<Index>& indices);

		uint32 mVerticesSize{};
		uint32 mIndeciesSize{};

		UniqueBuffer mBuffer{};
	};
} // namespace lune::vulkan