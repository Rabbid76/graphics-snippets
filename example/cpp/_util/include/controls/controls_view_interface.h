#ifndef __CONTROLS_VIEW_INTERFACE__
#define __CONTROLS_VIEW_INTERFACE__

#include <array>

namespace controls
{
    using TViewportRectangle = std::array<float, 4>;

    class ControlsViewInterface
    {
        virtual const TViewportRectangle& get_viewport_rectangle(void) = 0;
    };
}

#endif