#pragma once

#include "lune/lune.hxx"
#include "vulkan/vulkan.hpp"

#include "vk_mem_alloc.h"

#include <memory>

namespace lune
{
	struct VulkanContext final
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

	struct VulkanConfig final
	{
		vk::Format colorFormat{};
		vk::Format depthFormat{};
		vk::SampleCountFlagBits sampleCount{};
	};

	namespace vulkan
	{
		using SharedGraphicsPipeline = std::shared_ptr<class GraphicsPipeline>;
		using SharedShader = std::shared_ptr<class Shader>;
		using SharedPrimitive = std::shared_ptr<class Primitive>;
	} // namespace vulkan

	extern "C++" VulkanContext& getVulkanContext();
	extern "C++" VulkanConfig& getVulkanConfig();
} // namespace lune