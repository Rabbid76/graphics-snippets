#ifndef __CANVAS_INTERFACE__
#define __CANVAS_INTERFACE__

#include <tuple>

namespace view
{
    class CanvasInterface
    {
    public:

        virtual std::tuple<int, int> get_size(void) const = 0;
        virtual void activate(void) const = 0;
    };
}

#endif
