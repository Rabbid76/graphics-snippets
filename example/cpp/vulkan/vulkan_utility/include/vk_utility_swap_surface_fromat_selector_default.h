#pragma once


#include <vk_utility_object.h>
#include <vk_utility_exception.h>
#include <vk_utility_vulkan_include.h>
#include <vk_utility_physical_device.h>
#include <vk_utility_swap_surface_format_selector.h>


namespace vk_utility
{

    namespace swap
    {
        /// <summary>
        /// Interface for Vulkan swap surface format selection (vk::PresentModeKHR`)
        /// </summary>
        class SwapSurfaceFormatSelectorDefault
            : public ISwapSurfaceFormatSelector
        {
        public:

            SwapSurfaceFormatSelectorDefault(void) = default;
            SwapSurfaceFormatSelectorDefault(const SwapSurfaceFormatSelectorDefault&) = delete;

            virtual vk::SurfaceFormatKHR Select(device::PhysicalDevicePtr physical_device) override
            {
                auto &swapchain_support = physical_device->get().get_swapchain_support();
                auto &available_formats = swapchain_support._formats;

                // Each `vk::SurfaceFormatKHR` entry contains a format and a colorSpace member.
                // The format member specifies the color channels and types.
                // For example, `VK_FORMAT_B8G8R8A8_UNORM` means that we store the B, G, R and alpha channels
                // in that order with an 8 bit unsigned integer for a total of 32 bits per pixel.
                // The colorSpace member indicates if the SRGB color space is supported or not using the
                // `VK_COLOR_SPACE_SRGB_NONLINEAR_KHR` flag.
                // Note that this flag used to be called `VK_COLORSPACE_SRGB_NONLINEAR_KHR` in old versions of the specification.
    
                // The best case scenario is that the surface has no preferred format,
                // which Vulkan indicates by only returning one `vk::SurfaceFormatKHR`
                // entry which has its format member set to VK_FORMAT_UNDEFINED.
                if (available_formats.size() == 1 && available_formats[0].format == vk::Format::eUndefined)
                {
                    return {vk::Format::eB8G8R8A8Unorm, vk::ColorSpaceKHR::eSrgbNonlinear};
                }

                for (const auto & available_format : available_formats)
                {
                    if (available_format.format == vk::Format::eB8G8R8A8Unorm && 
                        available_format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear)
                    {
                        return available_format;
                    }
                }

                assert(false);
                return available_formats[0];
            }
        };
    }
}