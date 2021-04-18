#ifndef __MESH_DEFINITION_INTERFACE__
#define __MESH_DEFINITION_INTERFACE__

#include <mesh/mesh_data_interface.h>

#include <memory>

namespace mesh
{
    template <class T_VERTEX = float, class T_INDEX = unsigned int>
    class MeshDefinitionInterface
    {
    public:

        virtual std::shared_ptr<MeshDataInterface<T_VERTEX, T_INDEX>> generate_mesh_data(void) const = 0;
    };
}

#endif
