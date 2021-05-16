#ifndef __CONTROLS_VIEW_INTERFACE__H__
#define __CONTROLS_VIEW_INTERFACE__H__

#include <math/glm_include.h>

#include <array>

namespace controls
{
    using TViewportRectangle = std::array<float, 4>;

    class ControlsViewInterface
    {
    public:

        virtual const TViewportRectangle& get_viewport_rectangle(void) const = 0;
        virtual const glm::mat4& get_view_matrix(void) const = 0;
        virtual const glm::mat4& get_inverse_view_matrix(void) const = 0;
        virtual const glm::mat4& get_projection_matrix(void) const = 0;
        virtual const glm::mat4& get_inverse_projection_matrix(void) const = 0;
    };
}

#endif