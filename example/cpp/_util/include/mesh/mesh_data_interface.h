#ifndef __MESH_DATA_INTERFACE__
#define __MESH_DATA_INTERFACE__

#include <vector>
#include <tuple>
#include <numeric>

namespace mesh
{
    enum class AttributeType : int
    {
        vertex = 0,
        normal_vector = 1,
        texture_uvw = 2,
    };

    template <class T_VERTEX = float>
    using VertexAttributes = std::tuple<size_t, const T_VERTEX*>;

    template <class T_INDEX = unsigned int>
    using Indices = std::tuple<size_t, const T_INDEX*>;

    using VertexSpcification = std::vector<std::tuple<AttributeType, int>>;

    template <class T_VERTEX = float, class T_INDEX = int>
    class MeshDataInterface
    {
    public:

        virtual const VertexAttributes<T_VERTEX> get_vertex_attributes(void) const = 0;
        virtual const Indices<T_INDEX> get_indices(void) const = 0;
        virtual const VertexSpcification get_specification(void) const = 0;

        size_t get_attribute_size(void) const
        {
            auto &&specification = get_specification();
            return std::accumulate(specification.begin(), specification.end(), 0, [](auto&& a, const auto& b) -> int
                {
                    return std::move(a) + std::get<1>(b);
                });
        }
    };
}

#endif
