#pragma once

#include "vk_utility_fence_factory.h"

namespace vk_utility
{
    namespace core
    {
        /// <summary>
        /// There are two ways of synchronizing swap chain events: fences and semaphores.
        /// They're both objects that can be used for coordinating operations by having one operation signal
        /// and another operation wait for a fence or semaphore to go from the unsignaled to signaled state.
        ///
        /// The difference is that the state of fences can be accessed from your program using calls like `vkWaitForFences`
        /// and semaphores cannot be.
        //! Fences are mainly designed to synchronize your application itself with rendering operation,
        //! whereas semaphores are used to synchronize operations within or across command queues.
        /// </summary>
        class FenceFactoryDefault
            : public FenceFactory
        {
        public:

            virtual vk::Fence New(vk::Device device) const override
            {
                vk::FenceCreateInfo fence_information = {};
                fence_information.flags = vk::FenceCreateFlagBits::eSignaled;
                return device.createFence(fence_information);
            }
        };
    }
}
