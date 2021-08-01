#pragma once


#include <vk_utility_object.h>
#include <vk_utility_exception.h>
#include <vk_utility_vulkan_include.h>
#include <vk_utility_instance.h>
#include <vk_utility_surface.h>
#include <vk_utility_surface_factory.h>

#include <vk_glfw_utility_include.h>

#include <memory>


namespace vk_glfw_utility
{

    namespace device
    {
        /// <summary>
        /// Interface for Vulkan surface (`vk::SurfaceKHR`) factory for windows OS
        ///
        /// Although the `vk::SurfaceKHR` object and its usage is platform agnostic,
        /// its creation isn't because it depends on window system details.
        /// For example, it needs the HWND and HMODULE handles on Windows.
        /// Therefore there is a platform-specific addition to the extension,
        /// which on Windows is called `VK_KHR_win32_surface`
        /// and is also automatically included in the list from `glfwGetRequiredInstanceExtensions`. 
        /// 
        /// The glfwCreateWindowSurface function performs exactly this operation with a different implementation for each platform.
        /// </summary>
        class SurfaceFactory
            : public vk_utility::device::ISurfaceFactory
        {
        private:

            GLFWwindow *_window = nullptr;

        public:

            SurfaceFactory(GLFWwindow *window)
                :_window(window)
            {}

            virtual vk_utility::device::Surface Create(vk_utility::core::InstancePtr instance) override
            {
                VkSurfaceKHR surface;
                if (glfwCreateWindowSurface(instance->handle(), _window, nullptr, &surface) != VK_SUCCESS)
                    throw vk_utility::exception::Exception("failed to create window surface!");
                return vk_utility::device::Surface::Create(instance, surface);
            }

            virtual vk_utility::device::SurfacePtr New(vk_utility::core::InstancePtr instance)
            {
                return vk_utility::make_shared(Create(instance));
            }
        };
    }
}

