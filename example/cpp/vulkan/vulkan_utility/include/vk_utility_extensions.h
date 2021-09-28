#pragma once

#include "vk_utility_exception.h"
#include "vk_utility_vulkan_include.h"
#include <string>
#include <vector>
#include <algorithm>
#include <set>

namespace vk_utility
{

    namespace core
    {
        /// <summary>
        /// Vulkan extensions
        ///
        /// To retrieve a list of supported extensions before creating an instance, there's the vkEnumerateInstanceExtensionProperties function.
        /// It takes a pointer to a variable that stores the number of extensions and an array of `vk::ExtensionProperties` to store details of the extensions.
        /// It also takes an optional first parameter that allows us to filter extensions by a specific validation layer.
        /// </summary>
        class Extensions
        {
        private:

            std::string _validation_layer;
            mutable std::vector<vk::ExtensionProperties> _extensions;
            mutable std::vector<std::string> _extension_names;

        public:

            Extensions(void) = default;
            Extensions(const Extensions &) = default;
            Extensions &operator = (const Extensions &) = default;

            Extensions(const std::string &validation_layer)
                : _validation_layer(validation_layer)
            {}

            auto &get(void) const
            {
                if (_extensions.empty())
                    _extensions = vk::enumerateInstanceExtensionProperties(_validation_layer);
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

            bool supported(const std::string name) const
            {
                auto &extensions = get_names();
                return std::find(extensions.begin(), extensions.end(), name) != extensions.end();
            }

            auto select(const std::vector<std::string> &include)
            {
                std::set<std::string> selected_names;
                for (const auto &extension_name : get_names())
                {
                    if (std::find(include.begin(), include.end(), extension_name) != include.end())
                    {
                        selected_names.insert(extension_name);
                    }
                }
                return selected_names;
            }
        };
    }
}
