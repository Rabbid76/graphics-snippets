#include "mesh_utility/constructiveSolidGeometry.h"
#include <algorithm>

using namespace csg;

namespace {
    PolygonIndices createTrianglePolygons(CSG &csg, const uint32_t *indices, uint32_t noOfIndices, uint32_t baseIndex) {
        PolygonIndices polygonIndices;
        polygonIndices.reserve(noOfIndices / 3);
        csg.polygons.reserve(csg.polygons.size() + noOfIndices / 3);
        for (uint32_t triangleIndex = 0; triangleIndex < noOfIndices; triangleIndex += 3) {
            if (indices[triangleIndex] == indices[triangleIndex + 1] ||
                indices[triangleIndex] == indices[triangleIndex + 2] ||
                indices[triangleIndex + 1] == indices[triangleIndex + 2]) {
                continue;
            }
            VertexIndices vertexIndices;
            for (uint32_t i = triangleIndex; i < triangleIndex + 3; ++i) {
                auto index = baseIndex + indices[i];
                vertexIndices.push_back({index, false});
            }
            auto lengthSq = squareLength(cross3(
                    sub3(csg.vertices[vertexIndices[1].index].vertex, csg.vertices[vertexIndices[0].index].vertex),
                    sub3(csg.vertices[vertexIndices[2].index].vertex, csg.vertices[vertexIndices[0].index].vertex)));
            if (lengthSq > 0)
                polygonIndices.push_back(csg.newPolygon(vertexIndices));
        }
        return polygonIndices;
    }

    PolygonIndices createQuadPolygons(CSG &csg, const uint32_t *indices, uint32_t noOfIndices, uint32_t baseIndex) {
        PolygonIndices polygonIndices;
        polygonIndices.reserve(noOfIndices / 4);
        csg.polygons.reserve(csg.polygons.size() + noOfIndices / 4);
        for (uint32_t quadIndex = 0; quadIndex < noOfIndices; quadIndex += 4) {
            VertexIndices vertexIndices;
            for (uint32_t i = quadIndex; i < quadIndex + 4; ++i) {
                auto index = baseIndex + indices[i];
                vertexIndices.push_back({index, false});
            }
            auto lengthSq = squareLength(cross3(
                    sub3(csg.vertices[vertexIndices[1].index].vertex, csg.vertices[vertexIndices[0].index].vertex),
                    sub3(csg.vertices[vertexIndices[2].index].vertex, csg.vertices[vertexIndices[0].index].vertex)));
            if (lengthSq > 0)
                polygonIndices.push_back(csg.newPolygon(vertexIndices));
        }
        return polygonIndices;
    }
}

Scalar Plane::epsilonScale = 1.0;
Scalar Plane::epsilon = PLANE_EPSILON;

Epsilon::Epsilon(Scalar scale) {
    Plane::epsilonScale = scale;
    Plane::epsilon = PLANE_EPSILON * scale;
}

Epsilon::~Epsilon() {
    Plane::epsilonScale = 1.0f;
    Plane::epsilon = PLANE_EPSILON;
}

PolygonIndices CSG::operatorUnion(PolygonIndices &polygonsA, PolygonIndices &polygonsB) {
    auto a = Node::constructNode(*this, polygonsA);
    auto b = Node::constructNode(*this, polygonsB);
    clipTo(a, b);
    clipTo(b, a);
    invert(b);
    clipTo(b, a);
    invert(b);
    build(a, allPolygons(b));
    return allPolygons(a);
}

PolygonIndices CSG::operatorSubtract(PolygonIndices &polygonsA, PolygonIndices &polygonsB) {
    auto a = Node::constructNode(*this, polygonsA);
    auto b = Node::constructNode(*this, polygonsB);
    invert(a);
    clipTo(a, b);
    clipTo(b, a);
    invert(b);
    clipTo(b, a);
    invert(b);
    build(a, allPolygons(b));
    invert(a);
    return allPolygons(a);
}

