#pragma once

#include "vulkan/vulkan.hpp"

#include "lune.hxx"
#include "vk_mem_alloc.h"

namespace lune
{
	struct vulkan_context final
	{
		vk::Instance instance{};
		vk::PhysicalDevice physicalDevice{};
		vk::Device device{};
		vk::Queue graphicsQueue{};
		vk::Queue transferQueue{};

		uint32 graphicsQueueIndex{};
		uint32 transferQueueIndex{};
		std::vector<uint32> queueFamilyIndices{};

		vk::CommandPool graphicsCommandPool{};
		vk::CommandPool transferCommandPool{};

		vk::RenderPass renderPass{};

		VmaAllocator vmaAllocator{};
	};

	struct vulkan_config final
	{
		vk::Format colorFormat{};
		vk::Format depthFormat{};
		vk::SampleCountFlagBits sampleCount{};
	};

	extern "C++" vulkan_context& getVulkanContext();
	extern "C++" vulkan_config& getVulkanConfig();
} // namespace lune