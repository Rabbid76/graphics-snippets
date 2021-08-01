#pragma once


#include <vk_utility_object.h>
#include <vk_utility_exception.h>
#include <vk_utility_vulkan_include.h>
#include <vk_utility_parameter_helper.h>
#include <vk_utility_descriptor_set_layout_binding.h>
#include <vk_utility_descriptor_set_layout.h>
#include <vk_utility_descriptor_set_layout_factory.h>

#include <memory>


namespace vk_utility
{

    namespace core
    {
        /// <summary>
        /// Interface for Vulkan redner pass (`vk::RenderPass`) factories
        /// </summary>
        class DescriptorSetLayoutFactoryDefault
            : public IDescriptorSetLayoutFactory
        {
        public:

            DescriptorSetLayoutFactoryDefault(void) = default;
            DescriptorSetLayoutFactoryDefault(const DescriptorSetLayoutFactoryDefault&) = delete;

            virtual DescriptorSetLayout Create(const vk_utility::device::DevicePtr &device) override
            {
                // TODO [...] `DescriptorSetLayoutInformation` similar `vk_utility::buffer::BufferInformation`
                
                // We need to provide details about every descriptor binding used in the shaders for pipeline creation,
                // just like we had to do for every vertex attribute and its location index.
                // We'll set up a new function to define all of this information called createDescriptorSetLayout.
                // It should be called right before pipeline creation, because we're going to need it there.

                // The first two fields specify the binding used in the shader and the type of descriptor,
                // which is a uniform buffer object.
                // It is possible for the shader variable to represent an array of uniform buffer objects,
                // and descriptorCount specifies the number of values in the array.
                // This could be used to specify a transformation for each of the bones in a skeleton for skeletal animation, for example.
                // Our MVP transformation is in a single uniform buffer object, so we're using a descriptorCount of 1.
                // We also need to specify in which shader stages the descriptor is going to be referenced.
                // The stageFlags field can be a combination of `vk::ShaderStageFlagBits` values or the value VK_SHADER_STAGE_ALL_GRAPHICS.
                // In our case, we're only referencing the descriptor from the vertex shader.

                vk::DescriptorSetLayoutBinding ubo_layout_binding
                    = DescriptorSetLayoutBinding::New(0, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eVertex);

                // add a `vk::DescriptorSetLayoutBinding` for a combined image sampler descriptor. 
                // Make sure to set the stageFlags to indicate that we intend to use the combined image sampler descriptor in the fragment shader.
                // That's where the color of the fragment is going to be determined.
                // It is possible to use texture sampling in the vertex shader,
                // for example to dynamically deform a grid of vertices by a height map.

                vk::DescriptorSetLayoutBinding sampler_layout_binding
                    = DescriptorSetLayoutBinding::New(1, vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eFragment);

                //! All of the descriptor bindings are combined into a single `vk::DescriptorSetLayout` object. 

                std::array<vk::DescriptorSetLayoutBinding, 2> bindings = {ubo_layout_binding, sampler_layout_binding};
                vk::DescriptorSetLayoutCreateInfo layout_informatin
                (
                    vk::DescriptorSetLayoutCreateFlags{},
                    bindings
                );

                return DescriptorSetLayout::Create(device, layout_informatin);
            }

            virtual DescriptorSetLayoutPtr New(const vk_utility::device::DevicePtr &device)  override
            {
                return vk_utility::make_shared(Create(device));
            }
        };
    }
}
