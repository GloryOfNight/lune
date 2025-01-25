#include "lune/vulkan/vulkan_core.hxx"
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_core.h>

#include <SDL3/SDL_vulkan.h>

#if VULKAN_HPP_DISPATCH_LOADER_DYNAMIC == 1
VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE
#endif

vk::DispatchLoaderDynamic& lune::vulkan::getDynamicLoader() noexcept
{
	static vk::DispatchLoaderDynamic loader{};
	return loader;
}

void lune::vulkan::loadVulkanDynamicFunctions()
{
	vk::Instance instance = getVulkanContext().instance;
	auto& d = getDynamicLoader();
	d.vkCmdSetDepthTestEnableEXT = (PFN_vkCmdSetDepthTestEnableEXT)vkGetInstanceProcAddr(instance, "vkCmdSetDepthTestEnableEXT");
	d.vkCmdSetPrimitiveTopologyEXT = (PFN_vkCmdSetPrimitiveTopologyEXT)vkGetInstanceProcAddr(instance, "vkCmdSetPrimitiveTopologyEXT");
}

lune::VulkanContext& lune::getVulkanContext() noexcept
{
	static VulkanContext context{};
	return context;
}

lune::VulkanConfig& lune::getVulkanConfig() noexcept
{
	static VulkanConfig config{};
	return config;
}

lune::VulkanDeleteQueue& lune::getVulkanDeleteQueue() noexcept
{
	static VulkanDeleteQueue queue{};
	return queue;
}
