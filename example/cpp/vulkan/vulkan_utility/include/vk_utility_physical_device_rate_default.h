#pragma once


#include <vk_utility_object.h>
#include <vk_utility_physical_device.h>
#include <vk_utility_physical_device_extensions.h>
#include <vk_utility_physical_device_rate.h>


#include <vector>
#include <string>
#include <unordered_map>
#include <algorithm>


namespace vk_utility
{

    namespace device
    {

        class PhysicalDeviceRateDefault
            : public PhysicalDeviceRate
        {
        private:

            std::vector<std::string> _requested_extensions;
            bool _enable_swapchain_extension = true;

        public:

            PhysicalDeviceRateDefault(const std::vector<std::string> &requested_extensions, bool enable_swapchain_extension)
                : _requested_extensions(requested_extensions)
                , _enable_swapchain_extension(enable_swapchain_extension)
            {}

            static std::shared_ptr<PhysicalDeviceRateDefault> New(const std::vector<std::string> &requested_extensions, bool enable_swapchain_extension)
            {
                return std::make_unique<PhysicalDeviceRateDefault>(requested_extensions, enable_swapchain_extension);
            }

            /// <summary>
            /// Just checking if a swap chain is available is not sufficient, because it may not actually be compatible with our window surface.
            /// Creating a swap chain also involves a lot more settings than instance and device creation,
            /// so we need to query for some more details before we're able to proceed.
            /// There are basically three kinds of properties we need to check:
            /// - Basic surface capabilities (min/max number of images in swap chain, min/max width and height of images)
            /// - Surface formats (pixel format, color space)
            /// - Available presentation modes
            /// </summary>
            /// <param name="physical_device"></param>
            /// <returns></returns>
            virtual int rate(vk_utility::device::PhysicalDevicePtr physical_device) override
            {
                vk::PhysicalDeviceProperties deviceProperties = physical_device->get()->getProperties();
                vk::PhysicalDeviceFeatures deviceFeatures = physical_device->get()->getFeatures();

                int score = 0;

                static const std::unordered_map<uint32_t, int> typescore_map {
                    { (uint32_t)vk::PhysicalDeviceType::eOther,          0 },
                    { (uint32_t)vk::PhysicalDeviceType::eIntegratedGpu, 100 },
                    { (uint32_t)vk::PhysicalDeviceType::eDiscreteGpu,   1000 },
                    { (uint32_t)vk::PhysicalDeviceType::eVirtualGpu,    2 },
                    { (uint32_t)vk::PhysicalDeviceType::eCpu,           1 },
                };
                auto type_score_it = typescore_map.find( (uint32_t)deviceProperties.deviceType );
                if ( type_score_it != typescore_map.end() )
                    score += type_score_it->second;

                // check for requested device extensions
                if ( checkDeviceExtensionSupport(physical_device) == false ) {
                    return 0;
                }

                // check for swap chain support
                if (_enable_swapchain_extension)
                {
                    auto &swapchain_support = physical_device->get().get_swapchain_support();
                    if (swapchain_support._formats.empty() || swapchain_support._presentModes.empty())
                    {
                        score = 0;
                        return 0;
                    }
                }

                // check for anisotropic filer support
                if (!deviceFeatures.samplerAnisotropy) {
                    return 0;
                }

                /*
                // Maximum possible size of textures affects graphics quality
                score += deviceProperties.limits.maxImageDimension2D;

                // Application can't function without geometry shaders
                if (!deviceFeatures.geometryShader) {
                    return 0;
                }
                */

                return score;
            }

        private:

            bool checkDeviceExtensionSupport(vk_utility::device::PhysicalDevicePtr physical_device)
            {
                auto device_extensions = vk_utility::device::PhysicalDeviceExtensions(physical_device).get_names();
                std::set<std::string> extensin_names(device_extensions.begin(), device_extensions.end());
                
                for (auto &requ_ext : _requested_extensions)
                {
                    if (std::find(device_extensions.begin(), device_extensions.end(), requ_ext) == device_extensions.end() )
                        return false;
                }
                return true;
            }
        };
    }
}