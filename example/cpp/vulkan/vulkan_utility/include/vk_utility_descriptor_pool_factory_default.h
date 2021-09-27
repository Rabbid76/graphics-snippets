#pragma once

#include "vk_utility_descriptor_pool_factory.h"

namespace vk_utility
{
    namespace core
    {
        /// <summary>
        /// Descriptor sets can't be created directly, they must be allocated from a pool like command buffers.
        /// The equivalent for descriptor sets is unsurprisingly called a descriptor pool.
        /// </summary>
        class DescriptorPoolFactoryDefault
            : DescriptorPoolFactory
        {
        private:

            uint32_t _no_of_swapchain_images;

        public:

            DescriptorPoolFactory& set_no_of_swapchain_images(uint32_t no_of_swapchain_images)
            {
                _no_of_swapchain_images = no_of_swapchain_images;
                return *this;
            }

            virtual vk::DescriptorPool New(vk::Device device) const override
            {
                std::vector<vk::DescriptorPoolSize> pool_sizes
                {
                    vk::DescriptorPoolSize(vk::DescriptorType::eUniformBuffer, _no_of_swapchain_images),
                    vk::DescriptorPoolSize(vk::DescriptorType::eCombinedImageSampler, _no_of_swapchain_images),
                };
                vk::DescriptorPoolCreateInfo descriptor_pool_information
                (
                    vk::DescriptorPoolCreateFlags{},
                    _no_of_swapchain_images,
                    pool_sizes
                );
                return device.createDescriptorPool(descriptor_pool_information);
            }
        };
    }
}