#ifndef __OPENGL_MESH_VECTOR__
#define __OPENGL_MESH_VECTOR__

#include <gl/opengl_mesh_interface.h>

#include <vector>
#include <memory>
#include <algorithm>

namespace OpenGL::mesh
{
    class MeshVector
        : public MeshInterface
        , public MultiMeshInterface
    {
    private:

        std::vector<std::shared_ptr<OpenGL::mesh::MeshInterface>> _meshs;

    public:

        MeshVector(void) = default;
        MeshVector(const std::vector<std::shared_ptr<OpenGL::mesh::MeshInterface>>& meshs)
            : _meshs(meshs)
        {}

        virtual ~MeshVector() = default;

        virtual void destroy(void)
        {
            std::for_each(_meshs.begin(), _meshs.end(), [](const auto& mesh)
            {
                mesh->destroy();
            });
        }

        virtual void draw(void) const override
        {
            std::for_each(_meshs.begin(), _meshs.end(), [](const auto& mesh)
            {
                mesh->draw();
            });
        }

        virtual void draw(size_t from, size_t to) const override
        {
            for (auto i = from; i < to; ++i)
                _meshs[i]->draw();
        }
    };
}

#endif