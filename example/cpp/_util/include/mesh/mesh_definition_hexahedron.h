#ifndef __MESH_DEFINITION_HEXAHEDRON__
#define __MESH_DEFINITION_HEXAHEDRON__

#include <mesh/mesh_definition_interface.h>
#include <mesh/mesh_data_container.h>

#include <glm/glm.hpp>

#include <cmath>

namespace mesh
{
    // Hexahedron https://en.wikipedia.org/wiki/Hexahedron
    template <class T_VERTEX = float, class T_INDEX = unsigned int>
    class MeshDefinitonHexahedron
        : public MeshDefinitionInterface<T_VERTEX, T_INDEX>
    {
    private:

        T_VERTEX _radius = 1.0;

    public:
    
        MeshDefinitonHexahedron(void) {}
        MeshDefinitonHexahedron(T_VERTEX radius) : _radius(radius) {}

        virtual std::shared_ptr<MeshDataInterface<T_VERTEX, T_INDEX>> generate_mesh_data(void) const override;
    };

    template<class T_VERTEX, class T_INDEX>
    std::shared_ptr<MeshDataInterface<T_VERTEX, T_INDEX>> MeshDefinitonHexahedron<T_VERTEX, T_INDEX>::generate_mesh_data(void) const
    {
        const auto v = std::vector<T_VERTEX>{ -1,-1,1, 1,-1,1, 1,1,1, -1,1,1, -1,-1,-1, 1,-1,-1, 1,1,-1, -1,1,-1 };
        const auto t = std::vector<T_VERTEX>{ 0,1, 1,1, 1,0, 0,0 };
        const auto n = std::vector<T_VERTEX>{ 0,0,1, 1,0,0, 0,0,-1, -1,0,0, 0,1,0, 0,-1,0 };
        const auto e = std::vector<T_INDEX>{ 0,1,2,3, 1,5,6,2, 5,4,7,6, 4,0,3,7, 3,2,6,7, 1,0,4,5 };
           
        T_VERTEX l = _radius / std::sqrt(3.0f);
        std::vector<T_VERTEX> vertex_attributes;
        std::vector<T_INDEX> indices;
        for (int si = 0; si < 6; ++si)
        {
            for (int pi = 0; pi < 4; ++pi)
            {
                int i = si * 4 + pi;
                vertex_attributes.insert(vertex_attributes.end(),
                    {
                        l * v[e[i] * 3], l * v[e[i] * 3 + 1], l * v[e[i] * 3 + 2],
                        n[si * 3], n[si * 3 + 1], n[si * 3 + 2],
                        t[pi * 2], t[pi * 2 + 1], static_cast<T_VERTEX>(si) / 6
                    });
            }
            T_INDEX j = static_cast<T_INDEX>(si);
            indices.insert(indices.end(),
                {
                    j * 4, j * 4 + 1, j * 4 + 2, j * 4, j * 4 + 2, j * 4 + 3
                });
        }
        return std::make_shared<MeshDataContainer<T_VERTEX, T_INDEX>>(
            std::move(vertex_attributes),
            std::move(indices),
            VertexSpcification{ {AttributeType::vertex, 3}, {AttributeType::normal_vector, 3}, {AttributeType::texture_uvw, 3} }
        );
    }
}

#endif
