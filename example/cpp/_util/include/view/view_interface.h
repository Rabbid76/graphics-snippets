#ifndef __VIEW_INTERFACE__
#define __VIEW_INTERFACE__

#include <view/canvas_interface.h>

namespace view
{
    class view_interface
    {
    public:

        virtual void init(const canvas_interface&) = 0;
        virtual void resize(const canvas_interface&) = 0;
        virtual void render(const canvas_interface&) = 0;
    };
}

#endif