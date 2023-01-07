#include "mesh_utility/constructiveSolidGeometry.h"
#include "meshIntersectionTestHelper.h"
#include "catch2/catch.hpp"
#include <iostream>

using namespace csg;

std::tuple<bool, bool, bool, bool, bool> compareMeshData(
        const ResultMeshData &expectedMeshData,
        const ResultMeshData &actualMeshData) {

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

TEST_CASE("CSG - vertex object", "[mesh intersection]") {
    SECTION("flip") {
        auto v = Vertex{Coordinates3{1, 2, 3}, Coordinates3{4, 5, 6}, Coordinates2{7, 8}};
        v.flip();
        REQUIRE(v.vertex== Coordinates3{1, 2, 3});
        REQUIRE(v.normal== Coordinates3{-4, -5, -6});
        REQUIRE(v.uv == Coordinates2{7, 8});
    }
    SECTION("lerp") {
        SECTION("0") {
            auto v1 = Vertex{Coordinates3{1, 2, 3}, Coordinates3{4, 5, 6}, Coordinates2{7, 8}};
            auto v2 = Vertex{Coordinates3{-1, -2, -3}, Coordinates3{-4, -5, -6}, Coordinates2{-7, -8}};
            auto v = Vertex::interpolate(v1, v2, 0.5f);
            REQUIRE(v.vertex== Coordinates3{0, 0, 0});
            REQUIRE(v.normal== Coordinates3{0, 0, 0});
            REQUIRE(v.uv == Coordinates2{0, 0});
        }
        SECTION("1") {
            auto v1 = Vertex{Coordinates3{1, 2, 3}, Coordinates3{4, 5, 6}, Coordinates2{7, 8}};
            auto v2 = Vertex{Coordinates3{3, 4, 5}, Coordinates3{6, 7, 8}, Coordinates2{9, 10}};
            auto v = Vertex::interpolate(v1, v2, 0.5f);
            REQUIRE(v.vertex== Coordinates3{2, 3, 4});
            REQUIRE(v.normal== Coordinates3{5, 6, 7});
            REQUIRE(v.uv == Coordinates2{8, 9});
        }
    }
}

TEST_CASE("CSG - plan object", "[mesh intersection]") {
    SECTION("from points") {
        SECTION ("XY") {
            auto p = Plane::fromPoints(Coordinates3{0, 0, 0}, Coordinates3{1, 0, 0}, Coordinates3{0, 1, 0});
            REQUIRE(p.w == 0);
            REQUIRE(p.normal == Coordinates3{0, 0, 1});
        }
        SECTION ("XZ") {
            auto p = Plane::fromPoints(Coordinates3{0, 2, 0}, Coordinates3{1, 2, 0}, Coordinates3{0, 2, 1});
            REQUIRE(p.w == -2.0f);
            REQUIRE(p.normal == Coordinates3{0, -1, 0});
        }
        SECTION ("YZ") {
            auto p = Plane::fromPoints(Coordinates3{3, 0, 0}, Coordinates3{3, 1, 0}, Coordinates3{3, 0, 1});
            REQUIRE(p.w == 3.0f);
            REQUIRE(p.normal == Coordinates3{1, 0, 0});
        }
    }
    SECTION("flip") {
        SECTION ("XY") {
            auto p = Plane::fromPoints(Coordinates3{0, 0, 0}, Coordinates3{1, 0, 0}, Coordinates3{0, 1, 0});
            p.flip();
            REQUIRE(p.w == 0);
            REQUIRE(p.normal == Coordinates3{0, 0, -1});
        }
        SECTION ("XZ") {
            auto p = Plane::fromPoints(Coordinates3{0, 2, 0}, Coordinates3{1, 2, 0}, Coordinates3{0, 2, 1});
            p.flip();
            REQUIRE(p.w == 2.0f);
            REQUIRE(p.normal == Coordinates3{0, 1, 0});
        }
        SECTION ("YZ") {
            auto p = Plane::fromPoints(Coordinates3{3, 0, 0}, Coordinates3{3, 1, 0}, Coordinates3{3, 0, 1});
            p.flip();
            REQUIRE(p.w == -3.0f);
            REQUIRE(p.normal == Coordinates3{-1, 0, 0});
        }
    }
}

TEST_CASE("CSG - polygon object", "[mesh intersection]") {
    SECTION("construct") {
        auto polygon = Polygon::newPolygon({
            Vertex{Coordinates3{0, 0, 2}, Coordinates3{0, 0, 1}, Coordinates2{0, 0}},
            Vertex{Coordinates3{3, 0, 2}, Coordinates3{0, 0, 1}, Coordinates2{1, 0}},
            Vertex{Coordinates3{3, 4, 2}, Coordinates3{0, 0, 1}, Coordinates2{1, 1}}
        });
        REQUIRE(polygon->vertices[0].vertex == Coordinates3{0, 0, 2});
        REQUIRE(polygon->vertices[1].vertex == Coordinates3{3, 0, 2});
        REQUIRE(polygon->vertices[2].vertex == Coordinates3{3, 4, 2});
        REQUIRE(polygon->vertices[0].normal == Coordinates3{0, 0, 1});
        REQUIRE(polygon->vertices[1].normal == Coordinates3{0, 0, 1});
        REQUIRE(polygon->vertices[2].normal == Coordinates3{0, 0, 1});
        REQUIRE(polygon->vertices[0].uv == Coordinates2{0, 0});
        REQUIRE(polygon->vertices[1].uv == Coordinates2{1, 0});
        REQUIRE(polygon->vertices[2].uv == Coordinates2{1, 1});
        REQUIRE(polygon->plane.w == 2.0f);
        REQUIRE(polygon->plane.normal == Coordinates3{0, 0, 1});
        delete polygon;
    }
    SECTION("flip") {
        auto polygon = Polygon::newPolygon({
               Vertex{Coordinates3{0, 0, 2}, Coordinates3{0, 0, 1}, Coordinates2{0, 0}},
               Vertex{Coordinates3{3, 0, 2}, Coordinates3{0, 0, 1}, Coordinates2{1, 0}},
               Vertex{Coordinates3{3, 4, 2}, Coordinates3{0, 0, 1}, Coordinates2{1, 1}}
        });
        polygon->flip();
        REQUIRE(polygon->vertices[0].vertex == Coordinates3{3, 4, 2});
        REQUIRE(polygon->vertices[1].vertex == Coordinates3{3, 0, 2});
        REQUIRE(polygon->vertices[2].vertex == Coordinates3{0, 0, 2});
        REQUIRE(polygon->vertices[0].normal == Coordinates3{0, 0, -1});
        REQUIRE(polygon->vertices[1].normal == Coordinates3{0, 0, -1});
        REQUIRE(polygon->vertices[2].normal == Coordinates3{0, 0, -1});
        REQUIRE(polygon->vertices[0].uv == Coordinates2{1, 1});
        REQUIRE(polygon->vertices[1].uv == Coordinates2{1, 0});
        REQUIRE(polygon->vertices[2].uv == Coordinates2{0, 0});
        REQUIRE(polygon->plane.w == -2.0f);
        REQUIRE(polygon->plane.normal == Coordinates3{0, 0, -1});
        delete polygon;
    }
}

TEST_CASE("CSG - split polygon", "[mesh intersection]") {
    SECTION("coplanarFront") {
        auto polygon = PolygonPtr(Polygon::newPolygon({
              Vertex{Coordinates3{0, 0, 0}, Coordinates3{0, 0, 1}, Coordinates2{0, 0}},
              Vertex{Coordinates3{3, 0, 0}, Coordinates3{0, 0, 1}, Coordinates2{1, 0}},
              Vertex{Coordinates3{3, 4, 0}, Coordinates3{0, 0, 1}, Coordinates2{1, 1}}
        }));
        auto plane = Plane::fromPoints(Coordinates3{0, 0, 0}, Coordinates3{1, 0, 0}, Coordinates3{0, 1, 0});
        Polygons coplanarFront, coplanarBack, front, back;
        plane.splitPolygon(polygon, coplanarFront, coplanarBack, front, back);
        REQUIRE(coplanarFront.size() == 1);
        REQUIRE(coplanarBack.size() == 0);
        REQUIRE(front.size() == 0);
        REQUIRE(back.size() == 0);
    }
    SECTION("coplanarBack") {
        auto polygon = PolygonPtr(Polygon::newPolygon({
              Vertex{Coordinates3{0, 0, 0}, Coordinates3{0, 0, 1}, Coordinates2{0, 0}},
              Vertex{Coordinates3{3, 4, 0}, Coordinates3{0, 0, 1}, Coordinates2{1, 0}},
              Vertex{Coordinates3{3, 0, 0}, Coordinates3{0, 0, 1}, Coordinates2{1, 1}}
        }));
        auto plane = Plane::fromPoints(Coordinates3{0, 0, 0}, Coordinates3{1, 0, 0}, Coordinates3{0, 1, 0});
        Polygons coplanarFront, coplanarBack, front, back;
        plane.splitPolygon(polygon, coplanarFront, coplanarBack, front, back);
        REQUIRE(coplanarFront.size() == 0);
        REQUIRE(coplanarBack.size() == 1);
        REQUIRE(front.size() == 0);
        REQUIRE(back.size() == 0);
    }
    SECTION("front") {
        auto polygon = PolygonPtr(Polygon::newPolygon({
              Vertex{Coordinates3{0, 0, 2}, Coordinates3{0, 0, 1}, Coordinates2{0, 0}},
              Vertex{Coordinates3{3, 0, 2}, Coordinates3{0, 0, 1}, Coordinates2{1, 0}},
              Vertex{Coordinates3{3, 4, 2}, Coordinates3{0, 0, 1}, Coordinates2{1, 1}}
        }));
        auto plane = Plane::fromPoints(Coordinates3{0, 0, 0}, Coordinates3{1, 0, 0}, Coordinates3{0, 1, 0});
        Polygons coplanarFront, coplanarBack, front, back;
        plane.splitPolygon(polygon, coplanarFront, coplanarBack, front, back);
        REQUIRE(coplanarFront.size() == 0);
        REQUIRE(coplanarBack.size() == 0);
        REQUIRE(front.size() == 1);
        REQUIRE(back.size() == 0);
    }
    SECTION("back") {
        auto polygon = PolygonPtr(Polygon::newPolygon({
              Vertex{Coordinates3{0, 0, -2}, Coordinates3{0, 0, 1}, Coordinates2{0, 0}},
              Vertex{Coordinates3{3, 0, -2}, Coordinates3{0, 0, 1}, Coordinates2{1, 0}},
              Vertex{Coordinates3{3, 4, -2}, Coordinates3{0, 0, 1}, Coordinates2{1, 1}}
        }));
        auto plane = Plane::fromPoints(Coordinates3{0, 0, 0}, Coordinates3{1, 0, 0}, Coordinates3{0, 1, 0});
        Polygons coplanarFront, coplanarBack, front, back;
        plane.splitPolygon(polygon, coplanarFront, coplanarBack, front, back);
        REQUIRE(coplanarFront.size() == 0);
        REQUIRE(coplanarBack.size() == 0);
        REQUIRE(front.size() == 0);
        REQUIRE(back.size() == 1);
    }
    SECTION("spanning") {
        auto polygon = PolygonPtr(Polygon::newPolygon({
              Vertex{Coordinates3{-2, -2, 0}, Coordinates3{0, 0, 1}, Coordinates2{0, 0}},
              Vertex{Coordinates3{2, 0, 0}, Coordinates3{0, 0, 1}, Coordinates2{1, 0}},
              Vertex{Coordinates3{2, 2, 0}, Coordinates3{0, 0, 1}, Coordinates2{1, 1}}
        }));
        auto plane = Plane::fromPoints(Coordinates3{0, 0, 0}, Coordinates3{1, 0, 0}, Coordinates3{0, 0, 1});
        Polygons coplanarFront, coplanarBack, front, back;
        plane.splitPolygon(polygon, coplanarFront, coplanarBack, front, back);
        REQUIRE(coplanarFront.size() == 0);
        REQUIRE(coplanarBack.size() == 0);
        REQUIRE(front.size() == 1);
        REQUIRE(back.size() == 1);
    }
}

TEST_CASE("CSG - polygon from mesh", "[mesh intersection]") {
    std::vector<float> vertices{-2, -2, 0, 2, -2, 0, 2, 2, 0, -2, 2, 0};
    std::vector<float> normals{0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1};
    std::vector<float> uvs{0, 0, 1, 0, 1, 1, 0, 1};
    std::vector<uint32_t>indices{0, 1, 2, 0, 2, 3};
    auto mesh = mesh::MeshData::creatFromVectors(indices, vertices, normals, uvs, {0, 0, 0}, {0, 0, 0});
    auto polygonsPtr = polygonsFromMeshSpecification(mesh);
    REQUIRE(polygonsPtr != nullptr);
    auto &polygons = *polygonsPtr;
    REQUIRE(polygons.size() == 2);
    REQUIRE(polygons[0]->plane.normal == Coordinates3{0, 0, 1});
    REQUIRE(polygons[0]->plane.w == 0);
    REQUIRE(polygons[0]->vertices.size() == 3);
    REQUIRE(polygons[0]->vertices[0].vertex == Coordinates3{-2, -2, 0});
    REQUIRE(polygons[0]->vertices[0].normal == Coordinates3{0, 0, 1});
    REQUIRE(polygons[0]->vertices[0].uv == Coordinates2{0, 0});
    REQUIRE(polygons[0]->vertices[1].vertex == Coordinates3{2, -2, 0});
    REQUIRE(polygons[0]->vertices[1].normal == Coordinates3{0, 0, 1});
    REQUIRE(polygons[0]->vertices[1].uv == Coordinates2{1, 0});
    REQUIRE(polygons[0]->vertices[2].vertex == Coordinates3{2, 2, 0});
    REQUIRE(polygons[0]->vertices[2].normal == Coordinates3{0, 0, 1});
    REQUIRE(polygons[0]->vertices[2].uv == Coordinates2{1, 1});
    REQUIRE(polygons[1]->vertices.size() == 3);
    REQUIRE(polygons[1]->plane.normal == Coordinates3{0, 0, 1});
    REQUIRE(polygons[1]->plane.w == 0);
    REQUIRE(polygons[1]->vertices[0].vertex == Coordinates3{-2, -2, 0});
    REQUIRE(polygons[1]->vertices[0].normal == Coordinates3{0, 0, 1});
    REQUIRE(polygons[1]->vertices[0].uv == Coordinates2{0, 0});
    REQUIRE(polygons[1]->vertices[1].vertex == Coordinates3{2, 2, 0});
    REQUIRE(polygons[1]->vertices[1].normal == Coordinates3{0, 0, 1});
    REQUIRE(polygons[1]->vertices[1].uv == Coordinates2{1, 1});
    REQUIRE(polygons[1]->vertices[2].vertex == Coordinates3{-2, 2, 0});
    REQUIRE(polygons[1]->vertices[2].normal == Coordinates3{0, 0, 1});
    REQUIRE(polygons[1]->vertices[2].uv == Coordinates2{0, 1});
    delete polygonsPtr;
}

TEST_CASE("CSG - polygon to mesh", "[mesh intersection]") {
    auto polygon1 = PolygonPtr(Polygon::newPolygon({
          Vertex{Coordinates3{-2, -2, 0}, Coordinates3{0, 0, 1}, Coordinates2{0, 0}},
          Vertex{Coordinates3{2, -2, 0}, Coordinates3{0, 0, 1}, Coordinates2{1, 0}},
          Vertex{Coordinates3{2, 2, 0}, Coordinates3{0, 0, 1}, Coordinates2{1, 1}},
          Vertex{Coordinates3{-2, 2, 0}, Coordinates3{0, 0, 1}, Coordinates2{0, 1}}
    }));
    ResultMeshData expectedResult;
    expectedResult.vertices = {-2, -2, 0, 2, -2, 0, 2, 2, 0, -2, 2, 0, };
    expectedResult.normals = {0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, };
    expectedResult.uvs = {0, 0, 1, 0, 1, 1, 0, 1, };
    expectedResult.indicesOut = {0, 1, 2, 0, 2, 3, };
    auto polygons = Polygons{polygon1};
    ResultMeshData actualResult;
    polygonsToMesh(&polygons, actualResult);
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
    Operator meshOperator = Operator::MINUS;
    ResultMeshData expectedResult;
    SECTION("cube and cube") {
        vertices1 = {0.5, 0.5, 0.5, 0.5, 0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, -0.5, 0.5, -0.5, -0.5, 0.5, 0.5, -0.5, -0.5, -0.5, -0.5, -0.5, 0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0.5, 0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0.5, -0.5, 0.5, -0.5, -0.5, -0.5, 0.5, -0.5, -0.5, -0.5, 0.5, 0.5, 0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0.5, -0.5, 0.5, 0.5, 0.5, -0.5, -0.5, 0.5, -0.5, 0.5, -0.5, -0.5, -0.5, -0.5, -0.5, };
        normals1 = {1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, };
        uvs1 = {0, 1, 1, 1, 0, 0, 1, 0, 0, 1, 1, 1, 0, 0, 1, 0, 0, 1, 1, 1, 0, 0, 1, 0, 0, 1, 1, 1, 0, 0, 1, 0, 0, 1, 1, 1, 0, 0, 1, 0, 0, 1, 1, 1, 0, 0, 1, 0, };
        indices1 = {0, 2, 1, 2, 3, 1, 4, 6, 5, 6, 7, 5, 8, 10, 9, 10, 11, 9, 12, 14, 13, 14, 15, 13, 16, 18, 17, 18, 19, 17, 20, 22, 21, 22, 23, 21, };
        vertices2 = {1, 1, 1, 1, 1, 0, 1, 0, 1, 1, 0, 0, 0, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 1, 0, 1, 0, 1, 1, 0, 0, 1, 1, 1, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 1, 1, 0, 1, 1, 1, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, };
        normals2 = {1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, };
        uvs2 = {0, 1, 1, 1, 0, 0, 1, 0, 0, 1, 1, 1, 0, 0, 1, 0, 0, 1, 1, 1, 0, 0, 1, 0, 0, 1, 1, 1, 0, 0, 1, 0, 0, 1, 1, 1, 0, 0, 1, 0, 0, 1, 1, 1, 0, 0, 1, 0, };
        indices2 = {0, 2, 1, 2, 3, 1, 4, 6, 5, 6, 7, 5, 8, 10, 9, 10, 11, 9, 12, 14, 13, 14, 15, 13, 16, 18, 17, 18, 19, 17, 20, 22, 21, 22, 23, 21, };
        SECTION("and") {
            meshOperator = Operator::AND;
            expectedResult.vertices = {0.5, 0, 0.5, 0.5, -0.5, 0.5, 0.5, 0, 0, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0, -0.5, 0.5, 0, 0, 0.5, 0, 0, 0.5, 0.5, -0.5, 0.5, 0.5, 0, 0.5, 0, -0.5, 0.5, 0.5, -0.5, 0.5, 0, 0, 1, 1, 1, 1, 0, 1, 1, 1, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 0.5, 1, 0.5, 1, 1, 0, 0.5, 1, 0, 0.5, 1, 1, 1, 1, 1, 1, 1, 0, 0.5, 1, 0.5, 0.5, 0, 0.5, 1, 0, 1, 0.5, 0, 1, 0.5, 0, 0, 1, 0, 0, 1, 0, 1, 0.5, 0, 0.5, 0.5, 0.5, 1, 1, 1, 1, 0.5, 1, 1, 0.5, 0, 1, 1, 0, 1, 1, 1, 1, 0.5, 0.5, 1, 1, 1, 0, 1, 0, 0, 0.5, 0.5, 0, 0.5, 1, 0, 1, 0, 0, 0.5, 0, 0, 0.5, 0.5, 0, -0.5, 0.5, -0.5, -0.5, -0.5, -0.5, -0.5, 0.5, 0.5, -0.5, -0.5, -0.5, -0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, 0.5, -0.5, -0.5, 0.5, 0.5, 0, 0.5, 0, 0, 0.5, -0.5, -0.5, 0.5, 0.5, 0, 0.5, 0.5, 0, 0.5, 0, 0, 0.5, 0, 0.5, 0.5, -0.5, 0, 0.5, -0.5, 0.5, 0.5, 0, 0.5, 0.5, -0.5, 0, 0.5, 0, 0, 1, 0, 0, 0.5, 0, 0, 0.5, 0.5, 0, 1, 1, 0, 0.5, 1, 0, 1, 1, 0, 0.5, 0.5, 0, 1, 0, 0, 1, 1, 0.5, 1, 0.5, 0.5, 1, 0, 0, 1, 1, 0.5, 1, 1, 0.5, 1, 0.5, 0, 1, 1, 0, 0.5, 1, 0.5, 0.5, 1, 0.5, 1, 1, 0.5, 0.5, 0, 0, 1, 0, 0.5, 1, 0, 0, 0.5, 0, 0, 1, 0, 0.5, 0.5, 0, -0.5, -0.5, 0.5, -0.5, -0.5, -0.5, 0, -0.5, 0, 0, -0.5, 0.5, -0.5, -0.5, -0.5, 0, -0.5, -0.5, 0, -0.5, 0, 0, -0.5, 0, 0.5, -0.5, 0.5, 0, -0.5, 0.5, 0, -0.5, -0.5, 0.5, -0.5, -0.5, 0.5, -0.5, 0.5, 0, -0.5, 0, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0, 0, 0.5, 0, 0.5, 0.5, -0.5, -0.5, 0.5, 0, -0.5, 0.5, 0, 0, 0.5, 0, -0.5, 0.5, 0.5, -0.5, 0.5, 0.5, 0, 0.5, 0, 0, 0.5, 0, 0, 0.5, 0, 0, 1, 0, 0.5, 1, 0, 0.5, 0.5, 0, 0, 1, 0, 0, 0.5, 0.5, 0, 0.5, 0.5, 0, 1, 0, 0.5, 1, 0, 0, 1, 0.5, 0.5, 1, 0, 0, 1, 0.5, 0, 1, 0.5, 0.5, 1, 0, 0, -0.5, -0.5, 0.5, -0.5, 0, 0.5, -0.5, 0, -0.5, -0.5, -0.5, -0.5, -0.5, -0.5, 0.5, -0.5, 0, 0, -0.5, 0.5, 0, -0.5, 0.5, -0.5, -0.5, 0, 0, -0.5, 0.5, -0.5, -0.5, 0, -0.5, -0.5, 0, 0, -0.5, 0.5, 0.5, -0.5, 0.5, 0, -0.5, 0, 0, -0.5, 0, 0.5, -0.5, };
            expectedResult.normals = {1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, };
            expectedResult.uvs = {0, 0.5, 0, 0, 0.5, 0.5, 0, 0, 1, 0, 1, 0.5, 0.5, 0.5, 0.5, 0.5, 1, 1, 0.5, 1, 1, 0.5, 1, 1, 0.5, 0.5, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0.5, 0.5, 1, 1, 0.5, 1, 0.5, 0, 1, 0, 1, 1, 0.5, 0.5, 0.5, 0.5, 1, 1, 0.5, 1, 0.5, 0, 1, 0, 1, 1, 0.5, 0.5, 0.5, 0.5, 1, 1, 0.5, 1, 0.5, 0, 1, 0, 1, 1, 0.5, 0.5, 0, 1, 0, 0, 0.5, 0.5, 0.5, 1, 0, 0, 0.5, 0, 0.5, 0.5, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0, 1, 0, 0, 0.5, 0.5, 0.5, 1, 0, 0, 0.5, 0, 0.5, 0.5, 0.5, 0.5, 1, 1, 0.5, 1, 1, 0.5, 1, 1, 0.5, 0.5, 0, 1, 0, 0.5, 0.5, 0.5, 1, 1, 1, 0.5, 1, 1, 0.5, 0.5, 0, 1, 0, 0, 0.5, 0.5, 0.5, 1, 0, 0, 0.5, 0, 0.5, 0.5, 0, 1, 0, 0.5, 0.5, 0.5, 0.5, 1, 0.5, 0.5, 1, 1, 0.5, 1, 1, 0.5, 1, 1, 0.5, 0.5, 0, 1, 0, 0, 0.5, 0.5, 0.5, 1, 0, 0, 0.5, 0, 0.5, 0.5, 0.5, 0.5, 1, 1, 0.5, 1, 0.5, 0, 1, 0, 1, 1, 0.5, 0.5, 0, 1, 0, 0, 0.5, 0.5, 0.5, 1, 0, 0, 0.5, 0, 0.5, 0.5, 0.5, 0, 1, 0, 1, 0.5, 0.5, 0.5, 0.5, 0, 1, 0, 1, 0.5, 0.5, 0.5, 0, 1, 0, 0.5, 0.5, 0.5, 0.5, 1, 0, 0.5, 0, 0, 0.5, 0.5, 0, 0, 0.5, 0, 0.5, 0.5, 0.5, 0.5, 1, 1, 0.5, 1, 0.5, 0, 1, 0, 1, 1, 0.5, 0.5, 0, 0.5, 0, 0, 0.5, 0.5, 0, 0, 0.5, 0, 0.5, 0.5, 0, 1, 0, 0.5, 0.5, 0.5, 0.5, 1, };
            expectedResult.indicesOut = {0, 1, 2, 3, 4, 5, 3, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 22, 24, 25, 26, 27, 28, 29, 30, 31, 29, 31, 32, 33, 34, 35, 36, 37, 38, 36, 38, 39, 40, 41, 42, 40, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 53, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 66, 68, 69, 70, 71, 72, 73, 74, 75, 73, 75, 76, 77, 78, 79, 80, 81, 82, 80, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 90, 92, 93, 94, 95, 96, 97, 98, 99, 100, 101, 102, 100, 102, 103, 104, 105, 106, 104, 106, 107, 108, 109, 110, 111, 112, 113, 111, 113, 114, 115, 116, 117, 115, 117, 118, 119, 120, 121, 119, 121, 122, 123, 124, 125, 126, 127, 128, 129, 130, 131, 132, 133, 134, 132, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143, 144, 142, 144, 145, };
        }
        SECTION("or") {
            meshOperator = Operator::OR;
            expectedResult.vertices = {0.5, 0.5, 0, 0.5, 0.5, 0.5, 0.5, 0, 0.5, 0.5, 0, 0, 0, 0.5, 0, 0, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0, 0, 0.5, 0.5, 0, 0, 0.5, 0.5, 0.5, 0.5, 0, 0, 0.5, 0.5, 0, 0.5, 0.5, 0.5, 0.5, 0, 0.5, 0, 0, 0, 0, 0, 0.5, 0.5, 0, 0, 0, 0, 0, 0.5, 0, 0.5, 0.5, 0, 0, 0.5, 0, 0, 0, 0.5, 0, 0.5, 0, 0, 0, 0.5, 0, 0, 0.5, 0, 0.5, 0.5, 0, 0, 0, 0, 0, 0, 0.5, 0, 0.5, 0.5, 0, };
            expectedResult.normals = {1, -0, -0, 1, 0, 0, 1, -0, -0, 1, -0, -0, -0, 1, -0, -0, 1, -0, 0, 1, 0, -0, 1, -0, -0, -0, 1, -0, -0, 1, 0, 0, 1, -0, -0, 1, -0, -0, 1, 0, 0, 1, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, };
            expectedResult.uvs = {0.5, 1, 0, 1, 0, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0, 1, 0, 1, 0.5, 0.5, 1, 0.5, 0.5, 1, 1, 0.5, 0.5, 1, 0.5, 1, 1, 0, 0.5, 0, 0, 0.5, 0.5, 0, 0, 0.5, 0, 0.5, 0.5, 0, 0.5, 0, 0, 0.5, 0.5, 0, 0, 0.5, 0, 0.5, 0.5, 0.5, 0, 1, 0, 1, 0.5, 0.5, 0.5, };
            expectedResult.indicesOut = {0, 1, 2, 0, 2, 3, 4, 5, 6, 4, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 26, 28, 29, };
        }
        SECTION("minus") {
            meshOperator = Operator::MINUS;
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
        SECTION("and") {
            meshOperator = Operator::AND;
            expectedResult.vertices = {0.5, 0, 0.5, 0.5, -0.5, 0.5, 0.5, 0, 0, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0, -0.5, 0.5, 0, 0, 1, 1, 0.5, 1, 0, 0.5, 1, 1, -0.5, 1, 0, 0.5, 1, 0, -0.5, 1, 1, -0.5, 0.5, 1, 0, 1, 1, -0.5, 0.5, 1, -0.5, 0.5, 1, 0.5, 1, 1, 0.5, 1, 1, -0.5, 0.5, 1, 0, 0.5, 0, 0, 1, 0, 0.5, 0.5, 0, 0.5, 0.5, 0, -0.5, 1, 0, -0.5, 1, 0, 0.5, 0.5, 0, 0, 0.5, 0.5, 0.5, 1, 1, 0.5, 0.5, 1, 0.5, 0.5, 0, 0.5, 1, 0, 0.5, 1, 1, 0.5, 0.5, 0.5, 0.5, 1, 1, -0.5, 1, 0, -0.5, 0.5, 0.5, -0.5, 0.5, 1, -0.5, 1, 0, -0.5, 0.5, 0, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, -0.5, -0.5, -0.5, -0.5, -0.5, 0.5, 0.5, -0.5, -0.5, -0.5, -0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, 0.5, -0.5, -0.5, 0.5, 0.5, 0, 0.5, 0, 0, 0.5, -0.5, -0.5, 0.5, 0.5, 0, 0.5, 0.5, 0, 0.5, 0, 0, 1, -0.5, 0, 0.5, -0.5, 0, 0.5, 0, 0, 1, 0.5, 0, 0.5, 0.5, 0, 1, 0.5, 0, 0.5, 0, 0, 1, -0.5, 0, 1, 0.5, 0.5, 1, 0, 0.5, 1, -0.5, 0, 1, 0.5, 0.5, 1, 0.5, 0.5, 1, 0, 0, 1, 0.5, 0, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 1, 0.5, 0.5, 0.5, -0.5, 0, 1, -0.5, 0.5, 1, -0.5, 0, 0.5, -0.5, 0, 1, -0.5, 0.5, 0.5, -0.5, -0.5, -0.5, 0.5, -0.5, -0.5, -0.5, 0, -0.5, 0, 0, -0.5, 0.5, -0.5, -0.5, -0.5, 0, -0.5, -0.5, 0, -0.5, 0, 0, -0.5, 0, 0.5, -0.5, 0.5, 0, -0.5, 0.5, 0, -0.5, -0.5, 0.5, -0.5, -0.5, 0.5, -0.5, 0.5, 0, -0.5, 0, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0, 0, 0.5, 0, 0.5, 0.5, -0.5, -0.5, 0.5, 0, -0.5, 0.5, 0, 0, 0.5, 0, -0.5, 0.5, 0.5, -0.5, 0.5, 0.5, 0, 0.5, 0, 0, 0.5, 0, 0, 0.5, 0.5, 0.5, 0.5, 0, 0.5, 0.5, 0.5, 0, 0.5, 0.5, 0.5, 0.5, 0, 0, 0.5, 0, 0, -0.5, -0.5, 0.5, -0.5, 0, 0.5, -0.5, 0, -0.5, -0.5, -0.5, -0.5, -0.5, -0.5, 0.5, -0.5, 0, 0, -0.5, 0.5, 0, -0.5, 0.5, -0.5, -0.5, 0, 0, -0.5, 0.5, -0.5, -0.5, 0, -0.5, -0.5, 0, 0, -0.5, 0.5, 0.5, -0.5, 0.5, 0, -0.5, 0, 0, -0.5, 0, 0.5, -0.5, };
            expectedResult.normals = {1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, };
            expectedResult.uvs = {0, 0.5, 0, 0, 0.5, 0.5, 0, 0, 1, 0, 1, 0.5, 0.5, 0.5, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0.5, 0.5, 1, 1, 0.5, 1, 0.5, 0, 1, 0, 1, 1, 0.5, 0.5, 0.5, 0.5, 1, 1, 0.5, 1, 0.5, 0, 1, 0, 1, 1, 0.5, 0.5, 0.5, 0.5, 1, 1, 0.5, 1, 0.5, 0, 1, 0, 1, 1, 0.5, 0.5, 0, 1, 0, 0, 0.5, 0.5, 0.5, 1, 0, 0, 0.5, 0, 0.5, 0.5, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0, 1, 0, 0, 0.5, 0.5, 0.5, 1, 0, 0, 0.5, 0, 0.5, 0.5, 0, 1, 0, 0.5, 0.5, 0.5, 1, 1, 1, 0.5, 1, 1, 0.5, 0.5, 0, 1, 0, 0, 0.5, 0.5, 0.5, 1, 0, 0, 0.5, 0, 0.5, 0.5, 0, 1, 0, 0.5, 0.5, 0.5, 0.5, 1, 0.5, 0.5, 1, 1, 0.5, 1, 1, 0.5, 1, 1, 0.5, 0.5, 0, 1, 0, 0, 0.5, 0.5, 0.5, 1, 0, 0, 0.5, 0, 0.5, 0.5, 0.5, 0.5, 1, 1, 0.5, 1, 0.5, 0, 1, 0, 1, 1, 0.5, 0.5, 0, 1, 0, 0, 0.5, 0.5, 0.5, 1, 0, 0, 0.5, 0, 0.5, 0.5, 0.5, 0, 1, 0, 1, 0.5, 0.5, 0.5, 0.5, 0.5, 1, 1, 0.5, 1, 1, 0.5, 1, 1, 0.5, 0.5, 0.5, 0.5, 1, 1, 0.5, 1, 0.5, 0, 1, 0, 1, 1, 0.5, 0.5, 0, 0.5, 0, 0, 0.5, 0.5, 0, 0, 0.5, 0, 0.5, 0.5, 0, 1, 0, 0.5, 0.5, 0.5, 0.5, 1, };
            expectedResult.indicesOut = {0, 1, 2, 3, 4, 5, 3, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 16, 18, 19, 20, 21, 22, 23, 24, 25, 23, 25, 26, 27, 28, 29, 30, 31, 32, 30, 32, 33, 34, 35, 36, 34, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 47, 49, 50, 51, 52, 53, 54, 55, 56, 54, 56, 57, 58, 59, 60, 61, 62, 63, 61, 63, 64, 65, 66, 67, 68, 69, 70, 68, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 78, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 88, 90, 91, 92, 93, 94, 92, 94, 95, 96, 97, 98, 99, 100, 101, 99, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 112, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 122, 124, 125, };
        }
        SECTION("or") {
            meshOperator = Operator::OR;
            expectedResult.vertices = {0.5, 0.5, 0.5, 0.5, 0, 0.5, 0.5, 0, 0, 0.5, 0.5, -0.5, 0.5, 0, 0, 0.5, 0, -0.5, 0.5, 0.5, -0.5, 0, 0.5, -0.5, 0, 0.5, 0, 0.5, 0.5, -0.5, 0, 0.5, 0, 0, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, -0.5, 0, 0.5, 0.5, 0, 0, 0.5, 0.5, 0.5, 0.5, 0, 0, 0.5, 0.5, 0, 0.5, 0.5, 0.5, 0.5, 0, 0.5, -0.5, 0.5, 0.5, -0.5, 0.5, 0, -0.5, 0, 0, -0.5, 0, 0.5, -0.5, 0, 0, -0.5, 0, 0.5, 0, 0, 0, -0.5, 0, 0, 0.5, 0, 0.5, 0.5, 0, 0.5, 0, 0, 0, 0.5, 0, 0, -0.5, 0.5, 0, 0, 0.5, 0, 0.5, 0, 0, -0.5, 0.5, 0, -0.5, 0.5, 0, 0, };
            expectedResult.normals = {1, 0, 0, 1, -0, -0, 1, -0, -0, 1, 0, 0, 1, -0, -0, 1, -0, -0, 1, 0, 0, -0, 1, -0, -0, 1, -0, 0, 1, 0, -0, 1, -0, -0, 1, -0, 0, 1, 0, 0, 1, 0, -0, -0, 1, -0, -0, 1, 0, 0, 1, -0, -0, 1, -0, -0, 1, 0, 0, 1, -0, -0, -1, 0, 0, -1, -0, -0, -1, -0, -0, -1, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, };
            expectedResult.uvs = {0, 1, 0, 0.5, 0.5, 0.5, 1, 1, 0.5, 0.5, 1, 0.5, 1, 1, 0.5, 1, 0.5, 0.5, 1, 1, 0.5, 0.5, 0.5, 0, 1, 0, 1, 1, 0.5, 1, 0.5, 0.5, 1, 1, 0.5, 0.5, 1, 0.5, 1, 1, 0.5, 1, 0, 1, 0, 0.5, 0.5, 0.5, 0, 0.5, 0, 0, 0.5, 0.5, 0, 0, 1, 0, 1, 0.5, 0.5, 0.5, 0, 1, 0, 0, 0.5, 0.5, 0.5, 1, 0, 0, 0.5, 0, 0.5, 0.5, };
            expectedResult.indicesOut = {0, 1, 2, 0, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 10, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 20, 22, 23, 24, 25, 26, 27, 28, 29, 27, 29, 30, 31, 32, 33, 31, 33, 34, 35, 36, 37, };
        }
        SECTION("minus") {
            meshOperator = Operator::MINUS;
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