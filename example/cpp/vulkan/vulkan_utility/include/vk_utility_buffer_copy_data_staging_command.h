#pragma once

#include "vk_utility_core_command.h"
#include "vk_utility_command_buffer_factory.h"
#include "vk_utility_buffer_factory_default.h"
#include "vk_utility_buffer_and_memory.h"
#include "vk_utility_buffer_and_memory_factory_default.h"
#include "vk_utility_buffer_copy_data_to_memory_command.h"
#include "vk_utility_buffer_copy_to_buffer_command.h"
#include "vk_utility_command_buffer_factory.h"

namespace vk_utility
{
    namespace buffer
    {
        class CopyDataToBufferStagingCommand
            : public core::CoreCommand
        {
        private:

            const command::CommandBufferFactory* _command_buffer_factory;
            device::PhysicalDevicePtr _phyiscal_dievice;
            vk::DeviceSize _size;
            const void* _source_data;
            vk::Buffer _destination_buffer;

        public:

            CopyDataToBufferStagingCommand& set_command_buffer_factory(const command::CommandBufferFactory* command_buffer_factory)
            {
                _command_buffer_factory = command_buffer_factory;
                return *this;
            }

            CopyDataToBufferStagingCommand& set_physical_device(device::PhysicalDevicePtr phyiscal_dievice)
            {
                _phyiscal_dievice = phyiscal_dievice;
                return *this;
            }

            CopyDataToBufferStagingCommand& set_source_data(vk::DeviceSize size, const void* source_data)
            {
                _size = size;
                _source_data = source_data;
                return *this;
            }

            CopyDataToBufferStagingCommand& set_destination_buffer(vk::Buffer destination_buffer)
            {
                _destination_buffer = destination_buffer;
                return *this;
            }

            virtual void execute_command(vk::Device device, vk::CommandPool command_pool) const override
            {
                auto staging_buffer = BufferAndMemory::NewPtr(
                    device,
                    BufferAndMemoryFactoryDefault()
                    .set_buffer_factory(
                        &BufferFactoryDefault()
                        .set_buffer_size(_size)
                        .set_staging_buffer_usage())
                    .set_buffer_memory_factory(
                        &BufferDeviceMemoryFactory()
                        .set_staging_memory_properties()
                        .set_from_physical_device(*_phyiscal_dievice)));

                CopyDataToBufferMemoryCommand()
                    .set_source_data(_size, _source_data)
                    .set_destination_offset(0)
                    .set_destination_memory(staging_buffer->get().memory())
                    .execute_command(device, command_pool);

                CopyBufferMemoryToBufferCommand()
                    .set_command_buffer_factory(_command_buffer_factory)
                    .set_source_buffer(staging_buffer->get().buffer())
                    .set_destination_buffer(_destination_buffer)
                    .set_copy_size(_size)
                    .execute_command(device, command_pool);

                staging_buffer->get().destroy();
            }
        };
    }
}