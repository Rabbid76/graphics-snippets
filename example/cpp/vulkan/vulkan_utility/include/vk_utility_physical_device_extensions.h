#pragma once

#include <vk_utility_exception.h>
#include <vk_utility_vulkan_include.h>
#include <vk_utility_physical_device.h>


#include <string>
#include <vector>
#include <algorithm>


namespace vk_utility
{

    namespace device
    {

        class PhysicalDeviceExtensions
        {
        private:

            vk_utility::device::PhysicalDevicePtr _physical_device;
            mutable std::vector<vk::ExtensionProperties> _extensions;
            mutable std::vector<std::string> _extension_names;

        public:

            PhysicalDeviceExtensions(void) = default;
            PhysicalDeviceExtensions(const PhysicalDeviceExtensions &) = default;
            PhysicalDeviceExtensions &operator = (const PhysicalDeviceExtensions &) = default;

            PhysicalDeviceExtensions(const vk_utility::device::PhysicalDevicePtr &physical_device)
                : _physical_device(physical_device)
            {}

            auto &set_physical_device(vk_utility::device::PhysicalDevicePtr &physical_device)
            {
                _physical_device = physical_device;
                return *this;
            }

            auto &get(void) const
            {
                if (_physical_device && _extensions.empty())
                    _extensions = _physical_device->get().enumerate_device_extension_properties();
                return _extensions;
            }

            auto &get_names(void) const
            {
                if (_extension_names.empty())
                {
                    auto &extensions = get();
                    std::transform(extensions.begin(), extensions.end(), std::back_inserter(_extension_names), [](auto &extension) -> std::string {
                        return extension.extensionName.data();
                    });
                }
                return _extension_names;
            }
        };
    }
}