#pragma once


#include <vk_utility_object.h>
#include <vk_utility_exception.h>
#include <vk_utility_vulkan_include.h>
#include <vk_utility_instance.h>

#include <memory>


namespace vk_utility
{

    namespace device
    {
        class Surface;
        using SurfacePtr = vk_utility::Ptr<Surface>;


        /// <summary>
        /// Device
        /// </summary>
        class Surface
            : public GenericObject<vk::SurfaceKHR>
        {
        private:

            vk_utility::core::InstancePtr _instance;

        public:

            static Surface Create(const vk_utility::core::InstancePtr &instance, vk::SurfaceKHR surface)
            {
                return Surface(instance, surface);
            }

            static SurfacePtr New(const vk_utility::core::InstancePtr &instance, vk::SurfaceKHR surface)
            {
                return vk_utility::make_shared(Create(instance, surface));
            }

            Surface(void) = default;
            Surface(const Surface &) = default;
            Surface &operator = (const Surface &) = default;

            Surface(const vk_utility::core::InstancePtr &instance, vk::SurfaceKHR surface)
                : GenericObject(surface)
                , _instance(instance)
            {}

            virtual void destroy(void) override
            {
                if (_vk_object)
                {
                    if (_instance)
                        _instance->get()->destroySurfaceKHR(_vk_object);
                    _vk_object = vk::SurfaceKHR();
                    _instance = nullptr;
                }
            }
        };
    }
}

