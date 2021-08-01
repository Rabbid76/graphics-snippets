#pragma once

#include <vk_utility_object.h>
#include <vk_utility_exception.h>
#include <vk_utility_vulkan_include.h>
#include <vk_utility_shader_module.h>

namespace vk_utility
{

    namespace shader
    {
        /// <summary>
        /// Pipeline Shader Stage CreateInformation (`vk::PipelineShaderStageCreateInfo`)
        /// 
        /// The `vk::ShaderModule` object is just a dumb wrapper around the bytecode buffer.
        /// The shaders aren't linked to each other yet and they haven't even been given a purpose yet.
        /// Assigning a shader module to either the vertex or fragment shader stage in the pipeline
        /// happens through a `vk::PipelineShaderStageCreateInfo` structure,
        /// which is part of the actual pipeline creation process.
        /// </summary>
        class PipelineShaderStageCreateInformation
        {
        private:

            vk::PipelineShaderStageCreateInfo _create_information;

        public:

            static PipelineShaderStageCreateInformation New(vk::ShaderStageFlagBits type, const vk_utility::shader::ShaderModule &module)
            {
                // There is one more (optional) member, pSpecializationInfo,
                // which we won't be using here, but is worth discussing.
                // It allows you to specify values for shader constants.
                // You can use a single shader module where its behavior can be configured
                // at pipeline creation by specifying different values for the constants used in it.
                // This is more efficient than configuring the shader using variables at render time,
                // because the compiler can do optimizations like eliminating if statements that depend on these values.
                // If you don't have any constants like that, then you can set the member to nullptr,
                // which our `struct` initialization does automatically.

                vk::PipelineShaderStageCreateInfo shader_stage_information
                (
                    vk::PipelineShaderStageCreateFlags{},
                    type,
                    module,
                    "main",
                    nullptr
                );
                return PipelineShaderStageCreateInformation(shader_stage_information);
            }

            PipelineShaderStageCreateInformation(void) = default;
            PipelineShaderStageCreateInformation(const PipelineShaderStageCreateInformation&) = default;
            PipelineShaderStageCreateInformation& operator = (const PipelineShaderStageCreateInformation&) = default;

            PipelineShaderStageCreateInformation(const vk::PipelineShaderStageCreateInfo& create_information)
                : _create_information(create_information)
            {}

            PipelineShaderStageCreateInformation& operator = (const vk::PipelineShaderStageCreateInfo& create_information)
            {
                _create_information = create_information;
                return *this;
            }

            operator const vk::PipelineShaderStageCreateInfo& () const
            {
                return _create_information;
            }
        };
    }
}