#include "lune/vulkan/vulkan_core.hxx"

#if VULKAN_HPP_DISPATCH_LOADER_DYNAMIC == 1
VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE
#endif

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
