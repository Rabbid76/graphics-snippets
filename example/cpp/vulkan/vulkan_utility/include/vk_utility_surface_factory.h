#pragma once


#include <vk_utility_object.h>
#include <vk_utility_exception.h>
#include <vk_utility_vulkan_include.h>
#include <vk_utility_instance.h>
#include <vk_utility_surface.h>

#include <memory>


namespace vk_utility
{

    namespace device
    {
        /// <summary>
        /// Interface for Vulkan surface (`vk::SurfaceKHR`) factories
        ///
        /// Since Vulkan is a platform agnostic API, 
        /// it can not interface directly with the window system on its own.
        /// To establish the connection between Vulkan and the window system to present results to the screen,
        /// we need to use the WSI (Window System Integration) extensions.
        ///
        /// The window surface needs to be created right after the instance creation,
        /// because it can actually influence the physical device selection.
        /// The reason we postponed this is because window surfaces are part of the larger topic
        /// of render targets and presentation for which the explanation would have cluttered the basic setup.
        /// It should also be noted that window surfaces are an entirely optional component in Vulkan,
        /// if you just need off-screen rendering.
        /// Vulkan allows you to do that without hacks like creating an invisible window (necessary for OpenGL).
        /// </summary>
        class ISurfaceFactory
        {
        public:

            virtual Surface Create(vk_utility::core::InstancePtr instance) = 0;
            virtual SurfacePtr New(vk_utility::core::InstancePtr instance) = 0;
        };
    }
}