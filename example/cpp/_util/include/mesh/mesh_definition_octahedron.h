#ifndef __MESH_DEFINITION_OCTAHEDRON__
#define __MESH_DEFINITION_OCTAHEDRON__

#include <mesh/mesh_definition_interface.h>
#include <mesh/mesh_data_container.h>

#include <glm/glm.hpp>

#include <cmath>

namespace mesh
{
    // Octahedron https://en.wikipedia.org/wiki/Octahedron
    template <class T_VERTEX = float, class T_INDEX = unsigned int>
    class MeshDefinitonOctahedron
        : public MeshDefinitionInterface<T_VERTEX, T_INDEX>
    {
    private:

        T_VERTEX _radius = 1.0;

    public:

        MeshDefinitonOctahedron(void) {}
        MeshDefinitonOctahedron(T_VERTEX radius) : _radius(radius) {}

        virtual std::shared_ptr<MeshDataInterface<T_VERTEX, T_INDEX>> generate_mesh_data(void) const override;
    };

    template<class T_VERTEX, class T_INDEX>
    std::shared_ptr<MeshDataInterface<T_VERTEX, T_INDEX>> MeshDefinitonOctahedron<T_VERTEX, T_INDEX>::generate_mesh_data(void) const
    {
        auto v = std::vector<T_VERTEX>{ -1,0,0, 0,-1,0, 0,0,-1, 1,0,0, 0,1,0, 0,0,1 };
        auto t = std::vector<T_VERTEX>{ 0,0, 1,0, 0.5,0.5, 1,1, 0,1, 0.5, 0.5 };
        auto nf = std::vector<T_VERTEX>{ -1,-1,-1, 1,-1,-1, 1,1,-1, -1,1,-1, -1,-1,1, 1,-1,1, 1,1,1, -1,1,1 };
        auto ni = std::vector<int>{ 3, 1, 2, 0 };
        auto e = std::vector<int>{ 0,2,1, 1,2,3, 3,2,4, 4,2,0, 0,1,5, 1,3,5, 3,4,5, 4,0,5 };

        T_VERTEX l = _radius;
        std::vector<T_VERTEX> vertex_attributes;
        for (int si = 0; si < 8; ++si)
        {
            auto n = glm::normalize(glm::vec3(nf[si * 3], nf[si * 3 + 1], nf[si * 3 + 2]));
            for (int pi = 0; pi < 3; ++pi)
            {
                int i = si * 3 + pi;
                vertex_attributes.insert(vertex_attributes.end(),
                    {
                        l * v[e[i] * 3], l * v[e[i] * 3 + 1], l * v[e[i] * 3 + 2],
                        n.x, n.y, n.z,
                        t[e[i] * 2], t[e[i] * 2 + 1], static_cast<T_VERTEX>(si) / 8
                    });
            }
        }

        std::vector<T_INDEX> indices(24);
        std::iota(indices.begin(), indices.end(), 0);
        return std::make_shared<MeshDataContainer<T_VERTEX, T_INDEX>>(
            std::move(vertex_attributes),
            std::move(indices),
            VertexSpcification{ {AttributeType::vertex, 3}, {AttributeType::normal_vector, 3}, {AttributeType::texture_uvw, 3} }
        );
    }
}

#endif
