#pragma once

#include "vk_utility_object.h"
#include "vk_utility_exception.h"
#include "vk_utility_vulkan_include.h"
#include "vk_utility_device.h"
#include "vk_utility_device_memory.h"
#include "vk_utility_image_view.h"
#include "vk_utility_image.h"
#include "vk_utility_image_view_and_image_memory_factory.h"

namespace vk_utility
{
    namespace image
    {
        class ImageViewAndImageMemory;
        using ImageViewAndImageMemoryPtr = vk_utility::Ptr<ImageViewAndImageMemory>;

        class ImageViewAndImageMemory
            : public GenericObject<int>
        {
        private:

            device::DeviceMemoryPtr _device_memory;
            ImagePtr _image;
            ImageViewPtr _image_view;

        public:

            static ImageViewAndImageMemory New(
                vk::Device device, vk::ImageView image_view, vk::Image image, vk::DeviceMemory image_memory, vk::DeviceSize memory_size)
            {
                return ImageViewAndImageMemory(
                    make_shared(vk_utility::device::DeviceMemory(device, image_memory, memory_size)),
                    make_shared(vk_utility::image::Image(device, image)),
                    make_shared(vk_utility::image::ImageView(device, image_view)));
            }

            static ImageViewAndImageMemoryPtr NewPtr(
                vk::Device device, vk::ImageView image_view, vk::Image image, vk::DeviceMemory image_memory, vk::DeviceSize memory_size)
            {
                return make_shared(New(device, image_view, image, image_memory, memory_size));
            }

            static ImageViewAndImageMemory New(vk::Device device, ImageViewAndImageMemoryFactory& image_view_and_image_memory_factory)
            {
                auto [image_view, image, image_memory, memory_size] = image_view_and_image_memory_factory.New(device);
                return ImageViewAndImageMemory(
                    make_shared(vk_utility::device::DeviceMemory(device, image_memory, memory_size)),
                    make_shared(vk_utility::image::Image(device, image)),
                    make_shared(vk_utility::image::ImageView(device, image_view)));
            }

            static ImageViewAndImageMemoryPtr NewPtr(vk::Device device, ImageViewAndImageMemoryFactory& image_view_and_image_memory_factory)
            {
                return vk_utility::make_shared<ImageViewAndImageMemory>(New(device, image_view_and_image_memory_factory));
            }

            ImageViewAndImageMemory(void) = default;
            ImageViewAndImageMemory(const ImageViewAndImageMemory&) = default;
            ImageViewAndImageMemory& operator = (const ImageViewAndImageMemory&) = default;

            const device::DeviceMemory& device_memory(void) const { return *_device_memory; }
            const Image image(void) const { return *_image; }
            const ImageView image_view(void) const { return *_image_view; }

            ImageViewAndImageMemory(device::DeviceMemoryPtr device_memory, ImagePtr image, ImageViewPtr image_view)
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
