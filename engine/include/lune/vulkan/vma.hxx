#pragma once

#include "vulkan/vulkan.hpp"

#include "vk_mem_alloc.h"

namespace vma
{
    vk::DeviceSize getAllocationSize(VmaAllocation allocation);
}