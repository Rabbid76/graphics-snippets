#pragma once


#include <vk_utility_object.h>
#include <vk_utility_exception.h>
#include <vk_utility_vulkan_include.h>
#include <vk_utility_parameter_helper.h>
#include <vk_utility_render_pass.h>
#include <vk_utility_render_pass_factory.h>

#include <memory>


namespace vk_utility
{

    namespace core
    {
        /// <summary>
        /// Interface for Vulkan redner pass (`vk::RenderPass`) factories
        /// </summary>
        class RenderPassFactoryDefault
            : public IRenderPassFactory
        {
        private:

            vk::Format _color_format;
            vk::Format _depth_format;
            vk::SampleCountFlagBits _sampel_count;

        public:

            RenderPassFactoryDefault(void) = default;
            RenderPassFactoryDefault(const RenderPassFactoryDefault&) = delete;

            RenderPassFactoryDefault & set_color_format(vk::Format format)
            {
                _color_format = format;
                return *this;
            }

            RenderPassFactoryDefault & set_depth_format(vk::Format format)
            {
                _depth_format = format;
                return *this;
            }

            RenderPassFactoryDefault & set_sampple_count(vk::SampleCountFlagBits sampel_count)
            {
                _sampel_count = sampel_count;
                return *this;
            }

            virtual RenderPass Create(const vk_utility::device::DevicePtr &device) override
            {
                // TODO [...] `RenderPassInformation` similar `vk_utility::buffer::BufferInformation`
                // TODO [...] `AttachmentDescription` `AttachmentReference` `SubpassDescription` `SubpassDependency`

                vk::AttachmentDescription color_attachment
                (
                    vk::AttachmentDescriptionFlags{},
                    _color_format,
                    _sampel_count,
                    vk::AttachmentLoadOp::eClear,
                    vk::AttachmentStoreOp::eDontCare,
                    vk::AttachmentLoadOp::eDontCare,
                    vk::AttachmentStoreOp::eDontCare,
                    vk::ImageLayout::eUndefined, 
                    vk::ImageLayout::eColorAttachmentOptimal
                );
                vk::AttachmentReference color_attachment_reference(0, vk::ImageLayout::eColorAttachmentOptimal);
               
                vk::AttachmentDescription depth_attachment
                (
                    vk::AttachmentDescriptionFlags{},
                    _depth_format,
                    _sampel_count,
                    vk::AttachmentLoadOp::eClear,
                    vk::AttachmentStoreOp::eDontCare,
                    vk::AttachmentLoadOp::eDontCare,
                    vk::AttachmentStoreOp::eDontCare,
                    vk::ImageLayout::eUndefined, 
                    vk::ImageLayout::eDepthStencilAttachmentOptimal
                );
                vk::AttachmentReference depth_attachment_reference(1, vk::ImageLayout::eDepthStencilAttachmentOptimal);

                vk::AttachmentDescription color_attachment_resolve
                (
                    vk::AttachmentDescriptionFlags{},
                    _color_format,
                    vk::SampleCountFlagBits::e1,
                    vk::AttachmentLoadOp::eDontCare,
                    vk::AttachmentStoreOp::eStore,
                    vk::AttachmentLoadOp::eDontCare,
                    vk::AttachmentStoreOp::eDontCare,
                    vk::ImageLayout::eUndefined,
                    vk::ImageLayout::ePresentSrcKHR
                );
                vk::AttachmentReference color_attachment_resolve_reference(2, vk::ImageLayout::eColorAttachmentOptimal);


                std::vector<vk::AttachmentReference> input_attachments;
                std::vector<vk::AttachmentReference> color_attachments{color_attachment_reference};
                std::vector<vk::AttachmentReference> resolve_attachments{color_attachment_resolve_reference};
                std::vector<uint32_t> preserve_attachments;

                vk::SubpassDescription subpass
                (
                    vk::SubpassDescriptionFlags{},
                    vk::PipelineBindPoint::eGraphics, 
                    input_attachments,
                    color_attachments,
                    resolve_attachments,
                    &depth_attachment_reference,
                    preserve_attachments
                );

                // Remember that the subpasses in a render pass automatically take care of image layout transitions.
                // These transitions are controlled by subpass dependencies, which specify memory and execution dependencies between subpasses.
                // We have only a single subpass right now, but the operations right before and right after this subpass also count as implicit "subpasses".

                // There are two built-in dependencies that take care of the transition at the start of the render pass and at the end of the render pass, but the former does not occur at the right time.
                // It assumes that the transition occurs at the start of the pipeline, but we haven't acquired the image yet at that point!
                // There are two ways to deal with this problem.
                // We could change the waitStages for the imageAvailableSemaphore to `VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT` to ensure that the render passes don't begin until the image is available,
                // or we can make the render pass wait for the `VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT` stage.
                // I've decided to go with the second option here, because it's a good excuse to have a look at subpass dependencies and how they work.

                vk::SubpassDependency dependency
                (
                    VK_SUBPASS_EXTERNAL,
                    0,
                    vk::PipelineStageFlagBits::eColorAttachmentOutput,
                    vk::PipelineStageFlagBits::eColorAttachmentOutput,
                    vk::AccessFlags{},
                    vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite,
                    vk::DependencyFlags{}
                );

                std::vector<vk::AttachmentDescription> attachments{color_attachment, depth_attachment, color_attachment_resolve};
                std::vector<vk::SubpassDescription> subpasses{subpass};
                std::vector<vk::SubpassDependency> dependnecies{dependency};

                vk::RenderPassCreateInfo render_pass_create_information
                (
                    vk::RenderPassCreateFlags{},
                    attachments,
                    subpasses,
                    dependnecies
                );

                return RenderPass::Create(device, render_pass_create_information);
            }

            virtual RenderPassPtr New(const vk_utility::device::DevicePtr &device)  override
            {
                return vk_utility::make_shared(Create(device));
            }
        };
    }
}
