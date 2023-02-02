#ifndef __MESH_SPECIFICATION_H__
#define __MESH_SPECIFICATION_H__

#include <array>
#include <cstdint>
#include <vector>

namespace mesh {

    enum class Operator { MINUS, OR, AND };

    class MeshData {
    public:
        uint32_t noOfVertices;
        uint32_t noOfIndices;
        const float *vertices = nullptr;
        const float *normals = nullptr;
        const float *uvs = nullptr;
        const uint32_t *indices = nullptr;
        std::array<float, 3> aabbMin;
        std::array<float, 3> aabbMax;

        static MeshData creatFromVectors(
                const std::vector<uint32_t> &indices,
                const std::vector<float> &vertices,
                const std::vector<float> &normals,
                const std::vector<float> &uvs,
                const std::array<float, 3> &boxMin,
                const std::array<float, 3> &boxMax) {
            return mesh::MeshData{
                    (uint32_t)vertices.size() / 3, (uint32_t)indices.size(),
                    vertices.data(),
                    normals.empty() ? nullptr : normals.data(),
                    uvs.empty() ? nullptr : uvs.data(),
                    indices.data(),
                    boxMin, boxMax
            };
        }
    };

    class ResultMeshData {
    public:
        std::vector<float> vertices;
        std::vector<float> normals;
        std::vector<float> uvs;
        std::vector<uint32_t> indicesOut;
        std::vector<uint32_t> indicesIn;
        bool error = false;
    };
}

#endif