#include "mesh_utility/constructiveSolidGeometry.h"
#include "meshIntersectionTestHelper.h"
#include "catch2/catch.hpp"
#include <iostream>

using namespace csg;

std::tuple<bool, bool, bool, bool, bool> compareMeshData(
        const mesh::ResultMeshData &expectedMeshData,
        const mesh::ResultMeshData &actualMeshData) {

    bool verticesEqual = expectedMeshData.vertices.size() == actualMeshData.vertices.size();
    bool normalsEqual = expectedMeshData.normals.size() == actualMeshData.normals.size();
    bool uvsEqual = expectedMeshData.uvs.size() == actualMeshData.uvs.size();
    bool indicesOutEqual = expectedMeshData.indicesOut.size() == actualMeshData.indicesOut.size();
    bool indicesInEqual = expectedMeshData.indicesIn.size() == actualMeshData.indicesIn.size();
    for (uint32_t i = 0; verticesEqual && i < actualMeshData.vertices.size(); i++ )
        verticesEqual = std::fabs(actualMeshData.vertices[i] - expectedMeshData.vertices[i]) < 0.001f;
    for (uint32_t i = 0; normalsEqual && i < actualMeshData.normals.size(); i++ )
        normalsEqual = std::fabs(actualMeshData.normals[i] - expectedMeshData.normals[i]) < 0.001f;
    for (uint32_t i = 0; uvsEqual && i < actualMeshData.uvs.size(); i++ )
        uvsEqual = std::fabs(actualMeshData.uvs[i] - expectedMeshData.uvs[i]) < 0.001f;
    for (uint32_t i = 0; indicesOutEqual && i < actualMeshData.indicesOut.size(); i++ )
        indicesOutEqual = actualMeshData.indicesOut[i] == expectedMeshData.indicesOut[i];
    for (uint32_t i = 0; indicesInEqual && i < actualMeshData.indicesIn.size(); i++ )
        indicesInEqual = actualMeshData.indicesIn[i] == expectedMeshData.indicesIn[i];

    if (!verticesEqual) {
        std::cout << "vertices (" << actualMeshData.vertices.size() << "): {";
        for (auto &vertex: actualMeshData.vertices)
            std::cout << vertex << ", ";
        std::cout << "};" << std::endl;
    }
    if (!normalsEqual) {
        std::cout << "normals (" << actualMeshData.normals.size() << "): {";
        for (auto &vertex: actualMeshData.normals)
            std::cout << vertex << ", ";
        std::cout << "};" << std::endl;
    }
    if (!uvsEqual) {
        std::cout << "uvs (" << actualMeshData.uvs.size() << "): {";
        for (auto &vertex: actualMeshData.uvs)
            std::cout << vertex << ", ";
        std::cout << "};" << std::endl;
    }
    if (!indicesOutEqual) {
        std::cout << "indices out (" << actualMeshData.indicesOut.size() << "): {";
        for (auto &index: actualMeshData.indicesOut)
            std::cout << index << ", ";
        std::cout << "};" << std::endl;
    }
    if (!indicesInEqual) {
        std::cout << "indices in (" << actualMeshData.indicesIn.size() << "): {";
        for (auto &index: actualMeshData.indicesIn)
            std::cout << index << ", ";
        std::cout << "};" << std::endl;
    }

    return {verticesEqual, normalsEqual, uvsEqual, indicesOutEqual, indicesInEqual};
}

Vertex getVertex(const CSG &csg, const Polygon &polygon, uint32_t i) {
    auto &v = csg.vertices[polygon.vertices[i].index];
    return polygon.vertices[i].inverted ? Vertex(v).flip() : v;
}

Vertex getVertex(const CSG &csg, uint32_t polygonIndex, uint32_t i) {
    return getVertex(csg, csg.polygons[polygonIndex], i);
}

TEST_CASE("CSG - vertex object", "[mesh intersection]") {
    SECTION("flip") {
        auto v = Vertex{Vector3{1, 2, 3}, Vector3{4, 5, 6}, Vector2{7, 8}};
        v.flip();
        REQUIRE(v.vertex== Vector3{1, 2, 3});
        REQUIRE(v.normal== Vector3{-4, -5, -6});
        REQUIRE(v.uv == Vector2{7, 8});
    }
    SECTION("lerp") {
        SECTION("0") {
            auto v1 = Vertex{Vector3{1, 2, 3}, Vector3{4, 5, 6}, Vector2{7, 8}};
            auto v2 = Vertex{Vector3{-1, -2, -3}, Vector3{-4, -5, -6}, Vector2{-7, -8}};
            auto v = Vertex::interpolate(v1, v2, 0.5f);
            REQUIRE(v.vertex== Vector3{0, 0, 0});
            REQUIRE(v.normal== Vector3{0, 0, 0});
            REQUIRE(v.uv == Vector2{0, 0});
        }
        SECTION("1") {
            auto v1 = Vertex{Vector3{1, 2, 3}, Vector3{4, 5, 6}, Vector2{7, 8}};
            auto v2 = Vertex{Vector3{3, 4, 5}, Vector3{6, 7, 8}, Vector2{9, 10}};
            auto v = Vertex::interpolate(v1, v2, 0.5f);
            REQUIRE(v.vertex== Vector3{2, 3, 4});
            REQUIRE(v.normal== Vector3{5, 6, 7});
            REQUIRE(v.uv == Vector2{8, 9});
        }
    }
}

TEST_CASE("CSG - plan object", "[mesh intersection]") {
    SECTION("from points") {
        SECTION ("XY") {
            auto p = Plane::fromPoints(Vector3{0, 0, 0}, Vector3{1, 0, 0}, Vector3{0, 1, 0});
            REQUIRE(p.w == 0);
            REQUIRE(p.normal == Vector3{0, 0, 1});
        }
        SECTION ("XZ") {
            auto p = Plane::fromPoints(Vector3{0, 2, 0}, Vector3{1, 2, 0}, Vector3{0, 2, 1});
            REQUIRE(p.w == -2.0f);
            REQUIRE(p.normal == Vector3{0, -1, 0});
        }
        SECTION ("YZ") {
            auto p = Plane::fromPoints(Vector3{3, 0, 0}, Vector3{3, 1, 0}, Vector3{3, 0, 1});
            REQUIRE(p.w == 3.0f);
            REQUIRE(p.normal == Vector3{1, 0, 0});
        }
    }
    SECTION("flip") {
        SECTION ("XY") {
            auto p = Plane::fromPoints(Vector3{0, 0, 0}, Vector3{1, 0, 0}, Vector3{0, 1, 0});
            p.flip();
            REQUIRE(p.w == 0);
            REQUIRE(p.normal == Vector3{0, 0, -1});
        }
        SECTION ("XZ") {
            auto p = Plane::fromPoints(Vector3{0, 2, 0}, Vector3{1, 2, 0}, Vector3{0, 2, 1});
            p.flip();
            REQUIRE(p.w == 2.0f);
            REQUIRE(p.normal == Vector3{0, 1, 0});
        }
        SECTION ("YZ") {
            auto p = Plane::fromPoints(Vector3{3, 0, 0}, Vector3{3, 1, 0}, Vector3{3, 0, 1});
            p.flip();
            REQUIRE(p.w == -3.0f);
            REQUIRE(p.normal == Vector3{-1, 0, 0});
        }
    }
}

