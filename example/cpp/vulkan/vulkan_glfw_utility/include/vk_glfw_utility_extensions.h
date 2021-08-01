#pragma once


#include <vk_glfw_utility_include.h>


#include <string>
#include <vector>
#include <algorithm>


namespace vk_glfw_utility
{

    namespace core
    {
        /// <summary>
        /// GLFW Vulkan extensions
        /// see GLFW Vulkan reference https://www.glfw.org/docs/3.3/group__vulkan.html
        ///
        /// Vulkan is a platform agnostic API, which means that you need an extension to interface with the window system.
        /// GLFW has a handy built-in function that returns the extension(s).
        /// </summary>
        class Extensions
        {
        private:

            mutable std::vector<std::string> _extension_names;

        public:

            Extensions(void) = default;
            Extensions(const Extensions &) = default;
            Extensions &operator = (const Extensions &) = default;

            auto &get_names(void) const
            {
                if (_extension_names.empty())
                {
                    unsigned int glfwExtensionCount = 0;
                    const char **glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
                    _extension_names.resize(glfwExtensionCount);
                    std::copy(glfwExtensions, glfwExtensions + glfwExtensionCount, _extension_names.begin());
                }
                return _extension_names;
            }

            bool supported(const std::string name) const
            {
                auto &extensions = get_names();
                return std::find(extensions.begin(), extensions.end(), name) != extensions.end();
            }
        };
    }
}

