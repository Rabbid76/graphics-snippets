#pragma once


#include <vk_utility_object.h>
#include <vk_utility_instance.h>
#include <vk_utility_surface.h>
#include <vk_utility_physical_device.h>
#include <vk_utility_logging.h>
#include <vk_utility_logging_physical_device.h>


#include <memory>
#include <vector>
#include <algorithm>


namespace vk_utility
{

    namespace device
    {
        /// <summary>
        /// Abstract base class for device rating
        /// </summary>
        class PhysicalDeviceRate
        {
        public:
            virtual int rate(vk_utility::device::PhysicalDevicePtr) = 0;
        };

        using PhysicalDeviceRatePtr = std::shared_ptr<PhysicalDeviceRate>;


        /// <summary>
        /// After initializing the Vulkan library through a `vk::Instance` 
        /// we need to look for and select a graphics card in the system that supports the features we need.
        /// In fact we can select any number of graphics cards and use them simultaneously.
        /// </summary>
        class PhysicalDeviceSelctor 
        {
        private:

            PhysicalDeviceRatePtr _rater;
            bool _verbose = false;

        public:

            PhysicalDeviceSelctor()
            {}

            static PhysicalDeviceSelctor New()
            {
                return PhysicalDeviceSelctor();
            }

            PhysicalDeviceSelctor & verbose(bool verbose)
            {
                _verbose = verbose;
                return *this;
            }

            PhysicalDeviceSelctor &rater(PhysicalDeviceRatePtr rater)
            {
                _rater = rater;
                return *this;
            }

            vk_utility::device::PhysicalDevicePtr select(vk_utility::core::InstancePtr instance, vk_utility::device::SurfacePtr surface) const
            {                                                               
                auto physical_devices = vk_utility::device::PhysicalDevice::enumerate(*instance, *surface);
                return select(physical_devices);
            }

            vk_utility::device::PhysicalDevicePtr select(std::vector<vk_utility::device::PhysicalDevicePtr> &physical_devices) const
            {
                std::unordered_map<vk::PhysicalDevice, int> physical_device_rates;
                for (const auto& physical_device : physical_devices)
                {
                    physical_device_rates[physical_device->handle()] =  _rater->rate(physical_device);
                }
                std::sort(physical_devices.begin(), physical_devices.end(), [&]( auto &a, auto &b )
                {
                  return physical_device_rates[*a] > physical_device_rates[*b];
                } );

                if (_verbose)
                {
                    vk_utility::logging::Log log;
                    vk_utility::logging::LogList(log).log("devices are available", physical_devices, [&](auto physical_device)
                    {
                        vk_utility::logging::LogPhysicalDevice(log, "\t").Log(physical_device, physical_device_rates[*physical_device]);
                    });
                }

                return physical_devices[0];
            }
        };
    }
}
