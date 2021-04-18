#ifndef __MESH_DATA_CONTAINER__
#define __MESH_DATA_CONTAINER__

#include <mesh/mesh_data_interface.h>

#include <vector>
#include <tuple>

namespace mesh
{
    template <class T_VERTEX = float, class T_INDEX = int>
    class MeshDataContainer
        : public MeshDataInterface<T_VERTEX, T_INDEX>
    {
    private:

        std::vector<T_VERTEX> _vertex_attributes;
        std::vector<T_INDEX> _indices;
        VertexSpcification _specification;

    public:

        MeshDataContainer(
            std::vector<T_VERTEX>&& vertex_attributes,
            std::vector<T_INDEX>&& indices,
            VertexSpcification&& specification)
            : _vertex_attributes(std::move(vertex_attributes))
            , _indices(std::move(indices))
            , _specification(std::move(specification))
        {}

        static MeshDataContainer new_mesh(
            std::vector<T_VERTEX>&& vertex_attributes,
            std::vector<T_INDEX>&& indices,
            VertexSpcification&& specification)
        {
            return MeshDataContainer(std::move(vertex_attributes), std::move(indices), std::move(specification));
        }

        virtual const VertexAttributes<T_VERTEX> get_vertex_attributes(void) const
        {
            return std::make_tuple<size_t, const T_VERTEX*>(_vertex_attributes.size(), _vertex_attributes.data());
        }

        virtual const Indices<T_INDEX> get_indices(void) const
        {
            return std::make_tuple<size_t, const T_INDEX*>(_indices.size(), _indices.data());
        }

        virtual const VertexSpcification get_specification(void) const
        {
            return _specification;
        }
    };
}

#endif