PolygonIndices CSG::operatorIntersect(PolygonIndices &polygonsA, PolygonIndices &polygonsB) {
    auto a = Node::constructNode(*this, polygonsA);
    auto b = Node::constructNode(*this, polygonsB);
    invert(a);
    clipTo(b, a);
    invert(b);
    clipTo(a, b);
    clipTo(b, a);
    build(a, allPolygons(b));
    invert(a);
    return allPolygons(a);
}

PolygonIndices csg::polygonsFromMeshSpecification(CSG &csg, const mesh::MeshDataReference &mesh) {

    uint32_t baseIndex = csg.vertices.size();
    csg.vertices.reserve(baseIndex + mesh.noOfVertices);
    for (uint32_t vertexIndex = 0; vertexIndex < mesh.noOfVertices; ++vertexIndex) {
        Vector3 v{mesh.vertices[vertexIndex*3], mesh.vertices[vertexIndex*3 + 1], mesh.vertices[vertexIndex*3 + 2]};
        Vector3 nv{0, 0, 0};
        if (mesh.normals)
            nv = {mesh.normals[vertexIndex*3], mesh.normals[vertexIndex*3 + 1], mesh.normals[vertexIndex*3 + 2]};
        Vector2 uv{0, 0};
        if (mesh.uvs)
            uv = {mesh.uvs[vertexIndex*2], mesh.uvs[vertexIndex*2 + 1]};
        csg.vertices.push_back({v, nv, uv});
    }
    return createTrianglePolygons(csg, mesh.indices, mesh.noOfIndices, baseIndex);
}

PolygonIndices csg::polygonsFromMeshSpecification(CSG &csg, const mesh::MeshDataReference &mesh, std::vector<uint32_t> setOfTriangles) {

    std::vector<int32_t> indexMap(mesh.noOfIndices, -1);
    std::vector<uint32_t> indices;
    indices.reserve(mesh.noOfIndices);
    csg.vertices.reserve(csg.vertices.size() + mesh.noOfVertices);
    for (uint32_t triangle : setOfTriangles) {
        for (uint32_t i = 0; i < 3; ++i) {
            uint32_t vertexIndex = *(mesh.indices + triangle * 3 + i);
            if (indexMap[vertexIndex] < 0) {
                indexMap[vertexIndex] = (int32_t)csg.vertices.size();
                Vector3 v{mesh.vertices[vertexIndex*3], mesh.vertices[vertexIndex*3 + 1], mesh.vertices[vertexIndex*3 + 2]};
                Vector3 nv{0, 0, 0};
                if (mesh.normals)
                    nv = {mesh.normals[vertexIndex*3], mesh.normals[vertexIndex*3 + 1], mesh.normals[vertexIndex*3 + 2]};
                Vector2 uv{0, 0};
                if (mesh.uvs)
                    uv = {mesh.uvs[vertexIndex*2], mesh.uvs[vertexIndex*2 + 1]};
                csg.vertices.push_back({v, nv, uv});
            }
            indices.push_back((uint32_t)indexMap[vertexIndex]);
        }
    }
    return createTrianglePolygons(csg, indices.data(), indices.size(), 0);
}

PolygonIndices csg::polygonsFromQuads(CSG &csg, const std::vector<uint32_t> &quadIndices, const std::vector<float> &vertices) {
    uint32_t baseIndex = csg.vertices.size();
    uint32_t noOfVertices = vertices.size() / 3;
    csg.vertices.reserve(baseIndex + noOfVertices);
    for (uint32_t vertexIndex = 0; vertexIndex < noOfVertices; ++vertexIndex) {
        Vector3 v{vertices[vertexIndex*3], vertices[vertexIndex*3 + 1], vertices[vertexIndex*3 + 2]};
        csg.vertices.push_back({v, {0, 0, 0}, {0, 0}});
    }
    return createQuadPolygons(csg, quadIndices.data(), quadIndices.size(), baseIndex);
}

