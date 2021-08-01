#include <vk_utility_object.h>
#include <vk_utility_exception.h>
#include <vk_utility_vulkan_include.h>
#include <vk_utility_instance.h>

#include <memory>
#include <iostream>
#include <string>


namespace vk_utility
{

    namespace core
    {
        class Debug;
        using DebugPtr = vk_utility::Ptr<Debug>;


        /// <summary>
        /// Device
        /// </summary>
        class Debug
            : public GenericObject<vk::DebugReportCallbackEXT>
        {
        private:

            vk_utility::core::InstancePtr _instance;

        public:

            static Debug Create(const vk_utility::core::InstancePtr &instance, vk::DebugReportFlagsEXT flags)
            {
                return Debug(instance, flags);
            }

            static DebugPtr New(const vk_utility::core::InstancePtr &instance, vk::DebugReportFlagsEXT flags)
            {
                return vk_utility::make_shared(Create(instance, flags));
            }

            static Debug Create(const vk_utility::core::InstancePtr &instance, bool enable_debug_information, bool enable_validation_warnings)
            {
                vk::DebugReportFlagsEXT flags;
                if (enable_debug_information)
                    flags |= vk::DebugReportFlagBitsEXT::eInformation;
                if (enable_debug_information)
                    flags |= vk::DebugReportFlagBitsEXT::eDebug;
                if (enable_validation_warnings)
                    flags |= vk::DebugReportFlagBitsEXT::eWarning;
                if (enable_validation_warnings)
                    flags |= vk::DebugReportFlagBitsEXT::ePerformanceWarning;
                flags |= vk::DebugReportFlagBitsEXT::eError;

                return Debug(instance, flags);
            }

            static DebugPtr New(const vk_utility::core::InstancePtr &instance, bool enable_debug_information, bool enable_validation_warnings)
            {
                return vk_utility::make_shared(Create(instance, enable_debug_information, enable_validation_warnings));
            }

            Debug(void) = default;
            Debug(const Debug &) = default;
            Debug &operator = (const Debug &) = default;

            Debug(const vk_utility::core::InstancePtr &instance, vk::DebugReportFlagsEXT flags)
                : _instance(instance)
            {
                vk::DebugReportCallbackCreateInfoEXT create_info = {};
                create_info.setFlags(flags);
                create_info.setPfnCallback(&DebugCallback);
                create_info.pUserData = this;

                _instance->get()->createDebugReportCallbackEXT(&create_info, nullptr, &_vk_object);
            }

            virtual void destroy(void) override
            {
                if (_vk_object)
                {
                    if (_instance)
                        _instance->get()->destroyDebugReportCallbackEXT(_vk_object);
                    _vk_object = vk::DebugReportCallbackEXT();
                    _instance = nullptr;
                }
            }

            static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
                VkDebugReportFlagsEXT        flags,
                VkDebugReportObjectTypeEXT   objType,
                uint64_t                     obj,
                size_t                       location,
                int32_t                      code,
                const char *layerPrefix,
                const char *msg,
                void *userData)
            {
                std::cerr << "validation layer: " << msg << std::endl << std::endl;
                return VK_FALSE;

                //const Debug &debug = reinterpret_cast<const Debug&>(userData);
                //auto return_code  = debug.DebugCallBack(flags, objType, obj, location, code, layerPrefix, msg);
                //return return_code;
            }

            virtual VkBool32 DebugCallBack(
                VkDebugReportFlagsEXT        flags,
                VkDebugReportObjectTypeEXT   objType,
                uint64_t                     obj,
                size_t                       location,
                int32_t                      code,
                const std::string &layerPrefix,
                const std::string &msg
            ) const
            {
                std::cerr << "validation layer: " << msg << std::endl << std::endl;
                return VK_FALSE;
            }
        };
    }
}


