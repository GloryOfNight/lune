#include "lune/vulkan/vulkan_core.hxx"

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
