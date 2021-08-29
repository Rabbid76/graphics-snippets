#pragma once


#include "vk_utility_object.h"
#include "vk_utility_exception.h"
#include "vk_utility_vulkan_include.h"
#include "vk_utility_device.h"
#include "vk_utility_framebuffer.h"


namespace vk_utility
{
    namespace buffer
    {
        /// <summary>
        /// Interface for Vulkan framebuffer (`vk::Framebuffer`) factories
        /// </summary>
        class IFramebufferFactory
        {
        public:

            virtual std::vector<FramebufferPtr> New(vk::Device device) = 0;
        };
    }
}