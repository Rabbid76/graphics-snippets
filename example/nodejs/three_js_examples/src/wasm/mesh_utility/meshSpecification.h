#ifndef __MESH_SPECIFICATION_H__
#define __MESH_SPECIFICATION_H__

#include <array>
#include <cstdint>
#include <unordered_map>
#include <vector>

namespace mesh {

    enum class Operator { MINUS, OR, AND };
    using MeshVertex = std::array<float, 3>;
    constexpr float INDEX_EPSILON = 0.0001f;

    class MeshDataReference {
    public:
        uint32_t noOfVertices;
        uint32_t noOfIndices;
        const float *vertices = nullptr;
        const float *normals = nullptr;
        const float *uvs = nullptr;
        const uint32_t *indices = nullptr;
        MeshVertex aabbMin;
        MeshVertex aabbMax;

        static MeshDataReference creatFromVectors(
                const std::vector<uint32_t> &indices,
                const std::vector<float> &vertices,
                const std::vector<float> &normals,
                const std::vector<float> &uvs,
                const MeshVertex &boxMin,
                const MeshVertex &boxMax) {
            return mesh::MeshDataReference{
                    (uint32_t)vertices.size() / 3, (uint32_t)indices.size(),
                    vertices.data(),
                    normals.empty() ? nullptr : normals.data(),
                    uvs.empty() ? nullptr : uvs.data(),
                    indices.data(),
                    boxMin, boxMax
            };
        }

        const float * getVertex(uint32_t index) const { return vertices + index * 3; }

        static void findIntersecting(const MeshDataReference &meshA, const MeshDataReference &meshB, std::vector<uint32_t> &trianglesA, std::vector<uint32_t> &trianglesB);
    };

    class MeshDataInstance {
    public:
        std::vector<float> vertices;
        std::vector<float> normals;
        std::vector<float> uvs;
        std::vector<uint32_t> indicesOut;
        std::vector<uint32_t> indicesIn;
        bool error = false;
    };

    class UniqueIndices {
    public:
        using DistanceMap = std::unordered_map<float, uint32_t>;
        using PlaneMap = std::unordered_map<float, DistanceMap>;
        using VolumeMap = std::unordered_map<float, PlaneMap>;
        using IndexMap = std::vector<uint32_t>;

        uint32_t nextIndex = 0;
        VolumeMap vertexToIndexMap;

        uint32_t getNextIndex() const { return nextIndex; }
        uint32_t getIndex(float x, float y, float z);
        uint32_t getVertexIndex(const float v[], float epsilon = INDEX_EPSILON);
        IndexMap createUniqueIndices(const float *vertices, uint32_t noOfVertices, float epsilon = INDEX_EPSILON);
    };
}

#endif