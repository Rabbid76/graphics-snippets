#pragma once

//#define VK_UTILITY_DYNAMIC_LOADING 1

#if VK_UTILITY_DYNAMIC_LOADING == 1
#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1
#endif


#include <vulkan/vulkan.h>
#include <vulkan/vulkan.hpp>

namespace vk_utility
{

    extern void Init(void);

}
