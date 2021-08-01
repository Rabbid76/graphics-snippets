#pragma once


#include <vk_utility_object.h>
#include <vk_utility_exception.h>
#include <vk_utility_vulkan_include.h>
#include <vk_utility_parameter_helper.h>
#include <vk_utility_physical_device.h>
#include <vk_utility_device.h>
#include <vk_utility_device_factory.h>

#include <memory>
#include <vector>
#include <set>


namespace vk_utility
{

    namespace device
    {
        /// <summary>
        /// Interface for Vulkan device (`vk::Device`) factories
        /// </summary>
        class DeviceFactoryDefault
            : public IDeviceFactory
        {
        private:

            float _queue_priority = 1.0f;
            std::set<std::string> _requested_validation_layers;
            std::vector<std::string> _requested_device_extensions;

        public:

            DeviceFactoryDefault(void) = default;
            DeviceFactoryDefault(const DeviceFactoryDefault&) = delete;

            DeviceFactoryDefault & set_requested_validation_layers(const std::set<std::string> & requested_validation_layers)
            {
                _requested_validation_layers = requested_validation_layers;
                return *this;
            }

            DeviceFactoryDefault & set_requested_device_extensions(const std::vector<std::string> & requested_device_extensions)
            {
                _requested_device_extensions = requested_device_extensions;
                return *this;
            }

            virtual Device Create(vk_utility::device::PhysicalDevicePtr physical_device) override
            {
                auto unique_queue_families = get_unique_queue_families(physical_device);
                auto queue_create_information = generate_queue_create_information(unique_queue_families);
                auto physical_device_features = get_physical_device_fatures();
                vk_utility::type::StringPointerArray device_extension_list(_requested_device_extensions);
                vk_utility::type::StringPointerArray validation_layer_list(_requested_validation_layers);

                // TODO [...] `DeviceInformation` similar `vk_utility::buffer::BufferInformation`

                vk::DeviceCreateInfo create_inforamtion(
                    vk::DeviceCreateFlags{},
                    queue_create_information,
                    validation_layer_list.list(),
                    device_extension_list.list(),
                    &physical_device_features);

                return Device::Create(physical_device, create_inforamtion);
            }

            virtual DevicePtr New(vk_utility::device::PhysicalDevicePtr physical_device)  override
            {
                return vk_utility::make_shared(Create(physical_device));
            }

        private:

            std::set<int> get_unique_queue_families(vk_utility::device::PhysicalDevicePtr physical_device_ptr) const
            {
                auto &queuw_information = physical_device_ptr->get().get_queue_information();
                if (queuw_information._graphics.empty())
                    throw vk_utility::exception::Exception("no graphics device queue!");
                if (queuw_information._surface_support.empty())
                    throw vk_utility::exception::Exception("no surface presentation support!");

                std::set<int> unique_queue_families = 
                {
                    queuw_information._graphics[0],
                    queuw_information._surface_support[0]
                };
                return unique_queue_families;
            }

            std::vector<vk::DeviceQueueCreateInfo> generate_queue_create_information(const std::set<int> & unique_queue_families) const
            {
                // Vulkan lets you assign priorities to queues to influence the scheduling of command buffer execution
                // using floating point numbers between 0.0 and 1.0. 
                // This is required even if there is only a single queue
                std::vector<vk::DeviceQueueCreateInfo> queue_create_information_container;
                for (int queueFamily : unique_queue_families)
                    queue_create_information_container.emplace_back(vk::DeviceQueueCreateFlags{}, queueFamily, 1, &_queue_priority);
                return queue_create_information_container;
            }

            vk::PhysicalDeviceFeatures get_physical_device_fatures() const
            {
                return vk::PhysicalDeviceFeatures()
                    .setSamplerAnisotropy(VK_TRUE)
                    .setSampleRateShading(VK_TRUE); // enable sample shading feature for the device
            }
        };
    }
}
