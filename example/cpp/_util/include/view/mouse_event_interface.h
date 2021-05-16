#ifndef __MOUSEVENT_INTERFACE__H__
#define __MOUSEVENT_INTERFACE__H__

namespace view
{
    enum class MouseButton
    {
        LEFT,
        RIGHT
    };

    enum class MouseAction
    {
        PRESS,
        RELEASE
    };

    class MouseEventInterface
    {
    public:

        virtual void mouse_motion(int x, int y) const = 0;
        virtual void mouse_action(int x, int y, MouseButton button, MouseAction action) const = 0;
    };
}

#endif