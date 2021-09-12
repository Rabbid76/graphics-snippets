#pragma once

#include "vk_utility_device_memory_factory.h"
#include "vk_utility_physical_device.h"

#include <functional>

namespace vk_utility
{
    namespace image
    {
        class ImageDeviceMemoryFactory
            : public device::DeviceMemoryFactory
        {
        public:

            using FindMemoryTypeFunction = std::function<uint32_t(uint32_t, vk::MemoryPropertyFlags)>;

        private:

            vk::Image _image;
            vk::MemoryPropertyFlags _properties;
            FindMemoryTypeFunction _find_memory_type;
            vk_utility::device::PhysicalDevice* _physical_device;

        public:

            ImageDeviceMemoryFactory& set_image(vk::Image image)
            {
                _image = image;
                return *this;
            }

            ImageDeviceMemoryFactory& set_memory_properties(vk::MemoryPropertyFlags properties)
            {
                _properties = properties;
                return *this;
            }

            ImageDeviceMemoryFactory& set_from_physical_device(const vk_utility::device::PhysicalDevice& physical_device)
            {
                _find_memory_type = [physical_device](uint32_t mempry_type_btis, vk::MemoryPropertyFlags properties) -> uint32_t
                {
                    return physical_device.find_memory_type(mempry_type_btis, properties);
                };
                return *this;
            }

            virtual std::tuple<vk::DeviceMemory, vk::DeviceSize> New(vk::Device device) const override
            {
                const vk::MemoryRequirements memory_requirements = device.getImageMemoryRequirements(_image);
                const vk::MemoryAllocateInfo allocate_information
                (
                    memory_requirements.size,
                    _find_memory_type(memory_requirements.memoryTypeBits, _properties)
                );
                vk::DeviceMemory image_memory = device.allocateMemory(allocate_information);
                device.bindImageMemory(_image, image_memory, 0);
                return { image_memory, memory_requirements.size };
            }
        };
    }
}