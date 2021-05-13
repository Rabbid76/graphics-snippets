#ifndef __CONTROLS_VIEW_INTERFACE__
#define __CONTROLS_VIEW_INTERFACE__

#include <array>

namespace controls
{
    using TViewportRectangle = std::array<float, 4>;

    class ControlsViewInterface
    {
    public:

        virtual const double get_time(void) const = 0; // TODO: SOLID - interface segregation

        virtual const TViewportRectangle& get_viewport_rectangle(void) const = 0;
        virtual const glm::mat4 get_view_matrix(void) const = 0;
        virtual const glm::mat4 get_inverse_view_matrix(void) const = 0;
        virtual const glm::mat4 get_projection_matrix(void) const = 0;
        virtual const glm::mat4 get_inverse_projection_matrix(void) const = 0;
    };
}

#endif