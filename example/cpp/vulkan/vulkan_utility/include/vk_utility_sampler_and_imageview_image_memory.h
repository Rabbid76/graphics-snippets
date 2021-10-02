#pragma once

#include "vk_utility_image_view_and_image_memory.h"
#include "vk_utility_sampler.h"
#include "vk_utility_texture_factory.h" 

namespace vk_utility
{
    namespace image
    {
        class SamplerAndImageViewImageMemory;
        using SamplerAndImageViewImageMemoryPtr = vk_utility::Ptr<SamplerAndImageViewImageMemory>;

        class SamplerAndImageViewImageMemory
            : public GenericObject<int>
        {
        private:

            ImageViewAndImageMemoryPtr _image_view_and_memory;
            SamplerPtr _sampler;

        public:

            static SamplerAndImageViewImageMemory New(
                vk::Device device, vk::CommandPool command_pool, const TextureFactory& texture_factory)
            {
                auto [sampler, image_view, image, image_memory, memory_size] = texture_factory
                    .New(device, command_pool);
                return SamplerAndImageViewImageMemory(
                    vk_utility::image::Sampler::NewPtr(device, sampler),
                    vk_utility::image::ImageViewAndImageMemory::NewPtr(device, image_view, image, image_memory, memory_size));
            }

            static SamplerAndImageViewImageMemoryPtr NewPtr(
                vk::Device device, vk::CommandPool command_pool, const TextureFactory& texture_factory)
            {
                return vk_utility::make_shared<SamplerAndImageViewImageMemory>(New(device, command_pool, texture_factory));
            }

            SamplerAndImageViewImageMemory(void) = default;
            SamplerAndImageViewImageMemory(const SamplerAndImageViewImageMemory&) = default;
            SamplerAndImageViewImageMemory& operator = (const SamplerAndImageViewImageMemory&) = default;

            const ImageViewAndImageMemory& image_view_and_memory(void) const { return *_image_view_and_memory; }
            const Sampler sampler(void) const { return *_sampler; }

            SamplerAndImageViewImageMemory(SamplerPtr sampler, ImageViewAndImageMemoryPtr image_view_and_memory)
                : GenericObject(0)
                , _sampler(sampler)
                , _image_view_and_memory(image_view_and_memory)
            {}

            virtual void destroy(void) override
            {
                _sampler = nullptr;
                _image_view_and_memory = nullptr;
            }
        };
    }
}