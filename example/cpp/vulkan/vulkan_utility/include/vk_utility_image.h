#pragma once


#include "vk_utility_object.h"
#include "vk_utility_vulkan_include.h"
#include "vk_utility_image_factory.h"

#include <memory>


namespace vk_utility
{
    namespace image
    {
        class Image;
        using ImagePtr = vk_utility::Ptr<Image>;

        /// <summary>
        /// Image (`vk::Image`)
        /// </summary>
        class Image
            : public GenericObject<vk::Image>
        {
        private:

            vk::Device _device;

        public:

            static Image New(vk::Device device, const ImageFactory& image_factory)
            {
                return Image(device, image_factory.New(device));
            }

            static ImagePtr NewPtr(vk::Device device, const ImageFactory& image_factory)
            {
                return vk_utility::make_shared(New(device, image_factory));
            }

            Image(void) = default;
            Image(const Image &) = default;
            Image &operator = (const Image &) = default;

            Image(vk::Image image)
                : GenericObject(image)
            {}

            Image(vk::Device device, vk::Image image)
                : GenericObject(image)
                , _device(device)
            {}

            virtual void destroy(void) override
            {
                if (_vk_object && _device)
                {
                    _device.destroyImage(_vk_object);
                    _vk_object = vk::Image();
                }
            }
        };
    }
}
