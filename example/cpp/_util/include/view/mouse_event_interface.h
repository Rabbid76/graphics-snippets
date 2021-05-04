#pragma once

namespace view
{
    class MouseEventInterface
    {
    public:

        virtual void mouse_motion(int x, int y) const = 0;
    };
}