// file contains the implementation of the Vulkan Memory Allocator library
#define VMA_VULKAN_VERSION 1003000 // Vulkan 1.0; ABBBCCC, where A = major, BBB = minor, CCC = patch
#define VMA_STATIC_VULKAN_FUNCTIONS 1
#define VMA_DYNAMIC_VULKAN_FUNCTIONS 0
#define VMA_IMPLEMENTATION
#include "lune/vulkan/vma.hxx"

vk::DeviceSize vma::getAllocationSize(VmaAllocation allocation)
{
	return vk::DeviceSize(allocation->GetSize());
}