#pragma once

#include "vk_utility_image_view_and_image_memory_factory.h"
#include "vk_utility_image_and_memory_factory.h"
#include "vk_utility_image_view_factory.h"

namespace vk_utility
{
    namespace image
    {
        class ImageViewAndImageMemoryFactoryDefault
            : public ImageViewAndImageMemoryFactory
        {
        private:

            ImageAndMemoryFactory *_image_and_memory_factory;
            ImageViewFactory *_image_view_factory;

        public:

            auto& set_image_and_memory_factory(ImageAndMemoryFactory *image_and_memory_factory)
            {
                _image_and_memory_factory = image_and_memory_factory;
                return *this;
            }

            auto& set_image_view_factory(ImageViewFactory *image_view_factory)
            {
                _image_view_factory = image_view_factory;
                return *this;
            }

            std::tuple<vk::ImageView, vk::Image, vk::DeviceMemory, vk::DeviceSize> New(vk::Device device) const override
            {
                auto [image, image_memory, memory_size] = _image_and_memory_factory->New(device);
                auto image_view = _image_view_factory
                    ->set_image(image)
                    .New(device);
                return std::make_tuple(image_view, image, image_memory, memory_size);
            }
        };
    }
}
