#pragma once

#include "lune/vulkan/vulkan_core.hxx"

namespace lune::vulkan
{
	class VulkanCustomResource
	{
	public:
		VulkanCustomResource() = default;
		VulkanCustomResource(const VulkanCustomResource&) = delete;
		VulkanCustomResource(VulkanCustomResource&&) = default;
		virtual ~VulkanCustomResource() = default;
	};
} // namespace lune::vulkan