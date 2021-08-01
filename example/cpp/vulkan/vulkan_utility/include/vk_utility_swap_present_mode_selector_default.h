#pragma once


#include <vk_utility_object.h>
#include <vk_utility_exception.h>
#include <vk_utility_vulkan_include.h>
#include <vk_utility_physical_device.h>
#include <vk_utility_swap_present_mode_selector.h>


namespace vk_utility
{

    namespace swap
    {
        /// <summary>
        /// Interface for Vulkan swap present mode selection (vk::PresentModeKHR`)
        /// </summary>
        class SwapPresentModeSelectorDefault
            : public ISwapPresentModeSelector
        {
        public:

            SwapPresentModeSelectorDefault(void) = default;
            SwapPresentModeSelectorDefault(const SwapPresentModeSelectorDefault&) = delete;

            virtual vk::PresentModeKHR Select(device::PhysicalDevicePtr physical_device) override
            {
                auto &swapchain_support = physical_device->get().get_swapchain_support();
                auto &available_present_modes = swapchain_support._presentModes;

                // The presentation mode is arguably the most important setting for the swap chain, 
                // because it represents the actual conditions for showing images to the screen.
                // There are four possible modes available in Vulkan:
                // - `VK_PRESENT_MODE_IMMEDIATE_KHR`: Images submitted by your application are transferred to the screen right away,
                //   which may result in tearing.
                // - `VK_PRESENT_MODE_FIFO_KHR`: The swap chain is a queue where the display takes an image
                //   from the front of the queue when the display is refreshed and the program inserts rendered images
                //   at the back of the queue.
                //   If the queue is full then the program has to wait. This is most similar to vertical sync as found in modern games.
                //   The moment that the display is refreshed is known as "vertical blank".
                // - `VK_PRESENT_MODE_FIFO_RELAXED_KHR`: This mode only differs from the previous one if the application is late
                //   and the queue was empty at the last vertical blank.
                //   Instead of waiting for the next vertical blank, the image is transferred right away when it finally arrives.
                //   This may result in visible tearing.
                // - `VK_PRESENT_MODE_MAILBOX_KHR`: This is another variation of the second mode. 
                //   Instead of blocking the application when the queue is full, the images that are already queued
                //   are simply replaced with the newer ones.
                //   This mode can be used to implement triple buffering, which allows you to avoid tearing 
                //   with significantly less latency issues than standard vertical sync that uses double buffering.

                vk::PresentModeKHR best_mode = vk::PresentModeKHR::eFifo;
                for (const auto &available_present_mode : available_present_modes)
                {
                    if (available_present_mode == vk::PresentModeKHR::eMailbox)
                        return available_present_mode;
                    else if (available_present_mode == vk::PresentModeKHR::eImmediate)
                        best_mode = available_present_mode;
                }
                return best_mode;
            }
        };
    }
}
