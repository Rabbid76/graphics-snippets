#ifndef __MESH_SPECIFICATION_H__
#define __MESH_SPECIFICATION_H__

#include "meshMath.h"
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
        Point3 aabbMin;
        Point3 aabbMax;

        static MeshData creatFromVectors(
                const std::vector<uint32_t> &indices,
                const std::vector<float> &vertices,
                const std::vector<float> &normals,
                const std::vector<float> &uvs,
                const Point3 &boxMin,
                const Point3 &boxMax) {
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