#ifndef __MESH_DEFINITION_TETRAHEDRON__
#define __MESH_DEFINITION_TETRAHEDRON__

#include <mesh/mesh_definition_interface.h>
#include <mesh/mesh_data_container.h>

#include <cmath>

namespace mesh
{
    // Tetrahedron https://en.wikipedia.org/wiki/Tetrahedron
    template <class T_VERTEX = float, class T_INDEX = unsigned int>
    class MeshDefinitonTetrahedron
        : public MeshDefinitionInterface<T_VERTEX, T_INDEX>
    {
    private:

        T_VERTEX _radius = 1.0;

    public:

        MeshDefinitonTetrahedron(void) {}
        MeshDefinitonTetrahedron(T_VERTEX radius) : _radius(radius) {}

        virtual std::shared_ptr<MeshDataInterface<T_VERTEX, T_INDEX>> generate_mesh_data(void) const override;
    };

    template<class T_VERTEX, class T_INDEX>
    std::shared_ptr<MeshDataInterface<T_VERTEX, T_INDEX>> MeshDefinitonTetrahedron<T_VERTEX, T_INDEX>::generate_mesh_data(void) const
    {
        T_VERTEX s_8_9 = static_cast<T_VERTEX>(std::sqrt(8.0 / 9.0));
        T_VERTEX s_2_9 = static_cast<T_VERTEX>(std::sqrt(2.0 / 9.0));
        T_VERTEX s_2_3 = static_cast<T_VERTEX>(std::sqrt(2.0 / 3.0));
        T_VERTEX s_1_3 = static_cast<T_VERTEX>(1.0 / 3.0);
        auto v = std::vector<T_VERTEX>{ 0, 0, 1, s_8_9, 0, -s_1_3, -s_2_9, s_2_3, -s_1_3, -s_2_9, -s_2_3, -s_1_3 };
        auto t = std::vector<T_VERTEX>{ 0, 0, 0, 1, 1, 1, 1, 0 };
        auto ni = std::vector<int>{ 3, 1, 2, 0 };
        auto e = std::vector<int>{ 0, 1, 2, 0, 2, 3, 0, 3, 1, 1, 3, 2 };

        T_VERTEX l = _radius;
        std::vector<T_VERTEX> vertex_attributes;
        for (int si = 0; si < 4; ++si)
        {
            for (int pi = 0; pi < 3; ++pi)
            {
                int i = si * 3 + pi;
                vertex_attributes.insert(vertex_attributes.end(),
                    { 
                        l * v[e[i] * 3], l * v[e[i] * 3 + 1], l * v[e[i] * 3 + 2],
                        -v[ni[si] * 3], -v[ni[si] * 3 + 1], -v[ni[si] * 3 + 2],
                        t[e[i] * 2], t[e[i] * 2 + 1], static_cast<T_VERTEX>(si) / 4
                    });
            }
        }

        return std::make_shared<MeshDataContainer<T_VERTEX, T_INDEX>>(
            std::move(vertex_attributes),
            std::vector<T_INDEX>{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11 },
            VertexSpcification{ {AttributeType::vertex, 3}, {AttributeType::normal_vector, 3}, {AttributeType::texture_uvw, 3} }
        );
    }
}

#endif
