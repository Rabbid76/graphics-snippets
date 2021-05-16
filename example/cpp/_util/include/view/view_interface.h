#ifndef __VIEW_INTERFACE__H__
#define __VIEW_INTERFACE__H__

#include <view/canvas_interface.h>

namespace view
{
    class ViewInterface
    {
    public:

        virtual void init(const CanvasInterface&) = 0;
        virtual void resize(const CanvasInterface&) = 0;
        virtual void render(const CanvasInterface&) = 0;
    };
}

#endif