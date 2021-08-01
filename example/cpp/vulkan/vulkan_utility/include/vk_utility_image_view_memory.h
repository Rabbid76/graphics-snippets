#pragma once


#include "vk_utility_object.h"
#include "vk_utility_exception.h"
#include "vk_utility_vulkan_include.h"
#include "vk_utility_device.h"
#include "vk_utility_device_memory.h"
#include "vk_utility_image_view.h"
#include "vk_utility_image.h"

#include <memory>


namespace vk_utility
{
    namespace image
    {
        class ImageViewMemory;
        using ImageViewMemoryPtr = vk_utility::Ptr<ImageViewMemory>;

        /// <summary>
        /// Collection of Image, ImageView and DeviceMemory (`vk::Image`, `vk::ImageView`, `vk::DeviceMemory`)
        /// </summary>
        class ImageViewMemory
            : public GenericObject<int>
        {
        private:

            device::DeviceMemoryPtr _device_memory;
            ImagePtr _image;
            ImageViewPtr _image_view;

        public:

            static ImageViewMemory Create(device::DeviceMemoryPtr device_memory, ImagePtr image, ImageViewPtr image_view)
            {
                return ImageViewMemory(device_memory, image, image_view);
            }

            static ImageViewMemoryPtr New(device::DeviceMemoryPtr device_memory, ImagePtr image, ImageViewPtr image_view)
            {
                return vk_utility::make_shared(Create(device_memory, image, image_view));
            }

            ImageViewMemory(void) = default;
            ImageViewMemory(const ImageViewMemory&) = default;
            ImageViewMemory& operator = (const ImageViewMemory&) = default;

            const device::DeviceMemory& device_memory(void) const { return *_device_memory; }
            const Image  image(void) const { return *_image; }
            const ImageView  image_view(void) const { return *_image_view; }

            ImageViewMemory(device::DeviceMemoryPtr device_memory, ImagePtr image, ImageViewPtr image_view)
                : GenericObject(0)
                , _device_memory(device_memory)
                , _image(image)
                , _image_view(image_view)
            {}

            virtual void destroy(void) override
            {
                _image_view = nullptr;
                _image = nullptr;
                _device_memory = nullptr;
            }
        };
    }
}
