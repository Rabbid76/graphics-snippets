#pragma once


#include <vk_utility_object.h>
#include <vk_utility_exception.h>
#include <vk_utility_vulkan_include.h>

#include <memory>


namespace vk_utility
{

    namespace core
    {
        class Instance;
        using InstancePtr = vk_utility::Ptr<Instance>;


        /// <summary>
        /// Device
        /// </summary>
        class Instance
            : public GenericObject<vk::Instance>
        {
        public:

            template <typename T_VALIDAITON_LAYERS, typename T_EXTENSIONS>
            static Instance Create(const vk::ApplicationInfo &application_info, const T_VALIDAITON_LAYERS &validation_layers, const T_EXTENSIONS &extensions)
            {
                vk_utility::type::StringPointerArray validation_layer_list(validation_layers);
                vk_utility::type::StringPointerArray extension_list(extensions);
                
                vk::InstanceCreateInfo create_info(
                    {},
                    &application_info,
                    validation_layer_list.list(),
                    extension_list.list());

                return Instance(create_info);
            }

            template <typename T_VALIDAITON_LAYERS, typename T_EXTENSIONS>
            static InstancePtr New(const vk::ApplicationInfo &application_info, const T_VALIDAITON_LAYERS &validation_layers, const T_EXTENSIONS &extensions)
            {
                return vk_utility::make_shared(Create(application_info, validation_layers, extensions));
            }

            static Instance Create(vk::InstanceCreateInfo create_info)
            {
                return Instance(create_info);
            }

            static InstancePtr New(vk::InstanceCreateInfo create_info)
            {
                return vk_utility::make_shared(Create(create_info));
            }

            Instance(void) = default;
            Instance(const Instance &) = default;
            Instance &operator = (const Instance &) = default;

            Instance(vk::Instance device)
                : GenericObject(device)
            {}

            Instance(vk::InstanceCreateInfo createInfo)
                : GenericObject(vk::createInstance(createInfo))
            {
#if VK_UTILITY_DYNAMIC_LOADING == 1
                VULKAN_HPP_DEFAULT_DISPATCHER.init(_vk_object);
#endif
            }

            virtual void destroy(void) override
            {
                if (_vk_object)
                {
                    // The `VkInstance` should be only destroyed right before the program exits. 
                    // It can be destroyed in cleanup with the `VkDestroyInstance` function.
                    _vk_object.destroy();
                    _vk_object = vk::Instance();
                }
            }
        };
    }
}
