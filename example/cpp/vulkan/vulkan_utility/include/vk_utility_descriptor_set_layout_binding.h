#pragma once


#include <vk_utility_object.h>
#include <vk_utility_exception.h>
#include <vk_utility_vulkan_include.h>


namespace vk_utility
{

    namespace core
    {
        /// <summary>
        /// descriptor set layout binding (``)
        /// </summary>
        class DescriptorSetLayoutBinding
        {
        private:

            vk::DescriptorSetLayoutBinding _layout_binding;

        public:

            static DescriptorSetLayoutBinding New(const vk::DescriptorSetLayoutBinding &layout_binding)
            {
                return DescriptorSetLayoutBinding(layout_binding);
            }

            static DescriptorSetLayoutBinding New(uint32_t binding, vk::DescriptorType descriptor_type, vk::ShaderStageFlags stage_flags)
            {
                vk::DescriptorSetLayoutBinding layout_binding(
                    binding,
                    descriptor_type,
                    1,
                    stage_flags,
                    nullptr);

                return DescriptorSetLayoutBinding(layout_binding);
            }

            DescriptorSetLayoutBinding(void) = default;
            DescriptorSetLayoutBinding(const DescriptorSetLayoutBinding &) = default;
            DescriptorSetLayoutBinding &operator = (const DescriptorSetLayoutBinding &) = default;

            DescriptorSetLayoutBinding(const vk::DescriptorSetLayoutBinding &layout_binding)
                : _layout_binding(layout_binding)
            {}

            DescriptorSetLayoutBinding &operator = (const vk::DescriptorSetLayoutBinding & layout_binding)
            {
                _layout_binding = layout_binding;
                return *this;
            }

            operator const vk::DescriptorSetLayoutBinding &() const 
            {
                return _layout_binding;
            }
        };
    }
}