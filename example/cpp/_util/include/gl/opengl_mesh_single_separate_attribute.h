#ifndef __OPENGL_MESH_SINGLE_SEPERATE_ATTRIBUTE__
#define __OPENGL_MESH_SINGLE_SEPERATE_ATTRIBUTE__

#include <mesh/mesh_data_interface.h>
#include <gl/opengl_mesh_interface.h>

namespace OpenGL::mesh
{
    class SingleMeshSeparateAttributeFormat
        : public MeshInterface
    {
    private:

        unsigned int _vertex_array_object = 0;
        unsigned int _vertex_buffer_object = 0;
        unsigned int _index_buffer_object = 0;
        size_t _no_of_vertices = 0;
        size_t _no_of_indices = 0;

    public:

        size_t no_of_vertices(void) const
        {
            return _no_of_vertices;
        }

        size_t no_of_indices(void) const
        {
            return _no_of_indices;
        }

        SingleMeshSeparateAttributeFormat(const ::mesh::MeshDataInterface<float, unsigned int>& specification);

        virtual void destroy(void);
        virtual void draw(void) const override;
    };
}

#endif