void csg::polygonsToMesh(const CSG &csg, const PolygonIndices *polygonIndices, mesh::MeshDataInstance &mesh) {
    if (!polygonIndices)
        return;
    for (auto polygonIndex: *polygonIndices) {
        auto &polygon = csg.polygons[polygonIndex];
        uint32_t faceStartIndex = mesh.vertices.size() / 3;
        for (auto & vertexIndex: polygon.vertices) {
            auto &vertex = csg.vertices[vertexIndex.index];
            mesh.vertices.insert(mesh.vertices.end(), vertex.vertex.begin(), vertex.vertex.end());
            if (vertexIndex.inverted) {
                mesh.normals.insert(mesh.normals.end(), {(float)-vertex.normal[0], (float)-vertex.normal[1], (float)-vertex.normal[2]});
            } else {
                mesh.normals.insert(mesh.normals.end(), vertex.normal.begin(), vertex.normal.end());
            }
            mesh.uvs.insert(mesh.uvs.end(), vertex.uv.begin(), vertex.uv.end());
        }
        uint32_t faceEndIndex = mesh.vertices.size() / 3;
        if (faceEndIndex > faceStartIndex + 2) {
            for (uint32_t i = 0; i < faceEndIndex - faceStartIndex - 2; ++i) {
                mesh.indicesOut.push_back(faceStartIndex);
                mesh.indicesOut.push_back(faceStartIndex + i + 1);
                mesh.indicesOut.push_back(faceStartIndex + i + 2);
            }
        }
    }
}

std::vector<std::vector<float>> csg::polygonsToVertices(const CSG &csg, const PolygonIndices *polygonIndices) {
    if (!polygonIndices)
        return {};
    std::vector<std::vector<float>> polygons;
    for (auto polygonIndex: *polygonIndices) {
        auto &polygon = csg.polygons[polygonIndex];
        if (polygon.vertices.size() < 3)
            continue;
        polygons.emplace_back();
        for (auto & vertexIndex: polygon.vertices) {
            auto &vertex = csg.vertices[vertexIndex.index];
            polygons.back().insert(polygons.back().end(), vertex.vertex.begin(), vertex.vertex.end());
        }
    }
    return polygons;
}

PolygonIndices csg::meshOperation(CSG &csg, mesh::Operator op, const mesh::MeshDataReference &meshA, const mesh::MeshDataReference &meshB) {
    auto polygonsA = polygonsFromMeshSpecification(csg, meshA);
    auto polygonsB = polygonsFromMeshSpecification(csg, meshB);
    PolygonIndices resultPolygons;
    switch(op) {
        case mesh::Operator::OR: resultPolygons = csg.operatorUnion(polygonsA, polygonsB); break;
        case mesh::Operator::AND: resultPolygons = csg.operatorIntersect(polygonsA, polygonsB); break;
        case mesh::Operator::MINUS: resultPolygons = csg.operatorSubtract(polygonsA, polygonsB); break;
    }
    return resultPolygons;
}

void csg::meshOperation(mesh::Operator op, const mesh::MeshDataReference &meshA, const mesh::MeshDataReference &meshB, mesh::MeshDataInstance &resultMesh) {
    CSG csg;
    PolygonIndices resultPolygons = meshOperation(csg, op, meshA, meshB);
    polygonsToMesh(csg, &resultPolygons, resultMesh);
}

PolygonIndices csg::meshOperation(
        CSG &csg,
        mesh::Operator op, const
        mesh::MeshDataReference &meshA,
        const mesh::MeshDataReference &meshB,
        const std::vector<uint32_t> &trianglesA,
        const std::vector<uint32_t> &trianglesB) {
    auto polygonsA = polygonsFromMeshSpecification(csg, meshA, trianglesA);
    auto polygonsB = polygonsFromMeshSpecification(csg, meshB, trianglesB);
    PolygonIndices resultPolygons;
    switch(op) {
        case mesh::Operator::OR: resultPolygons = csg.operatorUnion(polygonsA, polygonsB); break;
        case mesh::Operator::AND: resultPolygons = csg.operatorIntersect(polygonsA, polygonsB); break;
        case mesh::Operator::MINUS: resultPolygons = csg.operatorSubtract(polygonsA, polygonsB); break;
    }
    return resultPolygons;
}

namespace {

