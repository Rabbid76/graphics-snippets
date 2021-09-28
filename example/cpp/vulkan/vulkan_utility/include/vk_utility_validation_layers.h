#pragma once


#include <vk_utility_exception.h>
#include <vk_utility_vulkan_include.h>


#include <string>
#include <vector>
#include <set>
#include <algorithm>


namespace vk_utility
{

    namespace core
    {
        /// <summary>
        /// Validation layers
        /// </summary>
        class ValidationLayers
        {
        private:

            mutable std::vector<vk::LayerProperties> _validation_layers;
            mutable std::vector<std::string> _validation_layer_names;

        public:

            ValidationLayers(void) = default;
            ValidationLayers(const ValidationLayers &) = default;
            ValidationLayers &operator = (const ValidationLayers &) = default;

            auto &get(void) const
            {
                if (_validation_layers.empty())
                    _validation_layers = vk::enumerateInstanceLayerProperties();
                return _validation_layers;
            }

            auto &get_names(void) const
            {
                if (_validation_layer_names.empty())
                {
                    auto &validation_layers = get();
                    std::transform(validation_layers.begin(), validation_layers.end(), std::back_inserter(_validation_layer_names), [](auto &layer) -> std::string {
                        return layer.layerName.data();
                    });
                }
                return _validation_layer_names;
            }

            bool supported(const std::string name) const
            {
                auto &validation_layers = get_names();
                return std::find(validation_layers.begin(), validation_layers.end(), name) != validation_layers.end();
            }

            auto select(bool include_all, const std::vector<std::string> &include, const std::vector<std::string> &exclude)
            {
                std::set<std::string> selected_names;
                for (const auto &layer_name : get_names())
                {
                    if (std::find(exclude.begin(), exclude.end(), layer_name) == exclude.end() &&
                        (include_all || std::find(include.begin(), include.end(), layer_name) != include.end()))
                    {
                        selected_names.insert(layer_name);
                    }
                }
                return selected_names;
            }
        };
    }
}