TEST_CASE("CSG - polygon object", "[mesh intersection]") {
    CSG csg;
    SECTION("construct") {
        csg.newVertex(Vertex{Vector3{0, 0, 2}, Vector3{0, 0, 1}, Vector2{0, 0}});
        csg.newVertex(Vertex{Vector3{3, 0, 2}, Vector3{0, 0, 1}, Vector2{1, 0}});
        csg.newVertex(Vertex{Vector3{3, 4, 2}, Vector3{0, 0, 1}, Vector2{1, 1}});
        auto polygonIndex = csg.newPolygon({{0, false}, {1, false}, {2, false}});
        auto &polygon = csg.polygons[polygonIndex];
        REQUIRE(getVertex(csg, polygon, 0).vertex == Vector3{0, 0, 2});
        REQUIRE(getVertex(csg, polygon, 1).vertex == Vector3{3, 0, 2});
        REQUIRE(getVertex(csg, polygon, 2).vertex == Vector3{3, 4, 2});
        REQUIRE(getVertex(csg, polygon, 0).normal == Vector3{0, 0, 1});
        REQUIRE(getVertex(csg, polygon, 1).normal == Vector3{0, 0, 1});
        REQUIRE(getVertex(csg, polygon, 2).normal == Vector3{0, 0, 1});
        REQUIRE(getVertex(csg, polygon, 0).uv == Vector2{0, 0});
        REQUIRE(getVertex(csg, polygon, 1).uv == Vector2{1, 0});
        REQUIRE(getVertex(csg, polygon, 2).uv == Vector2{1, 1});
        REQUIRE(polygon.plane.w == 2.0f);
        REQUIRE(polygon.plane.normal == Vector3{0, 0, 1});
    }
    SECTION("flip") {
        csg.newVertex(Vertex{Vector3{0, 0, 2}, Vector3{0, 0, 1}, Vector2{0, 0}});
        csg.newVertex(Vertex{Vector3{3, 0, 2}, Vector3{0, 0, 1}, Vector2{1, 0}});
        csg.newVertex(Vertex{Vector3{3, 4, 2}, Vector3{0, 0, 1}, Vector2{1, 1}});
        auto polygonIndex = csg.newPolygon({{0, false}, {1, false}, {2, false}});
        auto &polygon = csg.polygons[polygonIndex];
        polygon.flip();
        REQUIRE(getVertex(csg, polygon, 0).vertex == Vector3{3, 4, 2});
        REQUIRE(getVertex(csg, polygon, 1).vertex == Vector3{3, 0, 2});
        REQUIRE(getVertex(csg, polygon, 2).vertex == Vector3{0, 0, 2});
        REQUIRE(getVertex(csg, polygon, 0).normal == Vector3{0, 0, -1});
        REQUIRE(getVertex(csg, polygon, 1).normal == Vector3{0, 0, -1});
        REQUIRE(getVertex(csg, polygon, 2).normal == Vector3{0, 0, -1});
        REQUIRE(getVertex(csg, polygon, 0).uv == Vector2{1, 1});
        REQUIRE(getVertex(csg, polygon, 1).uv == Vector2{1, 0});
        REQUIRE(getVertex(csg, polygon, 2).uv == Vector2{0, 0});
        REQUIRE(polygon.plane.w == -2.0f);
        REQUIRE(polygon.plane.normal == Vector3{0, 0, -1});
    }
}

TEST_CASE("CSG - split polygon", "[mesh intersection]") {
    CSG csg;
    SECTION("coplanarFront") {
        csg.newVertex(Vertex{Vector3{0, 0, 0}, Vector3{0, 0, 1}, Vector2{0, 0}});
        csg.newVertex(Vertex{Vector3{3, 0, 0}, Vector3{0, 0, 1}, Vector2{1, 0}});
        csg.newVertex(Vertex{Vector3{3, 4, 0}, Vector3{0, 0, 1}, Vector2{1, 1}});
        auto polygonIndex = csg.newPolygon({{0, false}, {1, false}, {2, false}});
        auto plane = Plane::fromPoints(Vector3{0, 0, 0}, Vector3{1, 0, 0}, Vector3{0, 1, 0});
        PolygonIndices coplanarFront, coplanarBack, front, back;
        plane.splitPolygon(csg, polygonIndex, coplanarFront, coplanarBack, front, back);
        REQUIRE(coplanarFront.size() == 1);
        REQUIRE(coplanarBack.empty());
        REQUIRE(front.empty());
        REQUIRE(back.empty());
    }
    SECTION("coplanarBack") {
        csg.newVertex(Vertex{Vector3{0, 0, 0}, Vector3{0, 0, 1}, Vector2{0, 0}});
        csg.newVertex(Vertex{Vector3{3, 4, 0}, Vector3{0, 0, 1}, Vector2{1, 0}});
        csg.newVertex(Vertex{Vector3{3, 0, 0}, Vector3{0, 0, 1}, Vector2{1, 1}});
        auto polygonIndex = csg.newPolygon({{0, false}, {1, false}, {2, false}});
        auto plane = Plane::fromPoints(Vector3{0, 0, 0}, Vector3{1, 0, 0}, Vector3{0, 1, 0});
        PolygonIndices coplanarFront, coplanarBack, front, back;
        plane.splitPolygon(csg, polygonIndex, coplanarFront, coplanarBack, front, back);
        REQUIRE(coplanarFront.empty());
        REQUIRE(coplanarBack.size() == 1);
        REQUIRE(front.empty());
        REQUIRE(back.empty());
    }
    SECTION("front") {
        csg.newVertex(Vertex{Vector3{0, 0, 2}, Vector3{0, 0, 1}, Vector2{0, 0}});
        csg.newVertex(Vertex{Vector3{3, 0, 2}, Vector3{0, 0, 1}, Vector2{1, 0}});
        csg.newVertex(Vertex{Vector3{3, 4, 2}, Vector3{0, 0, 1}, Vector2{1, 1}});
        auto polygonIndex = csg.newPolygon({{0, false}, {1, false}, {2, false}});
        auto plane = Plane::fromPoints(Vector3{0, 0, 0}, Vector3{1, 0, 0}, Vector3{0, 1, 0});
        PolygonIndices coplanarFront, coplanarBack, front, back;
        plane.splitPolygon(csg, polygonIndex, coplanarFront, coplanarBack, front, back);
        REQUIRE(coplanarFront.empty());
        REQUIRE(coplanarBack.empty());
        REQUIRE(front.size() == 1);
        REQUIRE(back.empty());
    }
    SECTION("back") {
        csg.newVertex(Vertex{Vector3{0, 0, -2}, Vector3{0, 0, 1}, Vector2{0, 0}});
        csg.newVertex(Vertex{Vector3{3, 0, -2}, Vector3{0, 0, 1}, Vector2{1, 0}});
        csg.newVertex(Vertex{Vector3{3, 4, -2}, Vector3{0, 0, 1}, Vector2{1, 1}});
        auto polygonIndex = csg.newPolygon({{0, false}, {1, false}, {2, false}});
        auto plane = Plane::fromPoints(Vector3{0, 0, 0}, Vector3{1, 0, 0}, Vector3{0, 1, 0});
        PolygonIndices coplanarFront, coplanarBack, front, back;
        plane.splitPolygon(csg, polygonIndex, coplanarFront, coplanarBack, front, back);
        REQUIRE(coplanarFront.empty());
        REQUIRE(coplanarBack.empty());
        REQUIRE(front.empty());
        REQUIRE(back.size() == 1);
    }
    SECTION("spanning") {
        csg.newVertex(Vertex{Vector3{-2, -2, 0}, Vector3{0, 0, 1}, Vector2{0, 0}});
        csg.newVertex(Vertex{Vector3{2, 0, 0}, Vector3{0, 0, 1}, Vector2{1, 0}});
        csg.newVertex(Vertex{Vector3{2, 2, 0}, Vector3{0, 0, 1}, Vector2{1, 1}});
        auto polygonIndex = csg.newPolygon({{0, false}, {1, false}, {2, false}});
        auto plane = Plane::fromPoints(Vector3{0, 0, 0}, Vector3{1, 0, 0}, Vector3{0, 0, 1});
        PolygonIndices coplanarFront, coplanarBack, front, back;
        plane.splitPolygon(csg, polygonIndex, coplanarFront, coplanarBack, front, back);
        REQUIRE(coplanarFront.empty());
        REQUIRE(coplanarBack.empty());
        REQUIRE(front.size() == 1);
        REQUIRE(back.size() == 1);
    }
}

