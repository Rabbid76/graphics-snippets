#ifndef __CANVAS_INTERFACE__
#define __CANVAS_INTERFACE__

#include <tuple>

namespace view
{
    class canvas_interface
    {
    public:

        virtual std::tuple<int, int> get_size(void) const = 0;
    };
}

#endif
