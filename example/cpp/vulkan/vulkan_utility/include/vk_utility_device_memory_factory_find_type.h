#pragma once

#include "vk_utility_device_memory_factory.h"
#include "vk_utility_physical_device.h"

namespace vk_utility
{
    namespace device
    {
        class DeviceMemoryFactoryFindType
            : public DeviceMemoryFactory
        {
        public:

            using FindMemoryTypeFunction = std::function<uint32_t(uint32_t, vk::MemoryPropertyFlags)>;

        private:

            vk::DeviceSize _memory_size;
            vk::MemoryRequirements _memory_requirements;
            vk::MemoryPropertyFlags _properties = vk::MemoryPropertyFlagBits::eDeviceLocal;
            FindMemoryTypeFunction _find_memory_type;

        private:

            DeviceMemoryFactoryFindType& set_memory_size(vk::DeviceSize memory_size)
            {
                _memory_size = memory_size;
                return *this;
            }

            class DeviceMemoryFactoryFindType& set_memory_properties(vk::MemoryRequirements memory_requirements)
            {
                _memory_requirements = memory_requirements;
                return *this;
            }

            class DeviceMemoryFactoryFindType& set_memory_properties(vk::MemoryPropertyFlags properties)
            {
                _properties = properties;
                return *this;
            }

            class DeviceMemoryFactoryFindType& set_from_physical_device(const vk_utility::device::PhysicalDevice& physical_device)
            {
                _find_memory_type = [physical_device](uint32_t mempry_type_btis, vk::MemoryPropertyFlags properties) -> uint32_t
                {
                    return physical_device.find_memory_type(mempry_type_btis, properties);
                };
                return *this;
            }

            virtual std::tuple<vk::DeviceMemory, vk::DeviceSize> New(vk::Device device) const override
            {
                const vk::MemoryAllocateInfo allocate_information
                (
                    _memory_size,
                    _find_memory_type(_memory_requirements.memoryTypeBits, _properties)
                );
                vk::DeviceMemory memory = device.allocateMemory(allocate_information);
                return { memory, _memory_size };
            }
        };
    }
}
