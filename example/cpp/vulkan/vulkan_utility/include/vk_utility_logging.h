#pragma once


#include <vector>
#include <iostream>
#include <string>
#include <sstream>


namespace vk_utility
{

    namespace logging
    {

        /// <summary>
        /// Logging
        ///
        /// See `std::endl` is of unknown type when overloading `operator<<`
        /// https://stackoverflow.com/questions/1134388/stdendl-is-of-unknown-type-when-overloading-operator) 
        /// </summary>
        class Log
        {
        protected:

            std::stringstream _stream;

        public:

            template<typename T>
            Log & operator << (const T &data)
            {
                _stream << data;
                return flush();
            }

            Log & operator << (std::ostream& (*os)(std::ostream&))
            {
                _stream << os;
                return flush();
            }

            virtual Log & flush(void)
            {
                std::cout << _stream.str();
                _stream.str("");
                return *this;
            }
        };

        /// <summary>
        /// Log list of objects
        /// </summary>
        class LogList
        {
        private:
            vk_utility::logging::Log &_log;

        public:

            LogList(vk_utility::logging::Log &log)
                : _log(log)
            {}

            template<typename T>
            LogList &log(const std::string &title, const T &container)
            {
                _log << container.size() << " " << title << std::endl;
                for (const auto &name : container)
                    _log << "\t" << name << std::endl;
                _log << std::endl;
                return *this;
            }

            template<typename T_CONTAINER, typename T_FUNCTION>
            LogList &log(const std::string &title, const T_CONTAINER &container, T_FUNCTION function)
            {
                _log << container.size() << " " << title << std::endl;
                for (const auto &element : container)
                    function(element);
                _log << std::endl;
                return *this;
            }

            template<typename T_CONTAINER, typename T_CONDITION>
            LogList &log_if(const T_CONTAINER &container, const std::string &message, T_CONDITION condition) 
            {
                for (const auto &name : container)
                {
                    if (condition(name))
                        _log << "\"" << name << "\" " << message << std::endl;
                }
                _log << std::endl;
                return *this;
            }
        };
    }
}