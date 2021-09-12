#pragma once

#include <vk_utility_object.h>
#include <vk_utility_exception.h>
#include <vk_utility_vulkan_include.h>
#include <vk_utility_device.h>
#include <vk_utility_pipeline.h>
#include <vk_utility_pipeline_layout.h>
#include <vk_utility_pipeline_factory.h>
#include <vk_utility_shader_module.h>
#include <vk_utility_shader_stage_create_information.h>
#include <vk_utility_swapchain.h>
#include <vk_utility_render_pass.h>

#include <vector>
#include <tuple>

namespace vk_utility
{

    namespace pipeline
    {
        /// <summary>
        /// Interface for Vulkan pipeline (`vk::Pipeline`) factories
        /// 
        /// Graphics pipeline simplified overview:
        /// - Vertex/index buffer
        /// - Input assembler
        /// - Vertex shader
        /// - Tessellation
        /// - Geometry shader
        /// - Rasterization
        /// - Fragment shader
        /// - Color blending
        /// - Framebuffer
        ///
        /// Input assembler, Rasterization and Color blending stage are known as fixed-function stages.
        /// These stages allow you to tweak their operations using parameters, but the way they work is predefined. 
        ///
        /// Vertex shader, Tessellation, Geometry shader and Fragment shader stage are programmable, 
        /// which means that you can upload your own code to the graphics card to apply exactly the operations you want.
        /// This allows you to use fragment shaders, for example, 
        /// to implement anything from texturing and lighting to ray tracers.
        /// These programs run on many GPU cores simultaneously to process many objects, 
        /// like vertices and fragments in parallel.
        ///
        /// The input assembler collects the raw vertex data from the buffers you specify
        /// and may also use an index buffer to repeat certain elements without having to duplicate the vertex data itself.
        ///
        /// The vertex shader is run for every vertex and generally applies transformations to 
        /// turn vertex positions from model space to screen space.
        /// It also passes per-vertex data down the pipeline.
        ///
        /// The tessellation shaders allow you to subdivide geometry based on certain rules to increase the mesh quality.
        /// This is often used to make surfaces like brick walls and staircases look less flat when they are nearby.
        ///
        /// The geometry shader is run on every primitive (triangle, line, point)
        /// and can discard it or output more primitives than came in.
        /// This is similar to the tessellation shader, but much more flexible.
        /// However, it is not used much in today's applications because the performance
        /// is not that good on most graphics cards except for Intel's integrated GPUs.
        ///
        /// The rasterization stage discretizes the primitives into fragments.
        /// These are the pixel elements that they fill on the framebuffer.
        /// Any fragments that fall outside the screen are discarded
        /// and the attributes outputted by the vertex shader are interpolated across the fragments.
        /// Usually the fragments that are behind other primitive fragments are also discarded here because of depth testing.
        ///
        /// The fragment shader is invoked for every fragment that survives
        /// and determines which framebuffer(s) the fragments are written to and with which color and depth values.
        /// It can do this using the interpolated data from the vertex shader,
        /// which can include things like texture coordinates and normals for lighting.
        ///
        /// The color blending stage applies operations to mix different fragments
        /// that map to the same pixel in the framebuffer.
        /// Fragments can simply overwrite each other, add up or be mixed based upon transparency.
        /// </summary>
        class PipelienFactoryDefault
            : public IPipelineFactory
        {
        private:

            std::vector<std::tuple<vk_utility::shader::ShaderModulePtr, vk::ShaderStageFlagBits>> _shader_modules;
            std::vector<vk::VertexInputBindingDescription> _vertex_binding_descriptions;
            std::vector<vk::VertexInputAttributeDescription> _vertex_attribute_descriptions;
            bool _enable_face_culling = true;
            bool _enable_depth_test = true;
            vk_utility::pipeline::PipelineLayoutPtr _pipeline_layout;
            vk_utility::core::RenderPassPtr _render_pass;

        public:

            virtual Pipeline Create(vk_utility::device::DevicePtr device, vk_utility::swap::SwapchainPtr swapchain)
            {
                return create_pipeline(device, swapchain);
            }

            virtual PipelinePtr New(vk_utility::device::DevicePtr device, vk_utility::swap::SwapchainPtr swapchain)
            {
                return vk_utility::make_shared(Create(device, swapchain));
            }

            PipelienFactoryDefault& add_shader_module(vk_utility::shader::ShaderModulePtr shader_module, vk::ShaderStageFlagBits shader_flags)
            {
                _shader_modules.emplace_back(shader_module, shader_flags);
                return *this;
            }

            PipelienFactoryDefault& add_vertex_binding_description(const vk::VertexInputBindingDescription &vertex_binding_description)
            {
                _vertex_binding_descriptions.push_back(vertex_binding_description);
                return *this;
            }

            PipelienFactoryDefault& add_attribute_description(const vk::VertexInputAttributeDescription &vertex_attribute_description)
            {
                _vertex_attribute_descriptions.push_back(vertex_attribute_description);
                return *this;
            }

            PipelienFactoryDefault& add_vertex_binding_descriptions(const std::vector<vk::VertexInputBindingDescription>& vertex_binding_descriptions)
            {
                for (auto & vertex_binding_description : vertex_binding_descriptions)
                    _vertex_binding_descriptions.push_back(vertex_binding_description);
                return *this;
            }

            PipelienFactoryDefault& add_attribute_descriptions(const std::vector<vk::VertexInputAttributeDescription>& vertex_attribute_descriptions)
            {
                for (auto& vertex_attribute_description : vertex_attribute_descriptions)
                    _vertex_attribute_descriptions.push_back(vertex_attribute_description);
                return *this;
            }

            PipelienFactoryDefault& enable_face_culling(bool enable_face_culling)
            {
                _enable_face_culling = enable_face_culling;
                return *this;
            }

            PipelienFactoryDefault& enable_depth_test(bool enable_depth_test)
            {
                _enable_depth_test = enable_depth_test;
                return *this;
            }

            PipelienFactoryDefault& set_pipeline_layout(vk_utility::pipeline::PipelineLayoutPtr pipeline_layout)
            {
                _pipeline_layout = pipeline_layout;
                return *this;
            }

            PipelienFactoryDefault& set_render_pass(vk_utility::core::RenderPassPtr render_pass)
            {
                _render_pass = render_pass;
                return *this;
            }

        private:

            Pipeline create_pipeline(vk_utility::device::DevicePtr device, vk_utility::swap::SwapchainPtr swapchain)
            {
                auto physical_device = device->get().physical_device_ptr();

                // The graphics pipeline in Vulkan is almost completely immutable,
                // so you must recreate the pipeline from scratch if you want to change shaders,
                // bind different framebuffers or change the blend function.
                // The disadvantage is that you'll have to create a number of pipelines
                // that represent all of the different combinations of states
                // you want to use in your rendering operations.
                // However, because all of the operations you'll be doing in the pipeline are known in advance,
                // the driver can optimize for it much better.

                // The older graphics APIs provided default state for most of the stages of the graphics pipeline.
                // In Vulkan you have to be explicit about everything, from viewport size to color blending function.
                
                // You can use uniform values in shaders, which are globals similar to dynamic state variables
                // that can be changed at drawing time to alter the behavior of your shaders
                // without having to recreate them.
                // They are commonly used to pass the transformation matrix to the vertex shader,
                // or to create texture samplers in the fragment shader.
                // These uniform values need to be specified during pipeline creation by creating
                // a `vk::PipelineLayout` object.
                // Even though we won't be using them until a future chapter,
                // we are still required to create an empty pipeline layout.
                
                // We need to specify the descriptor set layout during pipeline creation
                // to tell Vulkan which descriptors the shaders will be using.
                // Descriptor set layouts are specified in the pipeline layout object. 
                // Modify the `vk::PipelineLayoutCreateInfo` to reference the layout object. 

                std::vector<vk::PipelineShaderStageCreateInfo> shader_stages;
                for (auto& shader_module : _shader_modules)
                {
                    shader_stages.push_back(
                        vk_utility::shader::PipelineShaderStageCreateInformation::New(std::get<1>(shader_module), *std::get<0>(shader_module)));
                }

                // The `vk::PipelineVertexInputStateCreateInfo` structure describes the format of the vertex data that will be passed to the vertex shader.
                // It describes this in roughly two ways:
                // - Bindings: spacing between data and whether the data is per-vertex or per-instance (see instancing)
                // - Attribute descriptions: type of the attributes passed to the vertex shader, which binding to load them from and at which offset
                
                // The `pVertexBindingDescriptions` and `pVertexAttributeDescriptions` members point to an array of `struct`s that describe the aforementioned details for loading vertex data.

                vk::PipelineVertexInputStateCreateInfo vertex_input_information
                (
                    vk::PipelineVertexInputStateCreateFlags{},
                    _vertex_binding_descriptions,
                    _vertex_attribute_descriptions
                );

                // The `vk::PipelineInputAssemblyStateCreateInfo` `struct` describes two things: what kind of geometry will be drawn from the vertices and if primitive restart should be enabled.
                // The former is specified in the topology member and can have values like:
                // - `VK_PRIMITIVE_TOPOLOGY_POINT_LIST`: points from vertices
                // - `VK_PRIMITIVE_TOPOLOGY_LINE_LIST`: line from every 2 vertices without reuse
                // - `VK_PRIMITIVE_TOPOLOGY_LINE_STRIP`: the end vertex of every line is used as start vertex for the next line
                // - `VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST: triangle from every 3 vertices without reuse
                // - `VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP`: the second and third vertex of every triangle are used as first two vertices of the next triangle

                // Normally, the vertices are loaded from the vertex buffer by index in sequential order, but with an element buffer you can specify the indices to use yourself.
                // This allows you to perform optimizations like reusing vertices.
                // If you set the primitiveRestartEnable member to `VK_TRUE , then it's possible to break up lines and triangles in the _STRIP topology modes by using a special index of 0xFFFF or 0xFFFFFFFF.

                vk::PipelineInputAssemblyStateCreateInfo input_assembly
                (
                    vk::PipelineInputAssemblyStateCreateFlags{},
                    vk::PrimitiveTopology::eTriangleList,
                    VK_FALSE
                );

                // A viewport basically describes the region of the framebuffer that the output will be rendered to. 
                // This will almost always be (0, 0) to (width, height) and in this tutorial that will also be the case.

                // Remember that the size of the swap chain and its images may differ from the WIDTH and HEIGHT of the window. The swap chain images will be used as framebuffers later on, so we should stick to their size.

                // The minDepth and maxDepth values specify the range of depth values to use for the framebuffer.
                // These values must be within the [0.0f, 1.0f] range, but minDepth may be higher than maxDepth.
                // If you aren't doing anything special, then you should stick to the standard values of 0.0f and 1.0f.

                // While viewports define the transformation from the image to the framebuffer, scissor rectangles define in which regions pixels will actually be stored.
                // Any pixels outside the scissor rectangles will be discarded by the rasterizer.
                // They function like a filter rather than a transformation. The difference is illustrated below.
                // Note that the left scissor rectangle is just one of the many possibilities that would result in that image, as long as it's larger than the viewport.

                vk::Viewport viewport(0.0f, 0.0f, (float)swapchain->get().image_width_2D(), (float)swapchain->get().image_height_2D(), 0.0f, 1.0f);

                vk::Rect2D scissor({ 0, 0 }, swapchain->get().image_extent_2D());

                std::vector<vk::Viewport> viewports{ viewport };
                std::vector<vk::Rect2D> scissors{ scissor };
                vk::PipelineViewportStateCreateInfo viewport_state
                (
                    vk::PipelineViewportStateCreateFlags{},
                    viewports,
                    scissors
                );

                // The rasterizer takes the geometry that is shaped by the vertices from the vertex shader and turns it into fragments to be colored by the fragment shader.
                // It also performs depth testing, face culling and the scissor test, and it can be configured to output fragments that fill entire polygons or just the edges (wireframe rendering).
                // All this is configured using the `vk::PipelineRasterizationStateCreateInfo` structure.

                // If `depthClampEnable` is set to `VK_TRUE`, then fragments that are beyond the near and far planes are clamped to them as opposed to discarding them.
                // This is useful in some special cases like shadow maps.
                // Using this requires enabling a GPU feature.

                // If `rasterizerDiscardEnable` is set to `VK_TRUE`, then geometry never passes through the rasterizer stage.
                //This basically disables any output to the framebuffer.

                // The polygonMode determines how fragments are generated for geometry.
                // The following modes are available:
                // - `VK_POLYGON_MODE_FILL`: fill the area of the polygon with fragments
                // - `VK_POLYGON_MODE_LINE`: polygon edges are drawn as lines
                // - `VK_POLYGON_MODE_POINT`: polygon vertices are drawn as points
                // Using any mode other than fill requires enabling a GPU feature.

                // The `lineWidth` member is straightforward, it describes the thickness of lines in terms of number of fragments.
                // The maximum line width that is supported depends on the hardware and any line thicker than 1.0f requires you to enable the wideLines GPU feature.

                // The `cullMode` variable determines the type of face culling to use.
                // You can disable culling, cull the front faces, cull the back faces or both.
                // The `frontFace` variable specifies the vertex order for faces to be considered front-facing and can be clockwise or counterclockwise.

                // The rasterizer can alter the depth values by adding a constant value or biasing them based on a fragment's slope.
                // This is sometimes used for shadow mapping, but we won't be using it.
                // Just set `depthBiasEnable` to `VK_FALSE`.

                vk::PipelineRasterizationStateCreateInfo rasterizer
                (
                    vk::PipelineRasterizationStateCreateFlags{},
                    VK_FALSE,
                    VK_FALSE,
                    vk::PolygonMode::eFill,
                    _enable_face_culling ? vk::CullModeFlagBits::eBack : vk::CullModeFlagBits::eNone,
                    vk::FrontFace::eCounterClockwise,
                    VK_FALSE,
                    0.0f,
                    0.0f,
                    0.0f,
                    1.0f
                );

                // The `vk::PipelineMultisampleStateCreateInfo` `struct` configures multisampling, which is one of the ways to perform anti-aliasing.
                // It works by combining the fragment shader results of multiple polygons that rasterize to the same pixel.
                // This mainly occurs along edges, which is also where the most noticeable aliasing artifacts occur.
                // Because it doesn't need to run the fragment shader multiple times if only one polygon maps to a pixel, it is significantly less expensive than simply rendering to a higher resolution and then downscaling.
                // Enabling it requires enabling a GPU feature.

                vk::PipelineMultisampleStateCreateInfo multisampling
                (
                    vk::PipelineMultisampleStateCreateFlags{},
                    physical_device->get().get_max_usable_sample_count(),
                    VK_TRUE, // enable sample shading in the pipeline
                    0.2f, // min fraction for sample shading; closer to one is smoother
                    nullptr,
                    VK_FALSE,
                    VK_FALSE
                );

                // If you are using a depth and/or stencil buffer, then you also need to configure the depth and stencil tests using `vk::PipelineDepthStencilStateCreateInfo`.

                vk::PipelineDepthStencilStateCreateInfo depth_stencil
                (
                    vk::PipelineDepthStencilStateCreateFlags{},
                    _enable_depth_test ? VK_TRUE : VK_FALSE,
                    _enable_depth_test ? VK_TRUE : VK_FALSE,
                    vk::CompareOp::eLess,
                    VK_FALSE,
                    VK_FALSE,
                    vk::StencilOpState{},
                    vk::StencilOpState{},
                    0.0f,
                    1.0f
                );

                // After a fragment shader has returned a color, it needs to be combined with the color that is already in the framebuffer.
                // This transformation is known as color blending and there are two ways to do it:
                // - Mix the old and new value to produce a final color
                // - Combine the old and new value using a bitwise operation
                // There are two types of `struct`s to configure color blending. 
                // The first `struct`, `vk::PipelineColorBlendAttachmentState` contains the configuration per attached framebuffer
                // and the second `struct`, `vk::PipelineColorBlendStateCreateInfo` contains the global color blending settings.

                // This per-framebuffer `struct` allows you to configure the first way of color blending.
                // The operations that will be performed are best demonstrated using the following pseudocode:
                //
                // if (blendEnable) {
                //     finalColor.rgb = (srcColorBlendFactor * newColor.rgb) <colorBlendOp> (dstColorBlendFactor * oldColor.rgb);
                //     finalColor.a = (srcAlphaBlendFactor * newColor.a) <alphaBlendOp> (dstAlphaBlendFactor * oldColor.a);
                // } else {
                //     finalColor = newColor;
                // }
                // finalColor = finalColor & colorWriteMask;

                // If blendEnable is set to `VK_FALSE`, then the new color from the fragment shader is passed through unmodified.
                // Otherwise, the two mixing operations are performed to compute a new color. 
                // The resulting color is AND'd with the colorWriteMask to determine which channels are actually passed through.
                //
                // The most common way to use color blending is to implement alpha blending, where we want the new color to be blended with the old color based on its opacity.
                // The finalColor should then be computed as follows:
                //
                // finalColor.rgb = newAlpha * newColor + (1 - newAlpha) * oldColor;
                // finalColor.a = newAlpha.a;

                // If you want to use the second method of blending (bitwise combination), then you should set `logicOpEnable` to `VK_TRUE`.
                // The bitwise operation can then be specified in the logicOp field.
                // Note that this will automatically disable the first method, as if you had set `blendEnable` to `VK_FALSE` for every attached framebuffer!
                // The `colorWriteMask` will also be used in this mode to determine which channels in the framebuffer will actually be affected.
                // It is also possible to disable both modes, as we've done here, in which case the fragment colors will be written to the framebuffer unmodified.

                vk::PipelineColorBlendAttachmentState color_blend_attachment
                (
                    VK_FALSE,
                    vk::BlendFactor::eOne,
                    vk::BlendFactor::eZero,
                    vk::BlendOp::eAdd,
                    vk::BlendFactor::eOne,
                    vk::BlendFactor::eZero,
                    vk::BlendOp::eAdd,
                    vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA
                );

                std::array<float, 4> blend_constants{ 0.0f, 0.0f, 0.0f, 0.0f };
                std::vector<vk::PipelineColorBlendAttachmentState> color_blend_attachments{ color_blend_attachment };
                vk::PipelineColorBlendStateCreateInfo color_blending
                (
                    vk::PipelineColorBlendStateCreateFlags{},
                    VK_FALSE,
                    vk::LogicOp::eCopy,
                    color_blend_attachments,
                    blend_constants
                );

                // A limited amount of the state that we've specified in the previous `struct`s can actually be changed without recreating the pipeline.
                // Examples are the size of the viewport, line width and blend constants.
                // If you want to do that, then you'll have to fill in a `vk::PipelineDynamicStateCreateInfo` structure like this:

                std::vector<vk::DynamicState> dynamic_states{ vk::DynamicState::eViewport, vk::DynamicState::eLineWidth };
                vk::PipelineDynamicStateCreateInfo dynamic_state
                (
                    vk::PipelineDynamicStateCreateFlags{},
                    dynamic_states
                );

                // And finally we have the reference to the render pass and the index of the sub pass where this graphics pipeline will be used.
                // It is also possible to use other render passes with this pipeline instead of this specific instance,
                // but they have to be compatible with renderPass.

                // There are actually two more parameters: `basePipelineHandle` and `basePipelineIndex`.
                // Vulkan allows you to create a new graphics pipeline by deriving from an existing pipeline.
                // The idea of pipeline derivatives is that it is less expensive to set up pipelines when they have much functionality in common with an existing pipeline and switching between pipelines from the same parent can also be done quicker.
                // You can either specify the handle of an existing pipeline with basePipelineHandle or reference another pipeline that is about to be created by index with basePipelineIndex.
                // Right now there is only a single pipeline, so we'll simply specify a null handle and an invalid index.
                // These values are only used if the VK_PIPELINE_CREATE_DERIVATIVE_BIT flag is also specified in the flags field of `vk::GraphicsPipelineCreateInfo`.

                vk::GraphicsPipelineCreateInfo pipeline_information
                (
                    vk::PipelineCreateFlags{},
                    shader_stages,
                    & vertex_input_information,
                    & input_assembly,
                    nullptr,
                    & viewport_state,
                    & rasterizer,
                    & multisampling,
                    & depth_stencil,
                    & color_blending,
                    nullptr,
                    _pipeline_layout->handle(),
                    _render_pass->handle(),
                    0,
                    vk::Pipeline(),
                    -1
                );

                return vk_utility::pipeline::Pipeline::Create(device, pipeline_information);
            }
        };
    }
}
