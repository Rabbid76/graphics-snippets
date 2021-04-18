#ifndef __MESH_DATA_INTERFACE__
#define __MESH_DATA_INTERFACE__

#include <vector>
#include <tuple>

namespace mesh
{
    template <class T_VERTEX = float>
    using VertexAttributes = std::tuple<size_t, const T_VERTEX*>;

    template <class T_INDEX = unsigned int>
    using Indices = std::tuple<size_t, const T_INDEX*>;

    template <class T_VERTEX = float, class T_INDEX = unsigned int>
    using VertexSpcification = std::vector<std::tuple<int, int>>;

    template <class T_VERTEX = float, class T_INDEX = int>
    class MeshDataInterface
    {
    public:

        virtual const VertexAttributes<T_VERTEX> get_vertex_attributes(void) const = 0;
        virtual const Indices<T_INDEX> get_indices(void) const = 0;
        virtual const VertexSpcification<T_VERTEX, T_INDEX> get_specification(void) const = 0;
    };
}

#endif
