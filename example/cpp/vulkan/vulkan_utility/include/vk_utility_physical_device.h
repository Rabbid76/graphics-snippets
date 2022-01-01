#pragma once

#include <vk_utility_object.h>
#include <vk_utility_exception.h>
#include <vk_utility_vulkan_include.h>
#include <vk_utility_instance.h>
#include <vk_utility_surface.h>

#include <algorithm>
#include <vector>
#include <set>
#include <string>
#include <bitset>
#include <memory>


namespace vk_utility
{

    namespace device
    {

        struct DeviceQueueInformation;
        struct SwapchainSupportDetails;
        class PhysicalDevice;

        using PhysicalDevicePtr = vk_utility::Ptr<vk_utility::device::PhysicalDevice>;


        /// <summary>
        /// Physical device
        /// </summary>
        class PhysicalDevice
            : public GenericObject<vk::PhysicalDevice>
        {
        private:

            mutable std::vector<vk::QueueFamilyProperties> _queue_families;
            mutable std::shared_ptr<DeviceQueueInformation> _queue_information;
            mutable std::shared_ptr<SwapchainSupportDetails> _swapchain_support;
            mutable vk::SampleCountFlagBits _msaa_samples = vk::SampleCountFlagBits::e1;

        public:

            static PhysicalDevice Create(vk::PhysicalDevice device)
            {
                return PhysicalDevice(device);
            }

            static PhysicalDevicePtr New(vk::PhysicalDevice device)
            {
                return vk_utility::make_shared(Create(device));
            }

            static auto enumerate(const vk_utility::core::Instance &instance)
            {
                auto physical_devices = instance->enumeratePhysicalDevices();
                std::vector<PhysicalDevicePtr> phyical_device_objects(physical_devices.size());
                std::transform(physical_devices.begin(), physical_devices.end(), phyical_device_objects.begin(), New);
                return phyical_device_objects;
            }

            static auto enumerate(const vk_utility::core::Instance &instance, const vk_utility::device::Surface &surface)
            {
                auto phyical_device_objects = enumerate(instance);
                for (auto phyical_device : phyical_device_objects)
                    phyical_device->get().evaluate_surface_capabilites(surface);
                return phyical_device_objects;
            }

            PhysicalDevice(void) = default;
            PhysicalDevice(const PhysicalDevice &) = default;
            PhysicalDevice &operator = (const PhysicalDevice &) = default;

            PhysicalDevice(vk::PhysicalDevice device) : GenericObject(device) {}

            PhysicalDevice &operator = (const vk::PhysicalDevice &pysical_device)
            {
                _vk_object = pysical_device;
                _queue_families.clear();
                _queue_information = nullptr;
                _swapchain_support = nullptr;
            }

            virtual void destroy(void) override
            {
                _vk_object = vk::PhysicalDevice();
            }

            vk::Device create_device(const vk::DeviceCreateInfo &create_information) const
            {
                vk::Device device = _vk_object.createDevice(create_information);

#if VK_UTILITY_DYNAMIC_LOADING == 1
                VULKAN_HPP_DEFAULT_DISPATCHER.init(device);
#endif

                return device;
            }

            const auto &get_queue_family_properties(void) const
            {
                if (_vk_object)
                {
                    if (_queue_families.empty())
                        _queue_families = _vk_object.getQueueFamilyProperties();
                }
                else
                {
                    _queue_families.clear();
                }
                return _queue_families;
            }

            const auto &get_queue_information(void) const
            {
                return *_queue_information;
            }

            const auto get_queue_information_ptr(void) const
            {
                return _queue_information;
            }

            const auto &get_swapchain_support(void) const
            {
                return *_swapchain_support;
            }

            const PhysicalDevice &evaluate_surface_capabilites(const Surface &surface) const
            {
                return evaluate_queue_information(surface)
                    .evaluate_swapchain_support(surface);
            }

            const PhysicalDevice &evaluate_queue_information(const Surface &surface) const
            {
                _queue_information = std::make_unique<DeviceQueueInformation>(*this, surface);
                return *this;
            }

            const PhysicalDevice &evaluate_swapchain_support(const Surface &surface) const
            {
                _swapchain_support = std::make_unique<SwapchainSupportDetails>(*this, surface);
                return *this;
            }

            const auto get_surface_support(int queue_family_index, const Surface &surface) const
            {
                return _vk_object.getSurfaceSupportKHR(queue_family_index, surface);
            }

            const auto get_surface_capabilities(const Surface &surface) const
            {
                return _vk_object.getSurfaceCapabilitiesKHR(surface);
            }

            const auto get_surface_formats(const Surface &surface) const
            {
                return _vk_object.getSurfaceFormatsKHR(surface);
            }

            const auto get_surface_present_modes(const Surface &surface) const
            {
                return _vk_object.getSurfacePresentModesKHR(surface);
            }

            const auto enumerate_device_extension_properties(void) const
            {
                return _vk_object.enumerateDeviceExtensionProperties();
            }

            const auto get_memory_properties(void) const
            {
                return _vk_object.getMemoryProperties();
            }

            vk::SampleCountFlagBits get_max_usable_sample_count(void) const
            {
                if (_msaa_samples == vk::SampleCountFlagBits::e1)
                {
                    vk::PhysicalDeviceProperties physicalDeviceProperties = _vk_object.getProperties();
                    vk::SampleCountFlags counts = physicalDeviceProperties.limits.framebufferColorSampleCounts & physicalDeviceProperties.limits.framebufferDepthSampleCounts;
                    _msaa_samples = vk::SampleCountFlagBits::e1;
                    if (counts & vk::SampleCountFlagBits::e64) { _msaa_samples = vk::SampleCountFlagBits::e64; }
                    else if (counts & vk::SampleCountFlagBits::e32) { _msaa_samples = vk::SampleCountFlagBits::e32; }
                    else if (counts & vk::SampleCountFlagBits::e16) { _msaa_samples = vk::SampleCountFlagBits::e16; }
                    else if (counts & vk::SampleCountFlagBits::e8) { _msaa_samples = vk::SampleCountFlagBits::e8; }
                    else if (counts & vk::SampleCountFlagBits::e4) { _msaa_samples = vk::SampleCountFlagBits::e4; }
                    else if (counts & vk::SampleCountFlagBits::e2) { _msaa_samples = vk::SampleCountFlagBits::e2; }
                }
                return _msaa_samples;
            }

            /// <summary>
            /// Graphics cards can offer different types of memory to allocate from.
            /// Each type of memory varies in terms of allowed operations and performance characteristics.
            /// We need to combine the requirements of the buffer and our own application requirements to find the right type of memory to use.
            ///
            /// The `typeFilter` parameter will be used to specify the bit field of memory types that are suitable.
            /// That means that we can find the index of a suitable memory type by simply iterating over them and checking if the corresponding bit is set to 1.
            /// However, we're not just interested in a memory type that is suitable for the vertex buffer.
            /// We also need to be able to write our vertex data to that memory.
            /// The memoryTypes array consists of `vk::MemoryType` `structs` that specify the heap and properties of each type of memory.
            /// The properties define special features of the memory, like being able to map it so we can write to it from the CPU.
            /// This property is indicated with `VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT`, but we also need to use the `VK_MEMORY_PROPERTY_HOST_COHERENT_BIT` property.
            /// </summary>
            /// <param name="type_filter"></param>
            /// <param name="properties"></param>
            /// <returns></returns>
            uint32_t find_memory_type(uint32_t type_filter, vk::MemoryPropertyFlags properties) const
            {
                auto memProperties = get_memory_properties();
                for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
                {
                    if ((type_filter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
                        return i;
                }
                vk_utility::exception::Exception("failed to find suitable memory type!");
                return 0;
            }
        };


        /// <summary>
        /// Indices of device queue families with certain features.
        /// </summary>
        struct DeviceQueueInformation
        {
            std::vector<int> _graphics;        // indices of queue families with `VK_QUEUE_GRAPHICS_BIT` set
            std::vector<int> _compute;         // indices of queue families with `VK_QUEUE_COMPUTE_BIT` set
            std::vector<int> _transfer;        // indices of queue families with `VK_QUEUE_TRANSFER_BIT` set
            std::vector<int> _sparse_binding;  // indices of queue families with `VK_QUEUE_SPARSE_BINDING_BIT` set
            std::vector<int> _surface_support; // indices of queue families with surface support, evaluated by `vkGetPhysicalDeviceSurfaceSupportKHR`

            DeviceQueueInformation(void) = default;
            DeviceQueueInformation(const DeviceQueueInformation &) = default;
            DeviceQueueInformation(const PhysicalDevice &physical_device, const Surface &surface);

            DeviceQueueInformation &evaluate_flags(const PhysicalDevice &physical_device);
            DeviceQueueInformation &evaluate_surface_support(const PhysicalDevice &physical_device, const Surface &surface);
        };


        /// <summary>
        /// Construct indices of device queue families.
        /// </summary>
        /// <param name="physical_device">`VkPhysicalDevice` handle</param>
        /// <param name="surface">`VkSurfaceKHR` handle</param>
        DeviceQueueInformation::DeviceQueueInformation(
            const PhysicalDevice &physical_device,
            const Surface &surface)
        {
            // In Vulkan, anything from drawing to uploading textures, requires commands to be submitted to a queue.
            // There are different types of queues that originate from different queue families and each family of queues allows only a subset of commands.
            // For example, there could be a queue family that only allows processing of compute commands or one that only allows memory transfer related commands.
            // We need to check which queue families are supported by the device and which one of these supports the commands that we want to use. 

            evaluate_flags(physical_device);
            evaluate_surface_support(physical_device, surface);
        }


        /// <summary>
        /// Evaluate indices of device queue families with certain properties.
        /// </summary>
        /// <param name="physical_device">`VkPhysicalDevice` handle</param>
        /// <returns>`this*`</returns>
        DeviceQueueInformation &DeviceQueueInformation::evaluate_flags(
            const PhysicalDevice &physical_device)
        {
            _graphics.clear();
            _compute.clear();
            _transfer.clear();
            _sparse_binding.clear();

            if (!*physical_device)
                return *this;

            auto &queue_families = physical_device.get_queue_family_properties();

            for (int queue_family_index = 0; queue_family_index < (int)queue_families.size(); ++ queue_family_index) {
                const auto &queueFamily = queue_families[queue_family_index];
                if (queueFamily.queueCount == 0)
                    continue;

                if (queueFamily.queueFlags & vk::QueueFlagBits::eGraphics)      _graphics.push_back(queue_family_index);
                if (queueFamily.queueFlags & vk::QueueFlagBits::eCompute)       _compute.push_back(queue_family_index);
                if (queueFamily.queueFlags & vk::QueueFlagBits::eTransfer)      _transfer.push_back(queue_family_index);
                if (queueFamily.queueFlags & vk::QueueFlagBits::eSparseBinding) _sparse_binding.push_back(queue_family_index);
            }

            return *this;
        }


        /// <summary>
        ///  Evaluate indices of device queue families with surface support.
        /// </summary>
        /// <param name="physical_device">`VkPhysicalDevice` handle</param>
        /// <param name="surface">`VkSurfaceKHR` handle</param>
        /// <returns>`this*`</returns>
        DeviceQueueInformation &DeviceQueueInformation::evaluate_surface_support(
            const PhysicalDevice &physical_device,
            const Surface &surface)
        {
            _surface_support.clear();

            if (!*physical_device || !*surface)
                return *this;

            auto &queue_families = physical_device.get_queue_family_properties();

            for (int queue_family_index = 0; queue_family_index < (int)queue_families.size(); ++ queue_family_index) {
                const auto &queueFamily = queue_families[queue_family_index];
                if (queueFamily.queueCount == 0)
                    continue;

                // The function to check for that is vkGetPhysicalDeviceSurfaceSupportKHR, which takes the physical device,
                // queue family index and surface as parameters.
                if (physical_device.get_surface_support(queue_family_index, surface))
                    _surface_support.push_back(queue_family_index);
            }

            return *this;
        }


        /// <summary>
        /// Swap chain capabilities, formats and presentation modes.
        /// </summary>
        struct SwapchainSupportDetails
        {
            vk::SurfaceCapabilitiesKHR        _capabilities;
            std::vector<vk::SurfaceFormatKHR> _formats;
            std::vector<vk::PresentModeKHR>   _presentModes;

            SwapchainSupportDetails(void) = default;
            SwapchainSupportDetails(const SwapchainSupportDetails &) = default;
            SwapchainSupportDetails(const PhysicalDevice &physical_device, const Surface &surface);

            SwapchainSupportDetails &evaluate_swapchain_support(const PhysicalDevice &physical_device, const Surface &surface);
        };


        /// <summary>
        /// Constructs swapchain support details dependent on physical device and surface.    
        /// </summary>
        /// <param name="physical_device">`VkPhysicalDevice` handle</param>
        /// <param name="surface">`VkSurfaceKHR` handle</param>
        SwapchainSupportDetails::SwapchainSupportDetails(
            const PhysicalDevice &physical_device,
            const Surface &surface)
        {
            evaluate_swapchain_support(physical_device, surface);
        }


        /// <summary>
        /// Set swapchain support details attributes.
        /// </summary>
        /// <param name="physical_device">`VkPhysicalDevice` handle</param>
        /// <param name="surface">`VkSurfaceKHR` handle</param>
        /// <returns>`this*`</returns>
        SwapchainSupportDetails &SwapchainSupportDetails::evaluate_swapchain_support(
            const PhysicalDevice &physical_device,
            const Surface &surface)
        {
            _formats.clear();
            _presentModes.clear();

            if (!*physical_device || !*surface)
                return *this;

            _capabilities = physical_device.get_surface_capabilities(surface);
            _formats = physical_device.get_surface_formats(surface);
            _presentModes = physical_device.get_surface_present_modes(surface);

            return *this;
        }
    }
}
