#ifndef __MESH_DEFINITON_DODECAHEDRON__
#define __MESH_DEFINITON_DODECAHEDRON__

#include <mesh/mesh_definition_interface.h>
#include <mesh/mesh_data_container.h>

#include <cmath>

namespace mesh
{
    // Dodecahedron https://en.wikipedia.org/wiki/Dodecahedron
    template <class T_VERTEX = float, class T_INDEX = unsigned int>
    class MeshDefinitonDodecahedron
        : public MeshDefinitionInterface<T_VERTEX, T_INDEX>
    {
    private:

        T_VERTEX _radius = 1.0;

    public:

        MeshDefinitonDodecahedron(void) {}
        MeshDefinitonDodecahedron(T_VERTEX radius) : _radius(radius) {}

        virtual std::shared_ptr<MeshDataInterface<T_VERTEX, T_INDEX>> generate_mesh_data(void) const override;
    };

    template<class T_VERTEX, class T_INDEX>
    std::shared_ptr<MeshDataInterface<T_VERTEX, T_INDEX>> MeshDefinitonDodecahedron<T_VERTEX, T_INDEX>::generate_mesh_data(void) const
    {
        const T_VERTEX phi = static_cast<T_VERTEX>((1.0 + std::sqrt(5.0)) / 2.0); //  phi = (1 + sqrt(5)) / 2 is the Golden Ratio.
        const T_VERTEX phi2 = phi * phi;
        const T_VERTEX a = static_cast<T_VERTEX>(1.0);
        const T_VERTEX b = static_cast<T_VERTEX>(1.0 / phi);
        const T_VERTEX c = static_cast<T_VERTEX>(1.0 / phi2);
        const auto v = std::vector<T_VERTEX>{
            -b, -b, -b, b, -b, -b, b, b, -b, -b, b, -b, // 0..7: (+/ -phi, +/ -phi, +/ -phi)
            -b, -b, b, b, -b, b, b, b, b, -b, b, b,
            0, -a, -c, 0, a, -c, 0, a, c, 0, -a, c, // 8..11: (0, +/ -phi ^ 2, +/ -1)
            -a, -c, 0, a, -c, 0, a, c, 0, -a, c, 0, // 12..15: (+/ -phi ^ 2, +/ -1, 0)
            -c, 0, -a, c, 0, -a, c, 0, a, -c, 0, a, // 16..19: (+/ -phi, 0, +/ -phi ^ 2)
        };
        const auto t = std::vector<T_VERTEX>{ 0,0, 1,0, 1,0.5, 0.5,1, 0,0.5 };
        const auto e = std::vector<int>{ 
            16,17,1,8,0, 17,16,3,9,2, 19,18,6,10,7, 18,19,4,11,5,
            14,13,1,17,2, 13,14,6,18,5, 15,12,4,19,7, 12,15,3,16,0,
            9,10,6,14,2, 10,9,3,15,7, 8,11,4,12,0, 11,8,1,13,5 
        };

        T_VERTEX l = _radius;
        std::vector<T_VERTEX> vertex_attributes;
        std::vector<T_INDEX> indices;
        for (int si = 0; si < e.size() / 5; ++si)
        {
            glm::vec3 n(0.0f);
            for (int pi = 0; pi < 5; ++pi)
                n += glm::vec3(v[e[si * 5 + pi] * 3], v[e[si * 5 + pi] * 3 + 1], v[e[si * 5 + pi] * 3 + 2]);
            auto cpt = n / static_cast<T_VERTEX>(5);
            n = normalize(n);
            auto w = static_cast<T_VERTEX>(si / 2 + si % 2 * 6) / 12;
            vertex_attributes.insert(vertex_attributes.end(),
                {
                    l * cpt[0], l * cpt[1], l * cpt[2], n.x, n.y, n.z, 0.5, 0.5, w
                });
            for (int pi = 0; pi < 5; ++pi)
            {
                int i = si * 5 + pi;
                vertex_attributes.insert(vertex_attributes.end(),
                    {
                        l * v[e[i] * 3], l * v[e[i] * 3 + 1], l * v[e[i] * 3 + 2],
                        n.x, n.y, n.z,
                        t[pi * 2], t[pi * 2 + 1], w
                    });

                T_INDEX e0 = si * 6;
                indices.insert(indices.end(),
                    {
                        e0, e0 + 1, e0 + 2, e0, e0 + 2, e0 + 3, e0, e0 + 3, e0 + 4, e0, e0 + 4, e0 + 5, e0, e0 + 5, e0 + 1
                    });
            }
        }
        return std::make_shared<MeshDataContainer<T_VERTEX, T_INDEX>>(
            std::move(vertex_attributes),
            std::move(indices),
            VertexSpcification{ {AttributeType::vertex, 3}, {AttributeType::normal_vector, 3}, {AttributeType::texture_uvw, 3} }
        );
        return nullptr;
    }
}

#endif