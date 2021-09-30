#pragma once

#include "vk_utility_object.h"
#include "vk_utility_exception.h"
#include "vk_utility_instance_factory.h"
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

            static Instance Create(InstanceFactory &instance_factory)
            {
                return Instance(instance_factory.New());
            }

            static InstancePtr New(InstanceFactory& instance_factory)
            {
                return vk_utility::make_shared(Create(instance_factory));
            }

            Instance(void) = default;
            Instance(const Instance &) = default;
            Instance &operator = (const Instance &) = default;

            Instance(vk::Instance device)
                : GenericObject(device)
            {
#if VK_UTILITY_DYNAMIC_LOADING == 1
                VULKAN_HPP_DEFAULT_DISPATCHER.init(_vk_object);
#endif
            }

            virtual void destroy(void) override
            {
                if (_vk_object)
                {
                    _vk_object.destroy();
                    _vk_object = vk::Instance();
                }
            }
        };
    }
}
