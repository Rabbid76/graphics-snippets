#include "mesh_utility/constructiveSolidGeometry.h"

using namespace csg;

Scalar Plane::epsilon = PLANE_EPSILON;

Epsilon::Epsilon(Scalar scale) {
    Plane::epsilon = PLANE_EPSILON * scale;
}

Epsilon::~Epsilon() {
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

PolygonIndices csg::polygonsFromMeshSpecification(CSG &csg, const mesh::MeshData &mesh) {
    PolygonIndices polygonIndices;

    uint32_t baseIndex = csg.vertices.size();
    csg.vertices.reserve(baseIndex + mesh.noOfVertices);
    for (uint32_t vertexIndex = 0; vertexIndex < mesh.noOfIndices; ++vertexIndex) {
        Vector3 v{mesh.vertices[vertexIndex*3], mesh.vertices[vertexIndex*3 + 1], mesh.vertices[vertexIndex*3 + 2]};
        Vector3 nv{0, 0, 0};
        if (mesh.normals)
            nv = {mesh.normals[vertexIndex*3], mesh.normals[vertexIndex*3 + 1], mesh.normals[vertexIndex*3 + 2]};
        Vector2 uv{0, 0};
        if (mesh.uvs)
            uv = {mesh.uvs[vertexIndex*2], mesh.uvs[vertexIndex*2 + 1]};
        csg.vertices.push_back({v, nv, uv});
    }

    polygonIndices.reserve(mesh.noOfIndices / 3);
    csg.polygons.reserve(csg.polygons.size() + mesh.noOfIndices / 3);
    for (uint32_t triangleIndex = 0; triangleIndex < mesh.noOfIndices; triangleIndex += 3) {
        if (mesh.indices[triangleIndex] == mesh.indices[triangleIndex + 1] ||
            mesh.indices[triangleIndex] == mesh.indices[triangleIndex + 2] ||
            mesh.indices[triangleIndex + 1] == mesh.indices[triangleIndex + 2]) {
            continue;
        }
        VertexIndices vertexIndices;
        for (uint32_t i = triangleIndex; i < triangleIndex + 3; ++i) {
            auto index = baseIndex + mesh.indices[i];
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

void csg::polygonsToMesh(const CSG &csg, const PolygonIndices *polygonIndices, mesh::ResultMeshData &mesh) {
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

PolygonIndices csg::meshOperation(CSG &csg, mesh::Operator op, const mesh::MeshData &meshA, const mesh::MeshData &meshB) {
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

void csg::meshOperation(mesh::Operator op, const mesh::MeshData &meshA, const mesh::MeshData &meshB, mesh::ResultMeshData &resultMesh) {
    CSG csg;
    PolygonIndices resultPolygons = meshOperation(csg, op, meshA, meshB);
    polygonsToMesh(csg, &resultPolygons, resultMesh);
}