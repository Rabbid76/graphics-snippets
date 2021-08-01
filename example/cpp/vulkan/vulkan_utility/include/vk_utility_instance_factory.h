#pragma once


#include <vk_utility_exception.h>
#include <vk_utility_vulkan_include.h>
#include <vk_utility_instance.h>
#include <vk_utility_validation_layers.h>
#include <vk_utility_extensions.h>
#include <vk_utility_logging.h>

#include <string>
#include <vector>


namespace vk_utility
{

    namespace core
    {
        /// <summary>
        /// Interface for Vulkan instance (`vk::Instance`) factories
        /// </summary>
        class IInstanceFactory
        {
        public:

            virtual IInstanceFactory &verbose(bool verbose) = 0;
            virtual IInstanceFactory &title(const std::string &title) = 0;
            virtual IInstanceFactory &validation_layers(bool include_all, const std::vector<std::string> &include, const std::vector<std::string> &exclude) = 0;
            virtual IInstanceFactory &extensions(const std::vector<std::string> &include, bool enable_debug_extension) = 0;

            virtual Instance Create(void) = 0;
            virtual InstancePtr New(void) = 0;
        };


        /// <summary>
        /// DEfault Vulkan instance (`vk::Instance`) factory
        /// </summary>
        class InstanceFactory
            : public IInstanceFactory
        {
        private:

            bool _verbose = false;
            bool _enable_all_validation_layers = false;
            bool _enable_debug_extension = false;
            std::string _title;
            std::vector<std::string> _requested_validation_layers;
            std::vector<std::string> _excluded_validation_layers;
            std::vector<std::string> _requested_extensions;
            vk_utility::core::Extensions _extensions;
            vk_utility::core::ValidationLayers _validation_layers;

        public:

            virtual InstanceFactory &verbose(bool verbose) override
            {
                _verbose = verbose;
                return *this;
            }

            virtual InstanceFactory &title(const std::string &title) override
            {
                _title = title;
                return *this;
            }

            virtual InstanceFactory &validation_layers(bool include_all, const std::vector<std::string> &include, const std::vector<std::string> &exclude) override
            {
                _enable_all_validation_layers = include_all;
                _requested_validation_layers = include;
                _excluded_validation_layers = exclude;
                return *this;
            }

            virtual InstanceFactory &extensions(const std::vector<std::string> &include, bool enable_debug_extension) override
            {
                _requested_extensions = include;
                _enable_debug_extension = enable_debug_extension;
                return *this;
            }

            virtual Instance Create(void)
            {
                auto selected_validation_layers = _validation_layers.select(_enable_all_validation_layers, _requested_validation_layers, _excluded_validation_layers);
                add_debug_extensions(selected_validation_layers);
                auto selected_extensions = _extensions.select(_requested_extensions);
                log(selected_validation_layers, selected_extensions);
                return Instance::Create(create_application_information(), selected_validation_layers, selected_extensions);
            }

            virtual InstancePtr New(void)
            {
                return vk_utility::make_shared(Create());
            }

        private:

            template <typename T_VALIDATION_LAYERS>
            InstanceFactory &add_debug_extensions(const T_VALIDATION_LAYERS &selected_validation_layers)
            {
                if (selected_validation_layers.empty())
                    return *this;

                std::vector<const char *> debug_extensions{ VK_EXT_DEBUG_REPORT_EXTENSION_NAME };
                if (_enable_debug_extension)
                    debug_extensions.push_back("VK_EXT_debug_utils");

                for (auto debug_extension : debug_extensions) {
                    if (std::find(_requested_extensions.begin(), _requested_extensions.end(), debug_extension) != _requested_extensions.end())
                        continue;
                    if (!_extensions.supported(debug_extension))
                        continue;
                    _requested_extensions.push_back(debug_extension);
                }

                return *this;
            }

            template <typename T_VALIDATION_LAYERS, typename T_EXTENSIONS>
            InstanceFactory &log(const T_VALIDATION_LAYERS &selected_validation_layers, const T_EXTENSIONS &selected_extensions)
            {
                if (!_verbose)
                    return *this;

                vk_utility::logging::Log log;

                auto is_validatio_layer_selected = [&](auto &name) -> bool { return selected_validation_layers.find(name) == selected_validation_layers.end(); };
                auto is_extension_selected = [&](auto &name) -> bool { return selected_extensions.find(name) == selected_extensions.end(); };

                vk_utility::logging::LogList log_list(log);
                log_list.log("validation layers supported", _validation_layers.get_names());
                log_list.log("extensions supported", _extensions.get_names());
                log_list.log("validation layers are set up", selected_validation_layers);
                log_list.log_if(_requested_validation_layers, "validation layer requested, but not available.", is_validatio_layer_selected);
                log_list.log("extensions are set up", selected_extensions);
                log_list.log_if(_requested_extensions, "extension requested, but not available.", is_extension_selected);

                return *this;
            }

            vk::ApplicationInfo create_application_information(void)
            {
                // To create an instance we'll first have to fill in a `struct vk::ApplicationInfo` with some information about our application.
                // This data is technically optional, but it may provide some useful information to the driver to optimize for our specific application,
                // for example because it uses a well-known graphics engine with certain special behavior.

                vk::ApplicationInfo app_info(
                    _title.c_str(), VK_MAKE_VERSION(1, 0, 0), "No Engine", VK_MAKE_VERSION(1, 0, 0), VK_API_VERSION_1_0);
                return app_info;
            }
        };
    }
}