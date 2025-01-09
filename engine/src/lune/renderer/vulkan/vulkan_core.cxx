#include "lune/vulkan/vulkan_core.hxx"

lune::VulkanContext& lune::getVulkanContext()
{
	static VulkanContext context{};
	return context;
}

lune::VulkanConfig& lune::getVulkanConfig()
{
	static VulkanConfig config{};
	return config;
}
