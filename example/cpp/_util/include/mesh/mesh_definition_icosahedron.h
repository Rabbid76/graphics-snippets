#ifndef __MESH_DEFINION_ICOSAHEDRON__
#define __MESH_DEFINION_ICOSAHEDRON__

#include <mesh/mesh_definition_interface.h>
#include <mesh/mesh_data_container.h>

#include <cmath>

namespace mesh
{
    // Icosahedron https://en.wikipedia.org/wiki/Icosahedron
    template <class T_VERTEX = float, class T_INDEX = unsigned int>
    class MeshDefinitonIcosahedron
        : public MeshDefinitionInterface<T_VERTEX, T_INDEX>
    {
    private:

        T_VERTEX _radius = 1.0;

    public:

        MeshDefinitonIcosahedron(void) {}
        MeshDefinitonIcosahedron(T_VERTEX radius) : _radius(radius) {}

        virtual std::shared_ptr<MeshDataInterface<T_VERTEX, T_INDEX>> generate_mesh_data(void) const override;
    };

    template<class T_VERTEX, class T_INDEX>
    std::shared_ptr<MeshDataInterface<T_VERTEX, T_INDEX>> MeshDefinitonIcosahedron<T_VERTEX, T_INDEX>::generate_mesh_data(void) const
    {
        // TODO 
        const T_VERTEX phi = static_cast<T_VERTEX>((1.0 + std::sqrt(5.0)) / 2.0); //  phi = (1 + sqrt(5)) / 2 is the Golden Ratio.
        // (0, +/-1, +/-phi)
        // (+/-1, +/-phi, 0)
        // (+/-phi, 0, +/-1)

        const auto v = std::vector<T_VERTEX>
        {
            0,0,1, 0.894f,0,0.447f, 0.276f,0.851f,0.447f, -0.724f,0.526f,0.447f, -0.724f,-0.526f,0.447f, 0.276f,-0.851f, 0.447f,
            0.724f, 0.526f,-0.447f, -0.276f,0.851f,-0.447f, -0.894f,0,-0.447f, -0.276f,-0.851f,-0.447f, 0.724f,-0.526f,-0.447f, 0,0,-1
        };
        const auto e = std::vector<int>{
            1,2,0, 2,3,0, 3,4,0, 4,5,0, 5,1,0, 7,6,11, 8,7,11, 9,8,11, 10,9,11, 6,10,11,
            1,6,2, 2,7,3, 3,8,4, 4,9,5, 5,10,1, 7,2,6, 8,3,7, 9,4,8, 10,5,9,  6,1,10
        };
        const auto t = std::vector<T_VERTEX>{ 0,0, 1,0, 1,1, 0,1 };
        std::vector<glm::vec3> n;
        for (int fi = 0; fi < e.size() / 3; ++fi)
        {
            glm::vec3 nv(0.0f);
            for (int ti = 0; ti < 3; ++ti)
                nv += glm::vec3(v[e[fi * 3 + ti] * 3], v[e[fi * 3 + ti] * 3 + 1], v[e[fi * 3 + ti] * 3 + 2]);
            n.push_back(glm::normalize(nv));
        }

        T_VERTEX l = _radius;
        std::vector<T_VERTEX> vertex_attributes;
        for (int i = 0; i < e.size(); ++i)
        {
            auto ti = i % 3;
            auto fi = i / 3;
            auto shift = ti == 0 || fi >= 10 ? 0 : 2;
            vertex_attributes.insert(vertex_attributes.end(),
                {
                    l* v[e[i] * 3], l* v[e[i] * 3 + 1], l* v[e[i] * 3 + 2],
                    n[fi].x, n[fi].y, n[fi].z,
                    t[ti * 2 + shift], t[ti * 2 + 1 + shift], static_cast<T_VERTEX>(fi / 2 + fi % 2 * 10) / 20
                });
        }

        std::vector<T_INDEX> indices(vertex_attributes.size() / 9);
        std::iota(indices.begin(), indices.end(), 0);
        return std::make_shared<MeshDataContainer<T_VERTEX, T_INDEX>>(
            std::move(vertex_attributes),
            std::move(indices),
            VertexSpcification{ {AttributeType::vertex, 3}, {AttributeType::normal_vector, 3}, {AttributeType::texture_uvw, 3} }
        );
    };
}

#endif
