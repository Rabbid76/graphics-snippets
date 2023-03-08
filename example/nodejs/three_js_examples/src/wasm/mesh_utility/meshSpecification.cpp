#include "mesh_utility/meshSpecification.h"
#include <algorithm>
#include <unordered_set>

using namespace mesh;

namespace {

    struct TriangleBox {
        MeshVertex aabbMin;
        MeshVertex aabbMax;
    };

    [[nodiscard]] MeshVertex minVertex(const float *a, const float *b, const float *c) {
        return {std::min(a[0], std::min(b[0], c[0])), std::min(a[1], std::min(b[1], c[1])), std::min(a[2], std::min(b[2], c[2]))};
    }

    [[nodiscard]] MeshVertex maxVertex(const float *a, const float *b, const float *c) {
        return {std::max(a[0], std::max(b[0], c[0])), std::max(a[1], std::max(b[1], c[1])), std::max(a[2], std::max(b[2], c[2]))};
    }

    [[nodiscard]] std::vector<TriangleBox> calculateBoxes(const MeshDataReference &mesh) {
        std::vector<TriangleBox> boxes;
        for (uint32_t triangleIndex = 0; triangleIndex < mesh.noOfIndices; triangleIndex += 3) {
            boxes.push_back({
                                    minVertex(mesh.getVertex(*(mesh.indices + triangleIndex)), mesh.getVertex(*(mesh.indices + triangleIndex + 1)), mesh.getVertex(*(mesh.indices + triangleIndex + 2))),
                                    maxVertex(mesh.getVertex(*(mesh.indices + triangleIndex)), mesh.getVertex(*(mesh.indices + triangleIndex + 1)), mesh.getVertex(*(mesh.indices + triangleIndex + 2)))
                            });
        }
        return boxes;
    }

    [[nodiscard]] bool isIntersectingAABB(const TriangleBox &aabbA, const TriangleBox &aabbB)  {
        constexpr float BOX_EPSILON = 0.00001f;
        return aabbA.aabbMin[0] - BOX_EPSILON <= aabbB.aabbMax[0] + BOX_EPSILON && aabbB.aabbMin[0] - BOX_EPSILON <= aabbA.aabbMax[0] + BOX_EPSILON &&
               aabbA.aabbMin[1] - BOX_EPSILON <= aabbB.aabbMax[1] + BOX_EPSILON && aabbB.aabbMin[1] - BOX_EPSILON <= aabbA.aabbMax[1] + BOX_EPSILON &&
               aabbA.aabbMin[2] - BOX_EPSILON <= aabbB.aabbMax[2] + BOX_EPSILON && aabbB.aabbMin[2] - BOX_EPSILON <= aabbA.aabbMax[2] + BOX_EPSILON;
    }
}

void MeshDataReference::findIntersecting(
        const MeshDataReference &meshA,
        const MeshDataReference &meshB,
        std::vector<uint32_t> &trianglesA,
        std::vector<uint32_t> &trianglesB) {

    auto boxesA = calculateBoxes(meshA);
    auto boxesB = calculateBoxes(meshB);

    std::unordered_set<uint32_t> setOfTrianglesB;
    for (uint32_t i = 0; i < boxesA.size(); ++ i) {
        for (uint32_t j = 0; j < boxesB.size(); ++ j) {
            if (isIntersectingAABB(boxesA[i], boxesB[j])) {
                if (trianglesA.empty() || trianglesA.back() != i)
                    trianglesA.push_back(i);
                setOfTrianglesB.insert(j);
            }
        }
    }
    trianglesB = std::vector<uint32_t>(setOfTrianglesB.begin(), setOfTrianglesB.end());
    std::sort(trianglesB.begin(), trianglesB.end());
}

uint32_t UniqueIndices::getIndex(float x, float y, float z) {
    auto volumeMapIt = vertexToIndexMap.find(x);
    if (volumeMapIt == vertexToIndexMap.end()) {
        auto [newVolumeItemIt, _added] = vertexToIndexMap.emplace(x, PlaneMap());
        volumeMapIt = newVolumeItemIt;
    }
    auto planeMapIt = volumeMapIt->second.find(y);
    if (planeMapIt == volumeMapIt->second.end()) {
        auto [newPlaneMapIt, _added] = volumeMapIt->second.emplace(y, DistanceMap());
        planeMapIt = newPlaneMapIt;
    }
    auto indexMapIt = planeMapIt->second.find(z);
    if (indexMapIt == planeMapIt->second.end()) {
        auto [newIndexMapIt, _added] = planeMapIt->second.emplace(z, nextIndex++);
        indexMapIt = newIndexMapIt;
    }
    return indexMapIt->second;
}

uint32_t UniqueIndices::getVertexIndex(const float v[], float epsilon) {
    return getIndex(std::round(v[0] / epsilon) * epsilon, std::round(v[1] / epsilon) * epsilon, std::round(v[2] / epsilon) * epsilon);
}

UniqueIndices::IndexMap UniqueIndices::createUniqueIndices(
        const float vertices[],
        uint32_t noOfVertices,
        float epsilon) {
    IndexMap indexMap;
    for (uint32_t index = 0; index < noOfVertices; ++index)
        indexMap.push_back(getVertexIndex(vertices + index*3, epsilon));
    return indexMap;
}