TEST_CASE("CSG - polygon from mesh", "[mesh intersection]") {
    CSG csg;
    std::vector<float> vertices1{-2, -2, 0, 2, -2, 0, 2, 2, 0, -2, 2, 0};
    std::vector<float> normals1{0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1};
    std::vector<float> uvs1{0, 0, 1, 0, 1, 1, 0, 1};
    std::vector<uint32_t>indices1{0, 1, 2, 0, 2, 3};
    auto mesh1 = mesh::MeshData::creatFromVectors(indices1, vertices1, normals1, uvs1, {0, 0, 0}, {0, 0, 0});
    auto polygonIndices1 = polygonsFromMeshSpecification(csg, mesh1);
    std::vector<float> vertices2{-1, 0, -1, 1, 0, -1, 0, 0, 1};
    std::vector<float> normals2{0, -1, 0, 0, -1, 0, 0, -1, 0};
    std::vector<float> uvs2{0, 0, 1, 0, 0.5, 1};
    std::vector<uint32_t>indices2{0, 1, 2};
    auto mesh2 = mesh::MeshData::creatFromVectors(indices2, vertices2, normals2, uvs2, {0, 0, 0}, {0, 0, 0});
    auto polygonIndices2 = polygonsFromMeshSpecification(csg, mesh2);
    REQUIRE(polygonIndices1.size() == 2);
    REQUIRE(csg.polygons[polygonIndices1[0]].plane.normal == Vector3{0, 0, 1});
    REQUIRE(csg.polygons[polygonIndices1[0]].plane.w == 0);
    REQUIRE(csg.polygons[polygonIndices1[0]].vertices.size() == 3);
    REQUIRE(getVertex(csg, polygonIndices1[0], 0).vertex == Vector3{-2, -2, 0});
    REQUIRE(getVertex(csg, polygonIndices1[0], 0).normal == Vector3{0, 0, 1});
    REQUIRE(getVertex(csg, polygonIndices1[0], 0).uv == Vector2{0, 0});
    REQUIRE(getVertex(csg, polygonIndices1[0], 1).vertex == Vector3{2, -2, 0});
    REQUIRE(getVertex(csg, polygonIndices1[0], 1).normal == Vector3{0, 0, 1});
    REQUIRE(getVertex(csg, polygonIndices1[0], 1).uv == Vector2{1, 0});
    REQUIRE(getVertex(csg, polygonIndices1[0], 2).vertex == Vector3{2, 2, 0});
    REQUIRE(getVertex(csg, polygonIndices1[0], 2).normal == Vector3{0, 0, 1});
    REQUIRE(getVertex(csg, polygonIndices1[0], 2).uv == Vector2{1, 1});
    REQUIRE(csg.polygons[polygonIndices1[1]].vertices.size() == 3);
    REQUIRE(csg.polygons[polygonIndices1[1]].plane.normal == Vector3{0, 0, 1});
    REQUIRE(csg.polygons[polygonIndices1[1]].plane.w == 0);
    REQUIRE(getVertex(csg, polygonIndices1[1], 0).vertex == Vector3{-2, -2, 0});
    REQUIRE(getVertex(csg, polygonIndices1[1], 0).normal == Vector3{0, 0, 1});
    REQUIRE(getVertex(csg, polygonIndices1[1], 0).uv == Vector2{0, 0});
    REQUIRE(getVertex(csg, polygonIndices1[1], 1).vertex == Vector3{2, 2, 0});
    REQUIRE(getVertex(csg, polygonIndices1[1], 1).normal == Vector3{0, 0, 1});
    REQUIRE(getVertex(csg, polygonIndices1[1], 1).uv == Vector2{1, 1});
    REQUIRE(getVertex(csg, polygonIndices1[1], 2).vertex == Vector3{-2, 2, 0});
    REQUIRE(getVertex(csg, polygonIndices1[1], 2).normal == Vector3{0, 0, 1});
    REQUIRE(getVertex(csg, polygonIndices1[1], 2).uv == Vector2{0, 1});
    REQUIRE(polygonIndices2.size() == 1);
    REQUIRE(csg.polygons[polygonIndices2[0]].plane.normal == Vector3{0, -1, 0});
    REQUIRE(csg.polygons[polygonIndices2[0]].plane.w == 0);
    REQUIRE(csg.polygons[polygonIndices2[0]].vertices.size() == 3);
    REQUIRE(getVertex(csg, polygonIndices2[0], 0).vertex == Vector3{-1, 0, -1});
    REQUIRE(getVertex(csg, polygonIndices2[0], 0).normal == Vector3{0, -1, 0});
    REQUIRE(getVertex(csg, polygonIndices2[0], 0).uv == Vector2{0, 0});
    REQUIRE(getVertex(csg, polygonIndices2[0], 1).vertex == Vector3{1, 0, -1});
    REQUIRE(getVertex(csg, polygonIndices2[0], 1).normal == Vector3{0, -1, 0});
    REQUIRE(getVertex(csg, polygonIndices2[0], 1).uv == Vector2{1, 0});
    REQUIRE(getVertex(csg, polygonIndices2[0], 2).vertex == Vector3{0, 0, 1});
    REQUIRE(getVertex(csg, polygonIndices2[0], 2).normal == Vector3{0, -1, 0});
    REQUIRE(getVertex(csg, polygonIndices2[0], 2).uv == Vector2{0.5, 1});
}

TEST_CASE("CSG - polygon to mesh", "[mesh intersection]") {
    CSG csg;
    csg.newVertex(Vertex{Vector3{-2, -2, 0}, Vector3{0, 0, 1}, Vector2{0, 0}});
    csg.newVertex(Vertex{Vector3{2, -2, 0}, Vector3{0, 0, 1}, Vector2{1, 0}});
    csg.newVertex(Vertex{Vector3{2, 2, 0}, Vector3{0, 0, 1}, Vector2{1, 1}});
    csg.newVertex(Vertex{Vector3{-2, 2, 0}, Vector3{0, 0, 1}, Vector2{0, 1}});
    auto polygonIndex = csg.newPolygon({{0, false}, {1, false}, {2, false}, {3, false}});
    mesh::ResultMeshData expectedResult;
    expectedResult.vertices = {-2, -2, 0, 2, -2, 0, 2, 2, 0, -2, 2, 0, };
    expectedResult.normals = {0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, };
    expectedResult.uvs = {0, 0, 1, 0, 1, 1, 0, 1, };
    expectedResult.indicesOut = {0, 1, 2, 0, 2, 3, };
    auto polygons = PolygonIndices{polygonIndex};
    mesh::ResultMeshData actualResult;
    polygonsToMesh(csg, &polygons, actualResult);
    REQUIRE(actualResult.vertices.size() % 3 == 0);
    REQUIRE(actualResult.indicesOut.size() % 3 == 0);
    REQUIRE(actualResult.indicesIn.size() % 3 == 0);
    auto [verticesEqual, normalsEqual, uvsEqual, indicesOuEqual, indicesInEqual] = compareMeshData(expectedResult, actualResult);
    REQUIRE(verticesEqual);
    REQUIRE(normalsEqual);
    REQUIRE(uvsEqual);
    REQUIRE(indicesOuEqual);
    REQUIRE(indicesInEqual);
}

