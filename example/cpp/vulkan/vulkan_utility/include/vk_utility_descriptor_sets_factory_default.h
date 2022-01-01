#pragma once

#include "vk_utility_descriptor_sets_factory.h"
#include "vk_utility_buffer_and_memory.h"
#include "vk_utility_sampler_and_imageview_image_memory.h"

namespace vk_utility
{
    namespace core
    {
        class DescriptorSetsFactoryDefault
            : public DescriptorSetsFactory
        {
        private:

            uint32_t _no_of_swapchain_images;
            vk::DescriptorSetLayout _descriptor_set_layout;
            vk::DescriptorPool _descriptor_pool;
            const std::vector<vk_utility::buffer::BufferAndMemoryPtr> *_uniform_buffers;
            size_t _uniform_buffer_size;
            const std::vector<vk_utility::image::SamplerAndImageViewImageMemoryPtr> *_texture_samplers;

        public:

            DescriptorSetsFactoryDefault& set_no_of_swapchain_images(uint32_t no_of_swapchain_images)
            {
                _no_of_swapchain_images = no_of_swapchain_images;
                return *this;
            }

            DescriptorSetsFactoryDefault& set_descriptor_set_layout(vk::DescriptorSetLayout descriptor_set_layout)
            {
                _descriptor_set_layout = descriptor_set_layout;
                return *this;
            }

            DescriptorSetsFactoryDefault& set_descriptor_pool(vk::DescriptorPool descriptor_pool)
            {
                _descriptor_pool = descriptor_pool;
                return *this;
            }

            DescriptorSetsFactoryDefault& set_uniform_buffers(
                const std::vector<vk_utility::buffer::BufferAndMemoryPtr>* uniform_buffers,
                size_t uniform_buffer_size)
            {
                _uniform_buffers = uniform_buffers;
                _uniform_buffer_size = uniform_buffer_size;
                return *this;
            }

            DescriptorSetsFactoryDefault& set_texture_samplers(
                const std::vector<vk_utility::image::SamplerAndImageViewImageMemoryPtr>* texture_samplers)
            {
                _texture_samplers = texture_samplers;
                return *this;
            }

            virtual std::vector<vk::DescriptorSet> New(vk::Device device) const override
            {
                // A descriptor set allocation is described with a `vk::DescriptorSetAllocateInfo` structures.
                // You need to specify the descriptor pool to allocate from, the number of descriptor sets to allocate, and the descriptor layout to base them on:

                std::vector<vk::DescriptorSetLayout> layouts(_no_of_swapchain_images, _descriptor_set_layout);
                vk::DescriptorSetAllocateInfo allocInfo(_descriptor_pool, layouts);

                auto descriptor_sets = device.allocateDescriptorSets(allocInfo);

                // The descriptor sets have been allocated now, but the descriptors within still need to be configured.
                // We'll now add a loop to populate every descriptor:

                // Descriptors that refer to buffers, like our uniform buffer descriptor, are configured with a `vk::DescriptorBufferInfo` structures.
                // This structure specifies the buffer and the region within it that contains the data for the descriptor.

                for (size_t i = 0; i < _no_of_swapchain_images; i++) {

                    std::vector<vk::WriteDescriptorSet> descriptor_writes;

                    // If you're overwriting the whole buffer, like we are in this case, then it is also possible to use the VK_WHOLE_SIZE value for the range.
                    // The configuration of descriptors is updated using the vkUpdateDescriptorSets function, which takes an array of `vk::WriteDescriptorSet` structures as parameter.

                    // The first two fields specify the descriptor set to update and the binding.
                    // We gave our uniform buffer binding index 0. Remember that descriptors can be arrays, so we also need to specify the first index in the array that we want to update.
                    // We're not using an array, so the index is simply 0.

                    // We need to specify the type of descriptor again.
                    // It's possible to update multiple descriptors at once in an array, starting at index dstArrayElement.
                    // The descriptorCount field specifies how many array elements you want to update.

                    std::vector<vk::DescriptorImageInfo> image_infos;
                    std::vector<vk::DescriptorBufferInfo> buffer_infos
                    {
                        vk::DescriptorBufferInfo((*_uniform_buffers)[i]->get().buffer(), 0, _uniform_buffer_size)
                    };
                    std::vector<vk::BufferView> texel_buffer_view;
                    descriptor_writes.push_back(
                        vk::WriteDescriptorSet
                        (
                            descriptor_sets[i],
                            0,
                            0,
                            vk::DescriptorType::eUniformBuffer,
                            image_infos,
                            buffer_infos,
                            texel_buffer_view
                        ));

                    // The last field references an array with descriptorCount structures that actually configure the descriptors.
                    // It depends on the type of descriptor which one of the three you actually need to use.
                    // The pBufferInfo field is used for descriptors that refer to buffer data, pImageInfo is used for descriptors that refer to image data,
                    // and pTexelBufferView is used for descriptors that refer to buffer views.
                    // Our descriptor is based on buffers, so we're using pBufferInfo.

                    for (size_t j = 0; j < _texture_samplers->size(); ++j)
                    {
                        vk::DescriptorImageInfo image_info(
                            (*_texture_samplers)[j]->get().sampler(),
                            (*_texture_samplers)[j]->get().image_view_and_memory().image_view(),
                            vk::ImageLayout::eShaderReadOnlyOptimal);

                        descriptor_writes.push_back(
                            vk::WriteDescriptorSet
                            (
                                descriptor_sets[i],
                                1,
                                0,
                                1,
                                vk::DescriptorType::eCombinedImageSampler,
                                &image_info,
                                nullptr,
                                nullptr
                            ));
                    }

                    //! The updates are applied using vkUpdateDescriptorSets. It accepts two kinds of arrays as parameters:
                    //! an array of `vk::WriteDescriptorSet` and an array of `vk::CopyDescriptorSet`. 
                    //! The latter can be used to copy descriptors to each other, as its name implies.

                    device.updateDescriptorSets(descriptor_writes, nullptr);
                }

                return descriptor_sets;
            }
        };
    }
}
