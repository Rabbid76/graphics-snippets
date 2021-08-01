#pragma once


#include <string>
#include <stdexcept>


namespace vk_utility
{

    namespace exception
    {

        class Exception
            : public std::runtime_error
        {
        public:

            explicit Exception(const char *what) : std::runtime_error(what) {}
            explicit Exception(const std::string &what) : std::runtime_error(what) {}
        };
    }
}
