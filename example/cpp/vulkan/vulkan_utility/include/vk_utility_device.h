#pragma once


#include <vk_utility_object.h>
#include <vk_utility_exception.h>
#include <vk_utility_vulkan_include.h>
#include <vk_utility_physical_device.h>

#include <memory>


namespace vk_utility
{

    namespace device
    {
        class Device;
        using DevicePtr = vk_utility::Ptr<Device>;


        /// <summary>
        /// Device
        /// </summary>
        class Device
            : public GenericObject<vk::Device>
        {
        private:

            vk_utility::device::PhysicalDevicePtr _physical_device;

        public:

            static Device Create(vk_utility::device::PhysicalDevicePtr physical_device, const vk::DeviceCreateInfo &create_information)
            {
                return Device(physical_device, create_information);
            }

            static DevicePtr New(vk_utility::device::PhysicalDevicePtr physical_device, const vk::DeviceCreateInfo &create_information)
            {
                return vk_utility::make_shared(Create(physical_device, create_information));
            }

            Device(void) = default;
            Device(const Device &) = default;
            Device &operator = (const Device &) = default;

            Device(vk::Device device)
                : GenericObject(device)
            {}

            Device(vk_utility::device::PhysicalDevicePtr physical_device, const vk::DeviceCreateInfo &create_information)
                : GenericObject(physical_device->get().create_device(create_information))
                , _physical_device(physical_device)
            {}

            virtual void destroy(void) override
            {
                if (_vk_object)
                {
                    _vk_object.destroy();
                    _vk_object = vk::Device();
                }
            }

            auto physical_divice(void)
            {
                return _physical_device;
            }

            const auto &get_queue_information(void) const
            {
                return _physical_device->get().get_queue_information();
            }

            const auto &get_swapchain_support(void) const
            {
                return _physical_device->get().get_swapchain_support();
            }

            vk_utility::device::PhysicalDevice & physical_device(void) { return _physical_device->get(); }
            const vk_utility::device::PhysicalDevice & physical_device(void) const { return _physical_device->get(); }

            auto allocate_memory(const vk::MemoryAllocateInfo &allocate_information) const
            {
                auto device_memory = _vk_object.allocateMemory(allocate_information);
                return device_memory;
            }

            void free_memory(const vk::DeviceMemory &device_memory) const
            {
                _vk_object.freeMemory(device_memory);
            }

            auto create_buffer(const vk::BufferCreateInfo & buffer_information) const
            {
                return _vk_object.createBuffer(buffer_information);
            }

            auto destroy_buffer(vk::Buffer buffer) const
            {
                return _vk_object.destroyBuffer(buffer);
            }

            auto get_buffer_memory_requirements(vk::Buffer buffer) const
            {
                return _vk_object.getBufferMemoryRequirements(buffer);
            }

            auto get_graphics_queue(int family_index, int queue_index) const
            {
                return _vk_object.getQueue(family_index, queue_index);
            }

            auto get_first_graphics_queue() const
            {
                return get_graphics_queue(get_queue_information()._graphics[0], 0);
            }

            auto create_pender_pass(vk::RenderPassCreateInfo render_pass_infomration)
            {
                return _vk_object.createRenderPass(render_pass_infomration);
            }

            void destroy_render_pass(vk::RenderPass render_pass) const
            {
                _vk_object.destroyRenderPass(render_pass);
            }

            auto create_descriptor_set_layout(vk::DescriptorSetLayoutCreateInfo descriptor_set_layout_infomrationn)
            {
                return _vk_object.createDescriptorSetLayout(descriptor_set_layout_infomrationn);
            }

            void destroy_descriptor_set_layout(vk::DescriptorSetLayout descriptor_set_layout) const
            {
                _vk_object.destroyDescriptorSetLayout(descriptor_set_layout);
            }

            auto create_swapchain(vk::SwapchainCreateInfoKHR swapchain_information) const
            {
                 return _vk_object.createSwapchainKHR(swapchain_information);
            }

            void destroy_swapchain(vk::SwapchainKHR swapchain) const
            {
                _vk_object.destroySwapchainKHR(swapchain);
            }

            auto get_swapchain_images(vk::SwapchainKHR swapchain) const
            {
                return _vk_object.getSwapchainImagesKHR(swapchain);
            }

            auto create_shader_module(const vk::ShaderModuleCreateInfo& create_information) const
            {
                return _vk_object.createShaderModule(create_information);
            }

            void destroy_shader_module(vk::ShaderModule shader_module) const
            {
                _vk_object.destroyShaderModule(shader_module);
            }

            auto create_graphics_pipeline(vk::PipelineCache pipeline_cache, vk::GraphicsPipelineCreateInfo create_information) const
            {
                return _vk_object.createGraphicsPipeline(pipeline_cache, create_information);
            }

            void destroy_pipeline(vk::Pipeline pipeline) const
            {
                _vk_object.destroyPipeline(pipeline);
            }

            auto create_graphics_pipeline_layout(vk::PipelineLayoutCreateInfo create_information) const
            {
                return _vk_object.createPipelineLayout(create_information);
            }

            void destroy_pipeline_layout(vk::PipelineLayout pipeline) const
            {
                _vk_object.destroyPipelineLayout(pipeline);
            }

            auto create_image(vk::ImageCreateInfo create_information) const
            {
                return _vk_object.createImage(create_information);
            }

            void destroy_image(vk::Image image) const
            {
                _vk_object.destroyImage(image);
            }

            auto create_image_view(vk::ImageViewCreateInfo create_information) const
            {
                return _vk_object.createImageView(create_information);
            }

            void destroy_image_view(vk::ImageView image_view) const
            {
                _vk_object.destroyImageView(image_view);
            }
        };
    }
}