    void createUniqueIndicesFromMesh(mesh::UniqueIndices &uniqueIndices, const mesh::MeshDataInstance &mesh) {
        for (uint32_t i = 0; i < mesh.vertices.size(); i += 3) {
            uniqueIndices.getVertexIndex(mesh.vertices.data() + i, mesh::INDEX_EPSILON * Plane::epsilonScale);
        }
    }

    void createUniqueIndicesFromMesh(mesh::UniqueIndices &uniqueIndices, const mesh::MeshDataReference &mesh, std::vector<uint32_t> &uniqueMeshIndices) {
        uniqueMeshIndices.reserve(mesh.noOfVertices);
        for (uint32_t i = 0; i < mesh.noOfVertices; ++i) {
            auto uniqueIndex = uniqueIndices.getVertexIndex(mesh.vertices + i * 3, mesh::INDEX_EPSILON * Plane::epsilonScale);
            uniqueMeshIndices.push_back(uniqueIndex);
        }
    }

    std::vector<uint8_t> getTrianglesToBeTested(const mesh::MeshDataReference &mesh, const std::vector<uint32_t> &triangles) {
        std::vector<uint8_t> trianglesToBeTested(mesh.noOfIndices / 3, 0);
        uint32_t nextInA = 0;
        for (uint32_t i = 0; i < mesh.noOfIndices / 3; ++i) {
            if (nextInA < triangles.size() && triangles[nextInA] == i)
                ++nextInA;
            else
                trianglesToBeTested[i] = 1;
        }
        return trianglesToBeTested;
    }

        void getTrianglesToBeAdded(
            uint32_t uniqueIndicesSize,
            uint32_t uniqueIntersectionIndicesSize,
            const mesh::MeshDataReference &mesh,
            std::vector<uint32_t> uniqueIndices,
            std::vector<uint8_t> &triangleStates) {

        std::vector<uint32_t> uniqueTriangles(triangleStates.size()*3);
        for (uint32_t i = 0; i < triangleStates.size(); ++i) {
            uniqueTriangles[i * 3] = uniqueIndices[mesh.indices[i * 3]];
            uniqueTriangles[i * 3 + 1] = uniqueIndices[mesh.indices[i * 3 + 1]];
            uniqueTriangles[i * 3 + 2] = uniqueIndices[mesh.indices[i * 3 + 2]];
        }

        std::vector<uint8_t> includedIndices(uniqueIndicesSize, 0);
        std::fill(includedIndices.begin(), includedIndices.begin() + (long)uniqueIntersectionIndicesSize, 1);

        bool newIndicesAdded = true;
        while (newIndicesAdded) {
            newIndicesAdded = false;
            for (uint32_t i = 0; i < triangleStates.size(); ++i) {
                if (triangleStates[i] != 1) {
                    continue;
                }
                uint32_t uti[] = {uniqueTriangles[i * 3], uniqueTriangles[i * 3 + 1], uniqueTriangles[i * 3 + 2]};
                auto inSum = includedIndices[uti[0]] + includedIndices[uti[1]] + includedIndices[uti[2]];
                if (inSum > 0) {
                    triangleStates[i] = 2;
                    if (inSum < 3) {
                        newIndicesAdded = true;
                        includedIndices[uti[0]] = 1;
                        includedIndices[uti[1]] = 1;
                        includedIndices[uti[2]] = 1;
                    }
                }
            }
        }
    }

