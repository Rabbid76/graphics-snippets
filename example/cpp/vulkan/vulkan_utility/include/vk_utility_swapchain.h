#pragma once


#include <vk_utility_object.h>
#include <vk_utility_exception.h>
#include <vk_utility_vulkan_include.h>
#include <vk_utility_device.h>
#include <vk_utility_swapchain_information.h>
#include <vk_utility_image.h>

#include <memory>
#include <vector>
#include <algorithm>
#include <iterator>


namespace vk_utility
{

    namespace swap
    {
        class Swapchain;
        using SwapchainPtr = vk_utility::Ptr<Swapchain>;


        /// <summary>
        /// Swapchain - `vk::SwapchainKHR`
        /// 
        /// Vulkan does not have the concept of a "default framebuffer", 
        /// hence it requires an infrastructure that will own the buffers we will render to before we visualize them on the screen.
        /// This infrastructure is known as the swap chain and must be created explicitly in Vulkan.
        /// The swap chain is essentially a queue of images that are waiting to be presented to the screen.
        /// 
        /// </summary>
        class Swapchain
            : public GenericObject<vk::SwapchainKHR>
        {
        private:

            vk_utility::device::DevicePtr _device;
            SwapchahinInformation _swapchain_information;
            mutable std::vector<vk_utility::image::Image> _swapchain_images;

        public:

            static Swapchain Create(vk_utility::device::DevicePtr device, const SwapchahinInformation &swapchain_information)
            {
                return Swapchain(device, device->get().create_swapchain(swapchain_information), swapchain_information);
            }

            static SwapchainPtr New(vk_utility::device::DevicePtr device, const SwapchahinInformation &swapchain_information)
            {
                return vk_utility::make_shared(Create(device, swapchain_information));
            }

            Swapchain(void) = default;
            Swapchain(const Swapchain &) = default;
            Swapchain &operator = (const Swapchain &) = default;

            Swapchain(vk::SwapchainKHR device)
                : GenericObject(device)
            {}

            Swapchain(vk_utility::device::DevicePtr device, vk::SwapchainKHR swapchain, const SwapchahinInformation &swapchain_information)
                : GenericObject(swapchain)
                , _device(device)
                , _swapchain_information(swapchain_information)
            {}

            virtual void destroy(void) override
            {
                if (_vk_object && _device)
                {
                    _device->get().destroy_swapchain(_vk_object);
                    _vk_object = vk::SwapchainKHR();
                }
            }

            uint32_t image_width_2D(void)
            {
                return image_extent_2D().width;
            }

            uint32_t image_height_2D(void)
            {
                return image_extent_2D().height;
            }

            const vk::Format & image_format(void) const 
            { 
                return _swapchain_information.image_format(); 
            }

            const vk::ColorSpaceKHR & image_color_space(void) const
            { 
                return _swapchain_information.image_color_space(); 
            }

            const vk::Extent2D & image_extent_2D(void) const 
            { 
                return _swapchain_information.image_extent_2D(); 
            }

            const vk::PresentModeKHR & present_mode(void) const 
            { 
                return _swapchain_information.present_mode(); 
            }

            const auto & get_swapchain_images() const
            {
                if (_swapchain_images.empty())
                {
                    auto swapchain_images = _device->get().get_swapchain_images(_vk_object);
                    std::copy(swapchain_images.begin(), swapchain_images.end(), std::back_inserter(_swapchain_images));
                }
                return _swapchain_images; 
            }

            uint32_t no_of_swapchain_images(void) const
            {
                return static_cast<uint32_t>(get_swapchain_images().size());
            }
        };
    }
}
