#pragma once

#include "lune/vulkan/vulkan_core.hxx"

#include "component.hxx"

#include <string>
#include <vector>

namespace lune
{
	struct Primitive
	{
		std::string primitiveName{};
		std::string materialName{};
		vk::PrimitiveTopology topology{};
	};
	struct MeshComponent : public ComponentBase
	{
		std::vector<Primitive> primitives{};
	};
} // namespace lune