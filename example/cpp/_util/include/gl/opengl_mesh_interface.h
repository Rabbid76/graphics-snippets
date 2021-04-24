#ifndef __OPENGL_MESH_INTERFACE__
#define __OPENGL_MESH_INTERFACE__

namespace OpenGL::mesh
{
    class MeshInterface
    {
    public:

        virtual void destroy(void) = 0;
        virtual void draw(void) const = 0;
    };

    class MultiMeshInterface
    {
        virtual void draw(size_t from, size_t to) const = 0;
    };
}

#endif
