#pragma once

#include <vk_utility_vulkan_include.h>
#include <vk_utility_physical_device.h>
#include <vk_utility_physical_device_extensions.h>
#include <vk_utility_logging.h>


#include <unordered_map>
#include <string>
#include <iostream>
#include <memory>


namespace vk_utility
{

    namespace logging
    {
        class LogPhysicalDevice
        {
        private:
            vk_utility::logging::Log &_log;
            std::string _indentation;

        public:

            LogPhysicalDevice(vk_utility::logging::Log &log, const std::string &indentation)
                : _log(log)
                , _indentation(indentation)
            {}

            LogPhysicalDevice Log(vk_utility::device::PhysicalDevicePtr phyical_device, int rate)
            {
                _log << _indentation;
                deviceInformation(*phyical_device);
                _log << "; rated: " << rate << std::endl;

                static const std::vector<std::string> familiy_names{
                    "graphics queue indices:       ",
                    "compute queue indices:        ",
                    "transfer queue indices:       ",
                    "sparse binding queue indices: ",
                    "surface support indices:      "
                };
                const auto &queue_information = phyical_device->get().get_queue_information();
                std::vector<std::vector<int>> queue_info{
                    queue_information._graphics,
                    queue_information._compute,
                    queue_information._transfer,
                    queue_information._sparse_binding,
                    queue_information._surface_support
                };
                for ( size_t i=0; i < queue_info.size(); ++i ) {
                    if ( queue_info[i].empty() )
                        continue;
                    _log << _indentation << "\t" << familiy_names[i] << queue_info[i][0];
                    for ( size_t j=1; j<queue_info[i].size(); ++j )
                        _log << ", " << queue_info[i][j];
                    _log << std::endl;
                }
                _log << std::endl;

                LogList(_log).log("physical device extensions are available", vk_utility::device::PhysicalDeviceExtensions(phyical_device).get_names());

                return *this;
            }

            void deviceInformation(vk::PhysicalDevice device)
            {
                vk::PhysicalDeviceProperties deviceProperties = device.getProperties();
                vk::PhysicalDeviceFeatures deviceFeatures = device.getFeatures();

                static const std::unordered_map<uint32_t, std::string> typename_map {
                    { (uint32_t)vk::PhysicalDeviceType::eOther,          "unknown" },
                    { (uint32_t)vk::PhysicalDeviceType::eIntegratedGpu, "integrated GPU" },
                    { (uint32_t)vk::PhysicalDeviceType::eDiscreteGpu,   "discrete GPU" },
                    { (uint32_t)vk::PhysicalDeviceType::eVirtualGpu,    "virtual GPU" },
                    { (uint32_t)vk::PhysicalDeviceType::eCpu,            "CPU" },
                };

                std::string device_name = deviceProperties.deviceName.data();
                std::string device_type = "unknown";
                auto it = typename_map.find( (uint32_t)deviceProperties.deviceType );
                if ( it != typename_map.end() )
                    device_type = it->second;

                _log << device_name << " : " << device_type;
            }
        };
    }
}
