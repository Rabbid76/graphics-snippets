#pragma once


#include <vk_utility_object.h>
#include <vk_utility_exception.h>
#include <vk_utility_vulkan_include.h>
#include <vk_utility_physical_device.h>
#include <vk_utility_swap_extent_selector.h>

#include <vk_glfw_utility_include.h>

#include<algorithm>


namespace vk_glfw_utility
{

    namespace swap
    {
        /// <summary>
        /// Interface for Vulkan swap extent selection (`vk::Extent2D`)
        /// </summary>
        class SwapExtentSelector
            : public vk_utility::swap::ISwapExtentSelector
        {
        private:

            GLFWwindow *_window = nullptr; 

        public:

            SwapExtentSelector(void) = default;

            SwapExtentSelector(GLFWwindow *window)
                : _window(window)
            {}

            SwapExtentSelector(const SwapExtentSelector&) = delete;

            SwapExtentSelector & set_window(GLFWwindow *window) 
            { 
                _window = window;
                return *this; 
            }

            virtual vk::Extent2D Select(vk_utility::device::PhysicalDevicePtr physical_device) override
            {
                auto &swapchain_support = physical_device->get().get_swapchain_support();
                auto &capabilities = swapchain_support._capabilities;

                // The swap extent is the resolution of the swap chain images and it's almost always exactly equal
                // to the resolution of the window that we're drawing to.
                // The range of the possible resolutions is defined in the `vk::SurfaceCapabilitiesKHR` structure.
                // Vulkan tells us to match the resolution of the window by setting the width and height in the currentExtent member.
                // However, some window managers do allow us to differ here and this is indicated
                // by setting the width and height in currentExtent to a special value: the maximum value of `uint32_t`.
                // In that case we'll pick the resolution that best matches the window
                // within the `minImageExtent` and `maxImageExtent` bounds.

                if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
                    return capabilities.currentExtent;
                } else {

                    //! To handle window resizes properly, we also need to query the current size of the framebuffer to make sure that the swap chain images have the (new) right size.

                    int width = 0, height = 0;
                    while (width == 0 || height == 0) {
                        glfwGetFramebufferSize(_window, &width, &height);
                        glfwWaitEvents();
                    }
        
                    vk::Extent2D actual_extent
                    {
                        static_cast<uint32_t>(width),
                        static_cast<uint32_t>(height)
                    };

                    actual_extent.width  = std::max( capabilities.minImageExtent.width, std::min( capabilities.maxImageExtent.width, actual_extent.width ) );
                    actual_extent.height = std::max( capabilities.minImageExtent.height, std::min( capabilities.maxImageExtent.height, actual_extent.height ) );
       
                    return actual_extent;
                }
            }
        };
    }
}
