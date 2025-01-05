#include "vulkan_core.hxx"

lune::vulkan_context& lune::getVulkanContext()
{
	static vulkan_context context{};
	return context;
}

lune::vulkan_config& lune::getVulkanConfig()
{
	static vulkan_config config{};
	return config;
}