    void addTriangles(
            mesh::MeshDataInstance &resultMesh,
            const mesh::MeshDataReference &mesh,
            const std::vector<uint8_t> &triangleStates,
            bool reverseWindingOrder) {
        std::vector<uint32_t> vertexIndicesA(mesh.noOfVertices, 0);
        for (uint32_t ti = 0; ti < triangleStates.size(); ++ti) {
            if (triangleStates[ti] != 2) {
                continue;
            }
            for (uint32_t j = ti*3; j < ti*3+3; ++j) {
                auto sourceIndex = mesh.indices[j];
                auto finalIndex = vertexIndicesA[sourceIndex];
                if (finalIndex == 0) {
                    finalIndex = resultMesh.vertices.size() / 3;
                    vertexIndicesA[sourceIndex] = finalIndex;
                    resultMesh.vertices.insert(resultMesh.vertices.end(), mesh.vertices + sourceIndex * 3, mesh.vertices + sourceIndex * 3 + 3);
                    if (reverseWindingOrder)
                        resultMesh.normals.insert(resultMesh.normals.end(), { -mesh.normals[sourceIndex * 3], -mesh.normals[sourceIndex * 3 + 1], -mesh.normals[sourceIndex * 3 + 2]});
                    else
                        resultMesh.normals.insert(resultMesh.normals.end(), mesh.normals + sourceIndex * 3, mesh.normals + sourceIndex * 3 + 3);
                    resultMesh.uvs.insert(resultMesh.uvs.end(), mesh.uvs + sourceIndex * 2, mesh.uvs + sourceIndex * 2 + 2);
                }
                resultMesh.indicesOut.push_back(finalIndex);
            }
            if (reverseWindingOrder) {
                std::swap(resultMesh.indicesOut[resultMesh.indicesOut.size() - 1], resultMesh.indicesOut[resultMesh.indicesOut.size() - 2]);
            }
        }
    }

    void addTriangles(
            mesh::MeshDataInstance &resultMesh,
            const mesh::MeshDataReference &mesh) {

        auto firstIndex = resultMesh.vertices.size();
        resultMesh.vertices.insert(resultMesh.vertices.end(), mesh.vertices, mesh.vertices + mesh.noOfVertices * 3);
        resultMesh.normals.insert(resultMesh.normals.end(), mesh.normals, mesh.normals + mesh.noOfVertices * 3);
        resultMesh.uvs.insert(resultMesh.uvs.end(), mesh.uvs, mesh.uvs + mesh.noOfVertices * 2);
        if (firstIndex == 0) {
            resultMesh.indicesOut.insert(resultMesh.indicesOut.end(), mesh.indices, mesh.indices + mesh.noOfIndices);
        } else {
            for (uint32_t i=0; i < mesh.noOfIndices; ++i) {
                resultMesh.indicesOut.push_back(mesh.indices[i] + firstIndex);
            }
        }
    }
}

void csg::meshOperation(
        mesh::Operator op,
        const mesh::MeshDataReference &meshA,
        const mesh::MeshDataReference &meshB,
        const std::vector<uint32_t> &trianglesA,
        const std::vector<uint32_t> &trianglesB,
        mesh::MeshDataInstance &resultMesh) {
    CSG csg;
    PolygonIndices resultPolygons = meshOperation(csg, op, meshA, meshB, trianglesA, trianglesB);
    polygonsToMesh(csg, &resultPolygons, resultMesh);
    if (resultMesh.vertices.empty())
    {
        if (op == mesh::Operator::MINUS) {
            addTriangles(resultMesh, meshA);
        } else if (op == mesh::Operator::OR) {
            addTriangles(resultMesh, meshA);
            addTriangles(resultMesh, meshB);
        }
        return;
    }

    mesh::UniqueIndices uniqueIndices;
    createUniqueIndicesFromMesh(uniqueIndices, resultMesh);
    uint32_t uniqueIndicesSize = uniqueIndices.getNextIndex();
    std::vector<uint32_t> uniqueIndicesA;
    createUniqueIndicesFromMesh(uniqueIndices, meshA, uniqueIndicesA);
    std::vector<uint32_t> uniqueIndicesB;
    createUniqueIndicesFromMesh(uniqueIndices, meshB, uniqueIndicesB);

    auto triangleStatesA = getTrianglesToBeTested(meshA, trianglesA);
    auto triangleStatesB = getTrianglesToBeTested(meshB, trianglesB);
    getTrianglesToBeAdded(uniqueIndices.getNextIndex(), uniqueIndicesSize, meshA, uniqueIndicesA, triangleStatesA);
    getTrianglesToBeAdded(uniqueIndices.getNextIndex(), uniqueIndicesSize, meshB, uniqueIndicesB, triangleStatesB);

    addTriangles(resultMesh, meshA, triangleStatesA, false);
    addTriangles(resultMesh, meshB, triangleStatesB, op == mesh::Operator::MINUS);
}