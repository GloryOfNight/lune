#pragma once

#include "vulkan_core.hxx"

#include "vk_mem_alloc.h"

namespace vma
{
    vk::DeviceSize getAllocationSize(VmaAllocation allocation);
}