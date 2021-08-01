#pragma once


#include <vk_utility_object.h>
#include <vk_utility_exception.h>
#include <vk_utility_vulkan_include.h>
#include <vk_utility_instance.h>
#include <vk_utility_surface.h>
#include <vk_utility_surface_factory.h>

#include <memory>


namespace vk_utility
{

    namespace device
    {


#if defined(VK_USE_PLATFORM_WIN32_KHR)
        namespace Windows
        {
            /// <summary>
            /// Interface for Vulkan surface (`vk::SurfaceKHR`) factory for windows OS
            ///
            /// Although the `vk::SurfaceKHR` object and its usage is platform agnostic,
            /// its creation isn't because it depends on window system details.
            /// For example, it needs the HWND and HMODULE handles on Windows.
            /// Therefore there is a platform-specific addition to the extension,
            /// which on Windows is called `VK_KHR_win32_surface`.
            ///
            /// Because a window surface is a Vulkan object,
            /// it comes with a `vk::Win32SurfaceCreateInfoKHR` structure that needs to be filled in.
            /// It has two important parameters: `hwnd` and `hinstance`. 
            /// These are the handles to the window and the process.
            /// </summary>
            class SurfaceFactory
                : public ISurfaceFactory
            {
            private:

                HWND _hwnd = NULL;
                HINSTANCE _hinstance = NULL;

            public:

                SurfaceFactory(HWND hwnd)
                    :_hwnd(hwnd)
                    , _hinstance(::GetModuleHandle(nullptr))
                {}

                virtual Surface Create(vk_utility::core::InstancePtr instance) override
                {
                    vk::Win32SurfaceCreateInfoKHR create_info = {};
                    create_info.hwnd = _hwnd;
                    create_info.hinstance = _hinstance;
                    vk::SurfaceKHR surface = instance->handle().createWin32SurfaceKHR(create_info);
                    return Surface::Create(instance, surface);
                }

                virtual SurfacePtr New(vk_utility::core::InstancePtr instance) override
                {
                    return vk_utility::make_shared(Create(instance));
                }
            };
        }
#endif

    }
}