TEST_CASE("CSG - csg", "[mesh intersection]") {
    std::vector<float> vertices1, vertices2;
    std::vector<float> normals1, normals2;
    std::vector<float> uvs1, uvs2;
    std::vector<uint32_t> indices1, indices2;
    mesh::Operator meshOperator = mesh::Operator::MINUS;
    mesh::ResultMeshData expectedResult;
    SECTION("cube and cube") {
        vertices1 = {0.5, 0.5, 0.5, 0.5, 0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, -0.5, 0.5, -0.5, -0.5, 0.5, 0.5, -0.5, -0.5, -0.5, -0.5, -0.5, 0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0.5, 0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0.5, -0.5, 0.5, -0.5, -0.5, -0.5, 0.5, -0.5, -0.5, -0.5, 0.5, 0.5, 0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0.5, -0.5, 0.5, 0.5, 0.5, -0.5, -0.5, 0.5, -0.5, 0.5, -0.5, -0.5, -0.5, -0.5, -0.5, };
        normals1 = {1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, };
        uvs1 = {0, 1, 1, 1, 0, 0, 1, 0, 0, 1, 1, 1, 0, 0, 1, 0, 0, 1, 1, 1, 0, 0, 1, 0, 0, 1, 1, 1, 0, 0, 1, 0, 0, 1, 1, 1, 0, 0, 1, 0, 0, 1, 1, 1, 0, 0, 1, 0, };
        indices1 = {0, 2, 1, 2, 3, 1, 4, 6, 5, 6, 7, 5, 8, 10, 9, 10, 11, 9, 12, 14, 13, 14, 15, 13, 16, 18, 17, 18, 19, 17, 20, 22, 21, 22, 23, 21, };
        vertices2 = {1, 1, 1, 1, 1, 0, 1, 0, 1, 1, 0, 0, 0, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 1, 0, 1, 0, 1, 1, 0, 0, 1, 1, 1, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 1, 1, 0, 1, 1, 1, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, };
        normals2 = {1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, };
        uvs2 = {0, 1, 1, 1, 0, 0, 1, 0, 0, 1, 1, 1, 0, 0, 1, 0, 0, 1, 1, 1, 0, 0, 1, 0, 0, 1, 1, 1, 0, 0, 1, 0, 0, 1, 1, 1, 0, 0, 1, 0, 0, 1, 1, 1, 0, 0, 1, 0, };
        indices2 = {0, 2, 1, 2, 3, 1, 4, 6, 5, 6, 7, 5, 8, 10, 9, 10, 11, 9, 12, 14, 13, 14, 15, 13, 16, 18, 17, 18, 19, 17, 20, 22, 21, 22, 23, 21, };
        SECTION("or") {
            meshOperator = mesh::Operator::OR;
            expectedResult.vertices = {0.5, 0, 0.5, 0.5, -0.5, 0.5, 0.5, 0, 0, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0, -0.5, 0.5, 0, 0, 0.5, 0, 0, 0.5, 0.5, -0.5, 0.5, 0.5, 0, 0.5, 0, -0.5, 0.5, 0.5, -0.5, 0.5, 0, 0, 1, 1, 1, 1, 0, 1, 1, 1, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, -0.5, 0.5, -0.5, -0.5, -0.5, -0.5, -0.5, 0.5, 0.5, -0.5, -0.5, -0.5, -0.5, -0.5, 0.5, -0.5, 0.5, 0.5, 0.5, 1, 0.5, 1, 1, 0, 0.5, 1, 0, 0.5, 1, 1, 1, 1, 1, 1, 1, 0, 0.5, 1, 0.5, -0.5, 0.5, -0.5, -0.5, 0.5, 0.5, 0, 0.5, 0, 0, 0.5, -0.5, -0.5, 0.5, 0.5, 0, 0.5, 0.5, 0, 0.5, 0, 0, 0.5, 0, 0.5, 0.5, -0.5, 0, 0.5, -0.5, 0.5, 0.5, 0, 0.5, 0.5, -0.5, 0, 0.5, 0, 0.5, 0, 0.5, 1, 0, 1, 0.5, 0, 1, 0.5, 0, 0, 1, 0, 0, 1, 0, 1, 0.5, 0, 0.5, 0, 1, 0, 0, 0.5, 0, 0, 0.5, 0.5, 0, 1, 1, 0, 0.5, 1, 0, 1, 1, 0, 0.5, 0.5, -0.5, -0.5, 0.5, -0.5, -0.5, -0.5, 0, -0.5, 0, 0, -0.5, 0.5, -0.5, -0.5, -0.5, 0, -0.5, -0.5, 0, -0.5, 0, 0, -0.5, 0, 0.5, -0.5, 0.5, 0, -0.5, 0.5, 0, -0.5, -0.5, 0.5, -0.5, -0.5, 0.5, -0.5, 0.5, 0, -0.5, 0, 0.5, 0.5, 1, 1, 1, 1, 0.5, 1, 1, 0.5, 0, 1, 1, 0, 1, 1, 1, 1, 0.5, 0.5, 1, 0, 1, 0, 0, 1, 1, 0.5, 1, 0.5, 0.5, 1, 0, 0, 1, 1, 0.5, 1, 1, 0.5, 1, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0, 0, 0.5, 0, 0.5, 0.5, -0.5, -0.5, 0.5, 0, -0.5, 0.5, 0, 0, 0.5, 0, -0.5, 0.5, 0.5, -0.5, 0.5, 0.5, 0, 0.5, 0, 0, 0.5, 1, 1, 0, 1, 0, 0, 0.5, 0.5, 0, 0.5, 1, 0, 1, 0, 0, 0.5, 0, 0, 0.5, 0.5, 0, 0, 1, 1, 0, 0.5, 1, 0.5, 0.5, 1, 0.5, 1, 1, 0, 0, 0.5, 0, 0, 1, 0, 0.5, 1, 0, 0.5, 0.5, 0, 0, -0.5, -0.5, 0.5, -0.5, 0, 0.5, -0.5, 0, -0.5, -0.5, -0.5, -0.5, -0.5, -0.5, 0.5, -0.5, 0, 0, -0.5, 0.5, 0, -0.5, 0.5, -0.5, -0.5, 0, 0, -0.5, 0.5, -0.5, -0.5, 0, -0.5, -0.5, 0, 0, -0.5, 0.5, 0.5, -0.5, 0.5, 0, -0.5, 0, 0, -0.5, 0, 0.5, -0.5, 0.5, 0.5, 0, 0, 1, 0, 0.5, 1, 0, 0, 0.5, 0, 0, 1, 0, 0.5, 0.5, 0, 0, 0, 1, 0, 0, 0.5, 0.5, 0, 0.5, 0.5, 0, 1, 0, 0.5, 1, 0, 0, 1, 0.5, 0.5, 1, 0, 0, 1, 0.5, 0, 1, 0.5, 0.5, 1, };
            expectedResult.normals = {1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, };
            expectedResult.uvs = {0, 0.5, 0, 0, 0.5, 0.5, 0, 0, 1, 0, 1, 0.5, 0.5, 0.5, 0.5, 0.5, 1, 1, 0.5, 1, 1, 0.5, 1, 1, 0.5, 0.5, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0.5, 0.5, 1, 1, 0.5, 1, 0.5, 0, 1, 0, 1, 1, 0.5, 0.5, 0, 1, 0, 0, 0.5, 0.5, 0.5, 1, 0, 0, 0.5, 0, 0.5, 0.5, 0.5, 0.5, 1, 1, 0.5, 1, 1, 0.5, 1, 1, 0.5, 0.5, 0.5, 0.5, 1, 1, 0.5, 1, 0.5, 0, 1, 0, 1, 1, 0.5, 0.5, 0, 1, 0, 0.5, 0.5, 0.5, 1, 1, 1, 0.5, 1, 1, 0.5, 0.5, 0, 1, 0, 0, 0.5, 0.5, 0.5, 1, 0, 0, 0.5, 0, 0.5, 0.5, 0.5, 0.5, 1, 1, 0.5, 1, 0.5, 0, 1, 0, 1, 1, 0.5, 0.5, 0.5, 0.5, 1, 1, 0.5, 1, 0.5, 0, 1, 0, 1, 1, 0.5, 0.5, 0, 1, 0, 0, 0.5, 0.5, 0.5, 1, 0, 0, 0.5, 0, 0.5, 0.5, 0, 1, 0, 0, 0.5, 0.5, 0.5, 1, 0, 0, 0.5, 0, 0.5, 0.5, 0.5, 0, 1, 0, 1, 0.5, 0.5, 0.5, 0, 1, 0, 0, 0.5, 0.5, 0.5, 1, 0, 0, 0.5, 0, 0.5, 0.5, 0, 1, 0, 0.5, 0.5, 0.5, 0.5, 1, 0.5, 0, 1, 0, 1, 0.5, 0.5, 0.5, 0.5, 0.5, 1, 1, 0.5, 1, 0.5, 0, 1, 0, 1, 1, 0.5, 0.5, 0, 0.5, 0, 0, 0.5, 0.5, 0, 0, 0.5, 0, 0.5, 0.5, 0, 1, 0, 0.5, 0.5, 0.5, 0.5, 1, 0.5, 0.5, 1, 1, 0.5, 1, 1, 0.5, 1, 1, 0.5, 0.5, 0, 1, 0, 0.5, 0.5, 0.5, 0.5, 1, 0, 0.5, 0, 0, 0.5, 0.5, 0, 0, 0.5, 0, 0.5, 0.5, };
            expectedResult.indicesOut = {0, 1, 2, 3, 4, 5, 3, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 28, 30, 31, 32, 33, 34, 32, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 48, 50, 51, 52, 53, 54, 52, 54, 55, 56, 57, 58, 59, 60, 61, 59, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 69, 71, 72, 73, 74, 75, 76, 77, 78, 76, 78, 79, 80, 81, 82, 80, 82, 83, 84, 85, 86, 87, 88, 89, 87, 89, 90, 91, 92, 93, 94, 95, 96, 94, 96, 97, 98, 99, 100, 98, 100, 101, 102, 103, 104, 105, 106, 107, 105, 107, 108, 109, 110, 111, 109, 111, 112, 113, 114, 115, 116, 117, 118, 116, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127, 128, 126, 128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 136, 138, 139, 140, 141, 142, 143, 144, 145, };
        }
        SECTION("and") {
            meshOperator = mesh::Operator::AND;
            expectedResult.vertices = {0.5, 0.5, 0, 0.5, 0.5, 0.5, 0.5, 0, 0.5, 0.5, 0, 0, 0, 0.5, 0, 0, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0, 0, 0.5, 0.5, 0, 0, 0.5, 0.5, 0.5, 0.5, 0, 0, 0.5, 0.5, 0, 0.5, 0.5, 0.5, 0.5, 0, 0.5, 0, 0, 0, 0, 0, 0.5, 0.5, 0, 0, 0, 0, 0, 0.5, 0, 0.5, 0.5, 0, 0, 0.5, 0, 0, 0, 0.5, 0, 0.5, 0, 0, 0, 0.5, 0, 0, 0.5, 0, 0.5, 0.5, 0, 0, 0, 0, 0, 0, 0.5, 0, 0.5, 0.5, 0, };
            expectedResult.normals = {1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, };
            expectedResult.uvs = {0.5, 1, 0, 1, 0, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0, 1, 0, 1, 0.5, 0.5, 1, 0.5, 0.5, 1, 1, 0.5, 0.5, 1, 0.5, 1, 1, 0, 0.5, 0, 0, 0.5, 0.5, 0, 0, 0.5, 0, 0.5, 0.5, 0, 0.5, 0, 0, 0.5, 0.5, 0, 0, 0.5, 0, 0.5, 0.5, 0.5, 0, 1, 0, 1, 0.5, 0.5, 0.5, };
            expectedResult.indicesOut = {0, 1, 2, 0, 2, 3, 4, 5, 6, 4, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 26, 28, 29, };
        }
        SECTION("minus") {
            meshOperator = mesh::Operator::MINUS;
            expectedResult.vertices = {0.5, 0, 0.5, 0.5, -0.5, 0.5, 0.5, 0, 0, 0.5, 0, 0, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0, -0.5, 0.5, 0.5, 0, 0.5, 0, 0, 0.5, 0.5, -0.5, 0.5, 0, 0, 0.5, 0, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, -0.5, -0.5, -0.5, -0.5, -0.5, 0.5, 0.5, -0.5, -0.5, -0.5, -0.5, -0.5, 0.5, -0.5, 0.5, 0.5, 0, 0.5, -0.5, -0.5, 0.5, -0.5, -0.5, 0.5, 0.5, 0, 0.5, 0, 0, 0.5, 0, -0.5, 0.5, 0.5, 0, 0.5, 0.5, 0, 0.5, -0.5, 0, 0.5, 0, 0.5, 0.5, -0.5, 0, 0.5, 0, 0.5, 0.5, 0, 0.5, 0.5, -0.5, 0, -0.5, 0.5, -0.5, -0.5, 0.5, -0.5, -0.5, -0.5, 0, -0.5, 0, 0, -0.5, 0, -0.5, -0.5, -0.5, 0, -0.5, -0.5, 0, -0.5, 0.5, 0, -0.5, 0, 0.5, -0.5, 0.5, 0, -0.5, 0, 0, -0.5, -0.5, 0.5, -0.5, -0.5, 0.5, -0.5, 0.5, 0, 0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0, 0, 0.5, 0, 0, 0.5, -0.5, -0.5, 0.5, 0, -0.5, 0.5, 0, 0, 0.5, 0, -0.5, 0.5, 0.5, -0.5, 0.5, 0.5, 0, 0.5, 0, 0.5, -0.5, 0, 0, -0.5, -0.5, 0.5, -0.5, 0, 0, -0.5, 0, -0.5, -0.5, -0.5, -0.5, -0.5, -0.5, 0.5, -0.5, 0.5, 0, -0.5, 0.5, -0.5, -0.5, 0, 0, -0.5, 0, 0, -0.5, 0.5, -0.5, -0.5, 0, -0.5, -0.5, 0, 0.5, -0.5, 0.5, 0.5, -0.5, 0.5, 0, -0.5, 0, 0, -0.5, 0, 0.5, 0.5, 0, 0, 0, 0, 0.5, 0, 0, 0.5, 0.5, 0, 0, 0.5, 0, 0, 0, 0.5, 0, 0.5, 0, 0, 0, 0, 0, 0.5, 0.5, 0, 0.5, 0.5, 0, 0, 0, 0, 0, 0.5, 0.5, 0, 0, 0.5, 0, 0, 0, 0, 0.5, 0, 0, };
            expectedResult.normals = {1, -0, -0, 1, 0, 0, 1, -0, -0, 1, -0, -0, 1, 0, 0, 1, 0, 0, 1, -0, -0, 1, -0, -0, 1, -0, -0, 1, 0, 0, 1, -0, -0, 1, -0, -0, 1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -0, 1, -0, 0, 1, 0, 0, 1, 0, -0, 1, -0, -0, 1, -0, 0, 1, 0, -0, 1, -0, -0, 1, -0, -0, 1, -0, 0, 1, 0, -0, 1, -0, -0, 1, -0, 0, 1, 0, -0, -1, -0, 0, -1, 0, 0, -1, 0, -0, -1, -0, -0, -1, -0, 0, -1, 0, -0, -1, -0, -0, -1, -0, -0, -1, -0, 0, -1, 0, -0, -1, -0, -0, -1, -0, 0, -1, 0, 0, -1, 0, -0, -0, 1, 0, 0, 1, 0, 0, 1, -0, -0, 1, -0, -0, 1, 0, 0, 1, -0, -0, 1, -0, -0, 1, -0, -0, 1, 0, 0, 1, -0, -0, 1, -0, -0, -1, -0, -0, -1, 0, 0, -1, -0, -0, -1, -0, -0, -1, 0, 0, -1, 0, 0, -1, -0, -0, -1, 0, 0, -1, -0, -0, -1, -0, -0, -1, 0, 0, -1, -0, -0, -1, -0, -0, -1, 0, 0, -1, -0, -0, -1, -0, -0, -1, 1, -0, -0, 1, -0, -0, 1, -0, -0, 1, -0, -0, 1, -0, -0, 1, -0, -0, -0, 1, -0, -0, 1, -0, -0, 1, -0, -0, 1, -0, -0, 1, -0, -0, 1, -0, -0, -0, 1, -0, -0, 1, -0, -0, 1, -0, -0, 1, };
            expectedResult.uvs = {0, 0.5, 0, 0, 0.5, 0.5, 0.5, 0.5, 0, 0, 1, 0, 1, 0.5, 0.5, 1, 0.5, 0.5, 1, 1, 0.5, 0.5, 1, 0.5, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0.5, 1, 0, 1, 0, 0, 0.5, 0.5, 0.5, 0.5, 0, 0, 0.5, 0, 0.5, 1, 0.5, 0.5, 1, 1, 0.5, 0.5, 1, 0.5, 1, 1, 0.5, 1, 0, 1, 0, 0, 0.5, 0.5, 0.5, 0.5, 0, 0, 0.5, 0, 0.5, 1, 0.5, 0.5, 1, 1, 0.5, 0.5, 0.5, 0, 1, 0, 1, 1, 0.5, 1, 0, 1, 0, 0, 0.5, 0.5, 0.5, 0.5, 0, 0, 0.5, 0, 0.5, 0.5, 0.5, 0, 1, 0, 1, 0.5, 0.5, 1, 0.5, 0.5, 1, 1, 0.5, 0.5, 0.5, 0, 1, 0, 1, 1, 0, 0.5, 0, 0, 0.5, 0.5, 0.5, 0.5, 0, 0, 0.5, 0, 0.5, 1, 0, 1, 0, 0.5, 0.5, 0.5, 0.5, 0.5, 0, 0, 0, 0.5, 0.5, 0.5, 0.5, 0, 0, 0, 0.5, 0.5, 0, 0, 0, 0.5, 0.5, 0.5, 0.5, 0, 0, 0, 0.5, 0.5, 1, 0.5, 1, 0, 0.5, 0, };
            expectedResult.indicesOut = {0, 1, 2, 3, 4, 5, 3, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 19, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 32, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 42, 44, 45, 46, 47, 48, 46, 48, 49, 50, 51, 52, 53, 54, 55, 53, 55, 56, 57, 58, 59, 60, 61, 62, 60, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 70, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 86, 88, 89, };
        }
    }
    SECTION("cube and cube planar") {
        vertices1 = {0.5, 0.5, 0.5, 0.5, 0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, -0.5, 0.5, -0.5, -0.5, 0.5, 0.5, -0.5, -0.5, -0.5, -0.5, -0.5, 0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0.5, 0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0.5, -0.5, 0.5, -0.5, -0.5, -0.5, 0.5, -0.5, -0.5, -0.5, 0.5, 0.5, 0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0.5, -0.5, 0.5, 0.5, 0.5, -0.5, -0.5, 0.5, -0.5, 0.5, -0.5, -0.5, -0.5, -0.5, -0.5, };
        normals1 = {1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, };
        uvs1 = {0, 1, 1, 1, 0, 0, 1, 0, 0, 1, 1, 1, 0, 0, 1, 0, 0, 1, 1, 1, 0, 0, 1, 0, 0, 1, 1, 1, 0, 0, 1, 0, 0, 1, 1, 1, 0, 0, 1, 0, 0, 1, 1, 1, 0, 0, 1, 0, };
        indices1 = {0, 2, 1, 2, 3, 1, 4, 6, 5, 6, 7, 5, 8, 10, 9, 10, 11, 9, 12, 14, 13, 14, 15, 13, 16, 18, 17, 18, 19, 17, 20, 22, 21, 22, 23, 21, };
        vertices2 = {1, 1, 0.5, 1, 1, -0.5, 1, 0, 0.5, 1, 0, -0.5, 0, 1, -0.5, 0, 1, 0.5, 0, 0, -0.5, 0, 0, 0.5, 0, 1, -0.5, 1, 1, -0.5, 0, 1, 0.5, 1, 1, 0.5, 0, 0, 0.5, 1, 0, 0.5, 0, 0, -0.5, 1, 0, -0.5, 0, 1, 0.5, 1, 1, 0.5, 0, 0, 0.5, 1, 0, 0.5, 1, 1, -0.5, 0, 1, -0.5, 1, 0, -0.5, 0, 0, -0.5, };
        normals2 = {1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, };
        uvs2 = {0, 1, 1, 1, 0, 0, 1, 0, 0, 1, 1, 1, 0, 0, 1, 0, 0, 1, 1, 1, 0, 0, 1, 0, 0, 1, 1, 1, 0, 0, 1, 0, 0, 1, 1, 1, 0, 0, 1, 0, 0, 1, 1, 1, 0, 0, 1, 0, };
        indices2 = {0, 2, 1, 2, 3, 1, 4, 6, 5, 6, 7, 5, 8, 10, 9, 10, 11, 9, 12, 14, 13, 14, 15, 13, 16, 18, 17, 18, 19, 17, 20, 22, 21, 22, 23, 21, };
        SECTION("or") {
            meshOperator = mesh::Operator::OR;
            expectedResult.vertices = {0.5, 0, 0.5, 0.5, -0.5, 0.5, 0.5, 0, 0, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0, -0.5, 0.5, 0, 0, 1, 1, 0.5, 1, 0, 0.5, 1, 1, -0.5, 1, 0, 0.5, 1, 0, -0.5, 1, 1, -0.5, -0.5, 0.5, -0.5, -0.5, -0.5, -0.5, -0.5, 0.5, 0.5, -0.5, -0.5, -0.5, -0.5, -0.5, 0.5, -0.5, 0.5, 0.5, 0.5, 1, 0, 1, 1, -0.5, 0.5, 1, -0.5, 0.5, 1, 0.5, 1, 1, 0.5, 1, 1, -0.5, 0.5, 1, 0, -0.5, 0.5, -0.5, -0.5, 0.5, 0.5, 0, 0.5, 0, 0, 0.5, -0.5, -0.5, 0.5, 0.5, 0, 0.5, 0.5, 0, 0.5, 0, 0.5, 0, 0, 1, 0, 0.5, 0.5, 0, 0.5, 0.5, 0, -0.5, 1, 0, -0.5, 1, 0, 0.5, 0.5, 0, 0, 0, 1, -0.5, 0, 0.5, -0.5, 0, 0.5, 0, 0, 1, 0.5, 0, 0.5, 0.5, 0, 1, 0.5, 0, 0.5, 0, -0.5, -0.5, 0.5, -0.5, -0.5, -0.5, 0, -0.5, 0, 0, -0.5, 0.5, -0.5, -0.5, -0.5, 0, -0.5, -0.5, 0, -0.5, 0, 0, -0.5, 0, 0.5, -0.5, 0.5, 0, -0.5, 0.5, 0, -0.5, -0.5, 0.5, -0.5, -0.5, 0.5, -0.5, 0.5, 0, -0.5, 0, 0.5, 0.5, 0.5, 1, 1, 0.5, 0.5, 1, 0.5, 0.5, 0, 0.5, 1, 0, 0.5, 1, 1, 0.5, 0.5, 0.5, 0.5, 0, 1, -0.5, 0, 1, 0.5, 0.5, 1, 0, 0.5, 1, -0.5, 0, 1, 0.5, 0.5, 1, 0.5, 0.5, 1, 0, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0, 0, 0.5, 0, 0.5, 0.5, -0.5, -0.5, 0.5, 0, -0.5, 0.5, 0, 0, 0.5, 0, -0.5, 0.5, 0.5, -0.5, 0.5, 0.5, 0, 0.5, 0, 0, 0.5, 0, 0, 0.5, 0.5, 0.5, 0.5, 0, 0.5, 0.5, 0.5, 0, 0.5, 0.5, 0.5, 0.5, 0, 0, 0.5, 1, 1, -0.5, 1, 0, -0.5, 0.5, 0.5, -0.5, 0.5, 1, -0.5, 1, 0, -0.5, 0.5, 0, -0.5, 0.5, 0.5, -0.5, 0, 1, 0.5, 0, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 1, 0.5, 0, 0, -0.5, -0.5, 0.5, -0.5, 0, 0.5, -0.5, 0, -0.5, -0.5, -0.5, -0.5, -0.5, -0.5, 0.5, -0.5, 0, 0, -0.5, 0.5, 0, -0.5, 0.5, -0.5, -0.5, 0, 0, -0.5, 0.5, -0.5, -0.5, 0, -0.5, -0.5, 0, 0, -0.5, 0.5, 0.5, -0.5, 0.5, 0, -0.5, 0, 0, -0.5, 0, 0.5, -0.5, 0.5, 0.5, -0.5, 0, 1, -0.5, 0.5, 1, -0.5, 0, 0.5, -0.5, 0, 1, -0.5, 0.5, 0.5, -0.5, };
            expectedResult.normals = {1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, };
            expectedResult.uvs = {0, 0.5, 0, 0, 0.5, 0.5, 0, 0, 1, 0, 1, 0.5, 0.5, 0.5, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0.5, 0.5, 1, 1, 0.5, 1, 0.5, 0, 1, 0, 1, 1, 0.5, 0.5, 0, 1, 0, 0, 0.5, 0.5, 0.5, 1, 0, 0, 0.5, 0, 0.5, 0.5, 0.5, 0.5, 1, 1, 0.5, 1, 0.5, 0, 1, 0, 1, 1, 0.5, 0.5, 0, 1, 0, 0.5, 0.5, 0.5, 1, 1, 1, 0.5, 1, 1, 0.5, 0.5, 0, 1, 0, 0, 0.5, 0.5, 0.5, 1, 0, 0, 0.5, 0, 0.5, 0.5, 0.5, 0.5, 1, 1, 0.5, 1, 0.5, 0, 1, 0, 1, 1, 0.5, 0.5, 0.5, 0.5, 1, 1, 0.5, 1, 0.5, 0, 1, 0, 1, 1, 0.5, 0.5, 0, 1, 0, 0, 0.5, 0.5, 0.5, 1, 0, 0, 0.5, 0, 0.5, 0.5, 0, 1, 0, 0, 0.5, 0.5, 0.5, 1, 0, 0, 0.5, 0, 0.5, 0.5, 0.5, 0, 1, 0, 1, 0.5, 0.5, 0.5, 0.5, 0.5, 1, 1, 0.5, 1, 1, 0.5, 1, 1, 0.5, 0.5, 0, 1, 0, 0, 0.5, 0.5, 0.5, 1, 0, 0, 0.5, 0, 0.5, 0.5, 0, 1, 0, 0.5, 0.5, 0.5, 0.5, 1, 0.5, 0.5, 1, 1, 0.5, 1, 0.5, 0, 1, 0, 1, 1, 0.5, 0.5, 0, 0.5, 0, 0, 0.5, 0.5, 0, 0, 0.5, 0, 0.5, 0.5, 0, 1, 0, 0.5, 0.5, 0.5, 0.5, 1, 0.5, 0.5, 1, 1, 0.5, 1, 1, 0.5, 1, 1, 0.5, 0.5, };
            expectedResult.indicesOut = {0, 1, 2, 3, 4, 5, 3, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 22, 24, 25, 26, 27, 28, 26, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 36, 38, 39, 40, 41, 42, 40, 42, 43, 44, 45, 46, 47, 48, 49, 47, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 57, 59, 60, 61, 62, 63, 64, 65, 66, 64, 66, 67, 68, 69, 70, 68, 70, 71, 72, 73, 74, 75, 76, 77, 75, 77, 78, 79, 80, 81, 82, 83, 84, 82, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 92, 94, 95, 96, 97, 98, 99, 100, 101, 99, 101, 102, 103, 104, 105, 106, 107, 108, 106, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 116, 118, 119, 120, 121, 122, 123, 124, 125, };
        }
        SECTION("and") {
            meshOperator = mesh::Operator::AND;
            expectedResult.vertices = {0.5, 0.5, 0.5, 0.5, 0, 0.5, 0.5, 0, 0, 0.5, 0.5, -0.5, 0.5, 0, 0, 0.5, 0, -0.5, 0.5, 0.5, -0.5, 0, 0.5, -0.5, 0, 0.5, 0, 0.5, 0.5, -0.5, 0, 0.5, 0, 0, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, -0.5, 0, 0.5, 0.5, 0, 0, 0.5, 0.5, 0.5, 0.5, 0, 0, 0.5, 0.5, 0, 0.5, 0.5, 0.5, 0.5, 0, 0.5, -0.5, 0.5, 0.5, -0.5, 0.5, 0, -0.5, 0, 0, -0.5, 0, 0.5, -0.5, 0, 0, -0.5, 0, 0.5, 0, 0, 0, -0.5, 0, 0, 0.5, 0, 0.5, 0.5, 0, 0.5, 0, 0, 0, 0.5, 0, 0, -0.5, 0.5, 0, 0, 0.5, 0, 0.5, 0, 0, -0.5, 0.5, 0, -0.5, 0.5, 0, 0, };
            expectedResult.normals = {1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, };
            expectedResult.uvs = {0, 1, 0, 0.5, 0.5, 0.5, 1, 1, 0.5, 0.5, 1, 0.5, 1, 1, 0.5, 1, 0.5, 0.5, 1, 1, 0.5, 0.5, 0.5, 0, 1, 0, 1, 1, 0.5, 1, 0.5, 0.5, 1, 1, 0.5, 0.5, 1, 0.5, 1, 1, 0.5, 1, 0, 1, 0, 0.5, 0.5, 0.5, 0, 0.5, 0, 0, 0.5, 0.5, 0, 0, 1, 0, 1, 0.5, 0.5, 0.5, 0, 1, 0, 0, 0.5, 0.5, 0.5, 1, 0, 0, 0.5, 0, 0.5, 0.5, };
            expectedResult.indicesOut = {0, 1, 2, 0, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 10, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 20, 22, 23, 24, 25, 26, 27, 28, 29, 27, 29, 30, 31, 32, 33, 31, 33, 34, 35, 36, 37, };
        }
        SECTION("minus") {
            meshOperator = mesh::Operator::MINUS;
            expectedResult.vertices = {0.5, 0, 0.5, 0.5, -0.5, 0.5, 0.5, 0, 0, 0.5, 0, 0, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0, -0.5, -0.5, 0.5, -0.5, -0.5, -0.5, -0.5, -0.5, 0.5, 0.5, -0.5, -0.5, -0.5, -0.5, -0.5, 0.5, -0.5, 0.5, 0.5, 0, 0.5, -0.5, -0.5, 0.5, -0.5, -0.5, 0.5, 0.5, 0, 0.5, 0, 0, 0.5, 0, -0.5, 0.5, 0.5, 0, 0.5, 0.5, 0, -0.5, 0.5, -0.5, -0.5, 0.5, -0.5, -0.5, -0.5, 0, -0.5, 0, 0, -0.5, 0, -0.5, -0.5, -0.5, 0, -0.5, -0.5, 0, -0.5, 0.5, 0, -0.5, 0, 0.5, -0.5, 0.5, 0, -0.5, 0, 0, -0.5, -0.5, 0.5, -0.5, -0.5, 0.5, -0.5, 0.5, 0, 0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0, 0, 0.5, 0, 0, 0.5, -0.5, -0.5, 0.5, 0, -0.5, 0.5, 0, 0, 0.5, 0, -0.5, 0.5, 0.5, -0.5, 0.5, 0.5, 0, 0.5, 0, 0.5, -0.5, 0, 0, -0.5, -0.5, 0.5, -0.5, 0, 0, -0.5, 0, -0.5, -0.5, -0.5, -0.5, -0.5, -0.5, 0.5, -0.5, 0.5, 0, -0.5, 0.5, -0.5, -0.5, 0, 0, -0.5, 0, 0, -0.5, 0.5, -0.5, -0.5, 0, -0.5, -0.5, 0, 0.5, 0, 0, 0, -0.5, 0, 0.5, -0.5, 0, 0.5, 0, 0, 0.5, 0.5, 0, 0, 0.5, 0, 0, -0.5, 0.5, 0, 0.5, 0.5, 0, 0, 0, 0, -0.5, 0, 0, 0.5, 0.5, 0, 0, 0.5, 0, -0.5, 0, 0, -0.5, };
            expectedResult.normals = {1, -0, -0, 1, 0, 0, 1, -0, -0, 1, -0, -0, 1, 0, 0, 1, 0, 0, 1, -0, -0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -0, 1, -0, 0, 1, 0, 0, 1, 0, -0, 1, -0, -0, 1, -0, 0, 1, 0, -0, 1, -0, -0, -1, -0, 0, -1, 0, 0, -1, 0, -0, -1, -0, -0, -1, -0, 0, -1, 0, -0, -1, -0, -0, -1, -0, -0, -1, -0, 0, -1, 0, -0, -1, -0, -0, -1, -0, 0, -1, 0, 0, -1, 0, -0, -0, 1, 0, 0, 1, 0, 0, 1, -0, -0, 1, -0, -0, 1, 0, 0, 1, -0, -0, 1, -0, -0, 1, -0, -0, 1, 0, 0, 1, -0, -0, 1, -0, -0, -1, -0, -0, -1, 0, 0, -1, -0, -0, -1, -0, -0, -1, 0, 0, -1, 0, 0, -1, -0, -0, -1, 0, 0, -1, -0, -0, -1, -0, -0, -1, 0, 0, -1, -0, -0, -1, 1, -0, -0, 1, -0, -0, 1, -0, -0, 1, -0, -0, 1, -0, -0, 1, -0, -0, 1, -0, -0, -0, 1, -0, -0, 1, -0, -0, 1, -0, -0, 1, -0, -0, 1, -0, -0, 1, -0, -0, 1, -0, };
            expectedResult.uvs = {0, 0.5, 0, 0, 0.5, 0.5, 0.5, 0.5, 0, 0, 1, 0, 1, 0.5, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0.5, 1, 0, 1, 0, 0, 0.5, 0.5, 0.5, 0.5, 0, 0, 0.5, 0, 0.5, 1, 0, 1, 0, 0, 0.5, 0.5, 0.5, 0.5, 0, 0, 0.5, 0, 0.5, 1, 0.5, 0.5, 1, 1, 0.5, 0.5, 0.5, 0, 1, 0, 1, 1, 0.5, 1, 0, 1, 0, 0, 0.5, 0.5, 0.5, 0.5, 0, 0, 0.5, 0, 0.5, 0.5, 0.5, 0, 1, 0, 1, 0.5, 0.5, 1, 0.5, 0.5, 1, 1, 0.5, 0.5, 0.5, 0, 1, 0, 1, 1, 0, 0.5, 0, 0, 0.5, 0.5, 0.5, 0.5, 0, 0, 0.5, 0, 0.5, 0.5, 0, 0, 0, 0.5, 0.5, 0.5, 1, 0.5, 1, 0, 0, 0, 0.5, 1, 0.5, 0.5, 0, 0, 0, 1, 0.5, 0.5, 0.5, 0, 0, 0, };
            expectedResult.indicesOut = {0, 1, 2, 3, 4, 5, 3, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 13, 15, 16, 17, 18, 19, 20, 21, 22, 20, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 30, 32, 33, 34, 35, 36, 34, 36, 37, 38, 39, 40, 41, 42, 43, 41, 43, 44, 45, 46, 47, 48, 49, 50, 48, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 61, 63, 64, 65, 66, 67, 65, 67, 68, 69, 70, 71, };
        }
    }
    auto meshA = mesh::MeshData::creatFromVectors(indices1, vertices1, normals1, uvs1, {0, 0, 0}, {0, 0, 0});
    auto meshB = mesh::MeshData::creatFromVectors(indices2, vertices2, normals2, uvs2, {0, 0, 0}, {0, 0, 0});
    mesh::ResultMeshData actualResult;
    csg::meshOperation(meshOperator, meshA, meshB, actualResult);
    REQUIRE(actualResult.vertices.size() % 3 == 0);
    REQUIRE(actualResult.indicesOut.size() % 3 == 0);
    REQUIRE(actualResult.indicesIn.size() % 3 == 0);
    auto [verticesEqual, normalsEqual, uvsEqual, indicesOuEqual, indicesInEqual] = compareMeshData(expectedResult, actualResult);
    REQUIRE(verticesEqual);
    REQUIRE(normalsEqual);
    REQUIRE(uvsEqual);
    REQUIRE(indicesOuEqual);
    REQUIRE(indicesInEqual);
}