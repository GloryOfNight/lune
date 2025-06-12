#pragma once

#define VULKAN_HPP_ENABLE_DYNAMIC_LOADER_TOOL 0
#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 0
#include "lune/lune.hxx"
#include "vulkan/vulkan.hpp"

#include "vk_mem_alloc.h"

#include <functional>
#include <memory>
#include <vector>

namespace lune
{
	namespace vulkan
	{
		using SharedMaterial = std::shared_ptr<class Material>;
		using SharedGraphicsPipeline = std::shared_ptr<class GraphicsPipeline>;
		using SharedShader = std::shared_ptr<class Shader>;
		using SharedPrimitive = std::shared_ptr<class Primitive>;
		using SharedTextureImage = std::shared_ptr<class TextureImage>;
		using SharedSampler = std::shared_ptr<class Sampler>;
		using SharedBuffer = std::shared_ptr<class Buffer>;

		extern "C++" vk::detail::DispatchLoaderDynamic& getDynamicLoader() noexcept;
		extern "C++" void loadVulkanDynamicFunctions();
	} // namespace vulkan

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

	struct VulkanDeleteQueue;

	extern "C++" VulkanContext& getVulkanContext() noexcept;
	extern "C++" VulkanConfig& getVulkanConfig() noexcept;
	extern "C++" VulkanDeleteQueue& getVulkanDeleteQueue() noexcept;
} // namespace lune

namespace lune
{
	// Delete queue for vulkan resources. Expects function that return bool(true) when resources it held cleaned up.
	struct VulkanDeleteQueue
	{
		void push(std::function<bool()> func)
		{
			mItems.push_back(func);
		}

		void cleanup()
		{
			auto eraseBeginIt = mItems.begin();
			auto eraseEndIt = mItems.end();
			for (auto it = eraseBeginIt; it != eraseEndIt; it++)
			{
				const bool cleanupRes = (*it)();
				if (!cleanupRes)
				{
					eraseBeginIt->swap(*it);
					++eraseBeginIt;
				}
			}
			mItems.erase(eraseBeginIt, eraseEndIt);
		}

	private:
		std::vector<std::function<bool()>> mItems;
	};
} // namespace lune