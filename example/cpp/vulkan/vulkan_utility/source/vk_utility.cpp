
#include <pch.h>

#include <vk_utility_vulkan_include.h>

// Extensions / Per Device function pointers
// https://github.com/KhronosGroup/Vulkan-Hpp#extensions--per-device-function-pointers
//
// Extension function are not automatically loaded.
// The address of an extension function has to be looked up by `vkGetInstanceProcAddr`.

#if VK_UTILITY_DYNAMIC_LOADING == 1

namespace vk
{
    DispatchLoaderDynamic defaultDispatchLoaderDynamic;
}

#else

VkResult vkCreateDebugReportCallbackEXT(
    VkInstance                                instance,
    const VkDebugReportCallbackCreateInfoEXT *pCreateInfo,
    const VkAllocationCallbacks              *pAllocator,
    VkDebugReportCallbackEXT                 *pCallback )
{
    auto func = (PFN_vkCreateDebugReportCallbackEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugReportCallbackEXT");
    if (func != nullptr) {
        return func(instance, pCreateInfo, pAllocator, pCallback);
    } else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

void vkDestroyDebugReportCallbackEXT(
    VkInstance                   instance,
    VkDebugReportCallbackEXT     callback,
    const VkAllocationCallbacks *pAllocator )
{
    auto func = (PFN_vkDestroyDebugReportCallbackEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugReportCallbackEXT");
    if (func != nullptr) {
        func(instance, callback, pAllocator);
    }
}

#endif

namespace vk_utility
{
    /// <summary>
    /// Global initialization of Vulkan utility library
    /// </summary>
    void Init(void)
    {
#if VK_UTILITY_DYNAMIC_LOADING == 1
        vk::DynamicLoader dl;
        PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr = dl.getProcAddress<PFN_vkGetInstanceProcAddr>("vkGetInstanceProcAddr");

        VULKAN_HPP_DEFAULT_DISPATCHER.init(vkGetInstanceProcAddr);
#endif
    }

}