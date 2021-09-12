#pragma once


#include <vk_utility_object.h>
#include <vk_utility_exception.h>
#include <vk_utility_vulkan_include.h>
#include <vk_utility_parameter_helper.h>
#include <vk_utility_device.h>
#include <vk_utility_swapchain.h>
#include <vk_utility_swapchain_factory.h>
#include <vk_utility_swap_surface_format_selector.h>
#include <vk_utility_swap_present_mode_selector.h>
#include <vk_utility_swap_extent_selector.h>

#include <memory>
#include <vector>
#include <set>


namespace vk_utility
{

    namespace swap
    {
        /// <summary>
        /// Interface for Vulkan swapchain (`vk::SwapchainKHR`) factories
        /// </summary>
        class SwapchainFactoryDefault
            : public ISwapchainFactory
        {
        private:

            SwapSurfaceFormatSelectorPtr _format_selector;
            SwapPresentModeSelectorPtr _present_mode_selector;
            SwapExtentSelectorPtr _extent_selector;

        public:

            SwapchainFactoryDefault(void) = default;
            SwapchainFactoryDefault(const SwapchainFactoryDefault&) = delete;

            SwapchainFactoryDefault & set_format_selector(SwapSurfaceFormatSelectorPtr format_selector)
            {
                _format_selector = format_selector;
                return *this;
            }

            SwapchainFactoryDefault & set_present_mode_selector(SwapPresentModeSelectorPtr present_mode_selector)
            {
                _present_mode_selector = present_mode_selector;
                return *this;
            }

            SwapchainFactoryDefault & set_extent_selector(SwapExtentSelectorPtr extent_selector)
            {
                _extent_selector = extent_selector;
                return *this;
            }

            virtual Swapchain Create(vk_utility::device::DevicePtr device, vk_utility::device::SurfacePtr surface) override
            {
                auto physical_device = device->get().physical_device_ptr();
                auto &swapchain_support = physical_device->get().get_swapchain_support();

                auto surface_format = _format_selector->Select(physical_device);
                auto present_mode = _present_mode_selector->Select(physical_device);
                auto extent = _extent_selector->Select(physical_device);

                // determine the number o f images in the swap chain
                uint32_t image_count = swapchain_support._capabilities.minImageCount + 1;
                if (swapchain_support._capabilities.maxImageCount > 0 && image_count > swapchain_support._capabilities.maxImageCount)
                {
                    image_count = swapchain_support._capabilities.maxImageCount;
                }

                // TODO [...] move to `SwapchainInformation`

                // Next, we need to specify how to handle swap chain images that will be used across multiple queue families.
                // That will be the case in our application if the graphics queue family is different from the presentation queue.
                // We'll be drawing on the images in the swap chain from the graphics queue and then submitting them on the presentation queue.
                // There are two ways to handle images that are accessed from multiple queues:
                // - `VK_SHARING_MODE_EXCLUSIVE`: An image is owned by one queue family at a time and ownership
                //   must be explicitly transfered before using it in another queue family.
                //   This option offers the best performance.
                // - `VK_SHARING_MODE_CONCURRENT`: Images can be used across multiple queue families without explicit ownership transfers.

                // If the graphics queue family and presentation queue family are the same, which will be the case on most hardware,
                // then we should stick to exclusive mode, because concurrent mode requires you to specify at least two distinct queue families.

                const auto &indices = physical_device->get().get_queue_information();
                std::vector<uint32_t> queue_family_indices{(uint32_t)indices._graphics[0], (uint32_t)indices._surface_support[0]};
                bool graphiscs_equal_presentation = indices._graphics[0] != indices._surface_support[0];

                // We can specify that a certain transform should be applied to images in the swap chain
                // if it is supported (`supportedTransforms` in `capabilities`),
                // like a 90 degree clockwise rotation or horizontal flip. 
                // To specify that you do not want any transformation, simply specify the current transformation.
                
                // The compositeAlpha field specifies if the alpha channel should be used for blending with other windows
                // in the window system.
                // You'll almost always want to simply ignore the alpha channel, hence `VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR`.
                
                // The presentMode member speaks for itself.
                // If the clipped member is set to VK_TRUE then that means that we don't care about the color of pixels
                // that are obscured, for example because another window is in front of them.
                // Unless you really need to be able to read these pixels back and get predictable results,
                // you'll get the best performance by enabling clipping.
                
                // That leaves one last field, oldSwapChain.
                // With Vulkan it's possible that your swap chain becomes invalid or unoptimized while your application is running,
                // for example because the window was resized. 
                // In that case the swap chain actually needs to be recreated from scratch
                // a reference to the old one must be specified in this field.
               
                std::vector<uint32_t> empty_queue_family_indices;
                vk::SwapchainCreateInfoKHR create_inforamtion
                (
                    vk::SwapchainCreateFlagsKHR{},
                    *surface,
                    image_count,
                    surface_format.format,
                    surface_format.colorSpace,
                    extent,
                    1,
                    vk::ImageUsageFlagBits::eColorAttachment,
                    graphiscs_equal_presentation ? vk::SharingMode::eConcurrent : vk::SharingMode::eExclusive,
                    graphiscs_equal_presentation ? queue_family_indices : empty_queue_family_indices,
                    swapchain_support._capabilities.currentTransform,
                    vk::CompositeAlphaFlagBitsKHR::eOpaque,
                    present_mode,
                    VK_TRUE,
                    vk::SwapchainKHR()
                );

                return Swapchain::Create(device, create_inforamtion);
            }

            virtual SwapchainPtr New(vk_utility::device::DevicePtr device, vk_utility::device::SurfacePtr surface) override
            {
                return vk_utility::make_shared(Create(device, surface));
            }
        };
    }
}
