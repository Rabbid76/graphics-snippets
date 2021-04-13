#ifndef __WX_OPENGL_VIEW_INTERFACE__
#define __WX_OPENGL_VIEW_INTERFACE__

namespace wxutil
{
    class open_gl_view_interface
    {
    public:

        virtual void init(void) = 0;
        virtual void resize(void) = 0;
        virtual void draw(void) = 0;
    };
}

#endif
