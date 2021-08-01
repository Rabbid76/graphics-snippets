#pragma once


#include <vk_utility_object.h>
#include <vk_utility_exception.h>
#include <vk_utility_vulkan_include.h>


namespace vk_utility
{
    namespace swap
    {
        class SwapchahinInformation
        {
        private:

            vk::SwapchainCreateInfoKHR _swapchain_information;

        public:

            static SwapchahinInformation New(vk::SwapchainCreateInfoKHR swapchain_information)
            {
                return SwapchahinInformation(swapchain_information);
            }

            SwapchahinInformation(void) = default;
            SwapchahinInformation(const SwapchahinInformation &) = default;
            SwapchahinInformation &operator = (const SwapchahinInformation &) = default;

            SwapchahinInformation(const vk::SwapchainCreateInfoKHR &swapchain_information)
                : _swapchain_information(swapchain_information)
            {}

            operator const vk::SwapchainCreateInfoKHR &() const 
            {
                return _swapchain_information;
            }

            const vk::Format & image_format(void) const 
            { 
                return _swapchain_information.imageFormat; 
            }

            const vk::ColorSpaceKHR & image_color_space(void) const 
            { 
                return _swapchain_information.imageColorSpace; 
            }

            const vk::Extent2D & image_extent_2D(void) const 
            { 
                return _swapchain_information.imageExtent; 
            }

            const vk::PresentModeKHR & present_mode(void) const 
            { 
                return _swapchain_information.presentMode; 
            }
        };
    }
}
