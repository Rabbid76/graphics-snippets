#pragma once

#include <vk_utility_object.h>
#include <vk_utility_exception.h>
#include <vk_utility_vulkan_include.h>
#include <vk_utility_device.h>
#include <vk_utility_shader_module.h>
#include <vk_utility_shader_module_factory.h>

#include <memory>
#include <fstream>

namespace vk_utility
{

    namespace shader
    {
        /// <summary>
        /// Interface for Vulkan pipeline (`vk::ShaderModule`) factories
        /// </summary>
        class ShaderModuleFactorySpirVFile
            : public IShaderModuleFactory
        {
        private:

            std::string _filename;

        public:

            ShaderModuleFactorySpirVFile(void) = default;
            ShaderModuleFactorySpirVFile(const ShaderModuleFactorySpirVFile&) = delete;

            ShaderModuleFactorySpirVFile& set_source_file(const std::string & filename)
            {
                _filename = filename;
                return *this;
            }

            virtual ShaderModule Create(vk_utility::device::DevicePtr device)
            {
                auto source_code = read_file(_filename);
                return ShaderModule::Create(device, source_code);
            }

            virtual ShaderModulePtr New(vk_utility::device::DevicePtr device)
            {
                return vk_utility::make_shared(Create(device));
            }

        private:

            std::vector<char> read_file(const std::string& filename)
            {
                std::ifstream file(filename, std::ios::ate | std::ios::binary);

                if (!file.is_open()) {
                    throw exception::Exception("failed to open file!");
                }

                size_t fileSize = (size_t)file.tellg();
                std::vector<char> buffer(fileSize);
                file.seekg(0);
                file.read(buffer.data(), fileSize);
                file.close();

                return buffer;
            }
        };
    }
}
