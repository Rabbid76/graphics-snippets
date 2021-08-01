#pragma once


#include <vector>
#include <algorithm>


namespace vk_utility
{

    namespace type
    {


        template<typename T_SRING_CONTAINER>
        struct StringPointerArray
        {
            std::vector<const char *> _list;

            StringPointerArray(const T_SRING_CONTAINER &container)
            {
                for (auto &str : container)
                    _list.push_back(str.c_str());
            }

            auto size(void) const
            {
                return static_cast<uint32_t>(_list.size());
            }

            auto data(void) const
            {
                return _list.empty() ? nullptr : _list.data();
            }

           const auto & list(void) const 
           {
               return _list;
           }
        };
    }
}
