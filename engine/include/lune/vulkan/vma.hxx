#pragma once

#include "vk_mem_alloc.h"
#include "vulkan_core.hxx"

namespace vma
{
	vk::DeviceSize getAllocationSize(VmaAllocation allocation);
}