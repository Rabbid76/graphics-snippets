#include "mesh_utility/meshIntersection.h"
#include "catch2/catch.hpp"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <vector>

using namespace mesh;

namespace mesh {
    bool operator ==(const Coordinates3 &a, const Coordinates3 &b) {
        return
                std::fabs(a[0] - b[0]) < 0.001f &&
                std::fabs(a[1] - b[1]) < 0.001f &&
                std::fabs(a[2] - b[2]) < 0.001f;
    }
}

namespace {
    Point3 getMin(const std::vector<float> &vertices) {
        Point3 p{vertices[0], vertices[1], vertices[2]};
        for (uint32_t i = 3; i < vertices.size(); i += 3) {
            p[0] = std::min(p[0], vertices[i]);
            p[1] = std::min(p[1], vertices[i+1]);
            p[2] = std::min(p[2], vertices[i+2]);
        }
        return p;
    }

    Point3 getMax(const std::vector<float> &vertices) {
        Point3 p{vertices[0], vertices[1], vertices[2]};
        for (uint32_t i = 3; i < vertices.size(); i += 3) {
            p[0] = std::max(p[0], vertices[i]);
            p[1] = std::max(p[1], vertices[i+1]);
            p[2] = std::max(p[2], vertices[i+2]);
        }
        return p;
    }

    std::tuple<bool, bool, bool> compareMeshData(
            const ResultMeshData &expectedMeshData,
            const ResultMeshData &actualMeshData) {

        bool verticesEqual = expectedMeshData.vertices.size() == actualMeshData.vertices.size();
        bool indicesOutEqual = expectedMeshData.indicesOut.size() == actualMeshData.indicesOut.size();
        bool indicesInEqual = expectedMeshData.indicesIn.size() == actualMeshData.indicesIn.size();
        for (uint32_t i = 0; verticesEqual && i < actualMeshData.vertices.size(); i++ )
            verticesEqual = std::fabs(actualMeshData.vertices[i] - expectedMeshData.vertices[i]) < 0.001f;
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

        return {verticesEqual, indicesOutEqual, indicesInEqual};
    }
}

TEST_CASE("mesh intersection - add vectors", "[mesh intersection]") {
    SECTION("0") {
        REQUIRE(add3(Coordinates3{0, 0, 0}, Coordinates3{0, 0, 0}) == Coordinates3{0, 0, 0});
    }
    SECTION("1") {
        REQUIRE(add3(Coordinates3{1, 0, 0}, Coordinates3{0, 0, 0}) == Coordinates3{1, 0, 0});
        REQUIRE(add3(Coordinates3{0, 1, 0}, Coordinates3{0, 0, 0}) == Coordinates3{0, 1, 0});
        REQUIRE(add3(Coordinates3{0, 0, 1}, Coordinates3{0, 0, 0}) == Coordinates3{0, 0, 1});
        REQUIRE(add3(Coordinates3{0, 0, 0}, Coordinates3{1, 0, 0}) == Coordinates3{1, 0, 0});
        REQUIRE(add3(Coordinates3{0, 0, 0}, Coordinates3{0, 1, 0}) == Coordinates3{0, 1, 0});
        REQUIRE(add3(Coordinates3{0, 0, 0}, Coordinates3{0, 0, 1}) == Coordinates3{0, 0, 1});
    }
    SECTION("sum") {
        REQUIRE(add3(Coordinates3{1, 2, 3}, Coordinates3{1, 2, 3}) == Coordinates3{2, 4, 6});
    }
}

TEST_CASE("mesh intersection - subtract vectors", "[mesh intersection]") {
    SECTION("0") {
        REQUIRE(sub3(Coordinates3{0, 0, 0}, Coordinates3{0, 0, 0}) == Coordinates3{0, 0, 0});
        REQUIRE(sub3(Coordinates3{1, 2, 3}, Coordinates3{1, 2, 3}) == Coordinates3{0, 0, 0});
    }
    SECTION("1") {
        REQUIRE(sub3(Coordinates3{1, 0, 0}, Coordinates3{0, 0, 0}) == Coordinates3{1, 0, 0});
        REQUIRE(sub3(Coordinates3{0, 1, 0}, Coordinates3{0, 0, 0}) == Coordinates3{0, 1, 0});
        REQUIRE(sub3(Coordinates3{0, 0, 1}, Coordinates3{0, 0, 0}) == Coordinates3{0, 0, 1});
        REQUIRE(sub3(Coordinates3{0, 0, 0}, Coordinates3{1, 0, 0}) == Coordinates3{-1, 0, 0});
        REQUIRE(sub3(Coordinates3{0, 0, 0}, Coordinates3{0, 1, 0}) == Coordinates3{0, -1, 0});
        REQUIRE(sub3(Coordinates3{0, 0, 0}, Coordinates3{0, 0, 1}) == Coordinates3{0, 0, -1});
    }
    SECTION("sum") {
        REQUIRE(sub3(Coordinates3{2, 4, 6}, Coordinates3{1, 2, 3}) == Coordinates3{1, 2, 3});
    }
}

TEST_CASE("mesh intersection - multiply vector with scalar", "[mesh intersection]") {
    SECTION("0") {
        REQUIRE(multiply3scalar(Coordinates3{0, 0, 0}, 0) == Coordinates3{0, 0, 0});
        REQUIRE(multiply3scalar(Coordinates3{1, 2, 3}, 0) == Coordinates3{0, 0, 0});
        REQUIRE(multiply3scalar(Coordinates3{0, 0, 0}, 1) == Coordinates3{0, 0, 0});
    }SECTION("1") {
        REQUIRE(multiply3scalar(Coordinates3{1, 0, 0}, 1) == Coordinates3{1, 0, 0});
        REQUIRE(multiply3scalar(Coordinates3{0, 1, 0}, 1) == Coordinates3{0, 1, 0});
        REQUIRE(multiply3scalar(Coordinates3{0, 0, 1}, 1) == Coordinates3{0, 0, 1});
        REQUIRE(multiply3scalar(Coordinates3{1, 1, 1}, 1) == Coordinates3{1, 1, 1});
    }SECTION("multiply") {
        REQUIRE(multiply3scalar(Coordinates3{1, 2, 3}, 1) == Coordinates3{1, 2, 3});
        REQUIRE(multiply3scalar(Coordinates3{1, 2, 3}, 2) == Coordinates3{2, 4, 6});
    }
}

TEST_CASE("mesh intersection - dot product", "[mesh intersection]") {
    SECTION("0") {
        REQUIRE(dot3(Coordinates3{0, 0, 0}, Coordinates3{0, 0, 0}) == 0);
        REQUIRE(dot3(Coordinates3{1, 0, 0}, Coordinates3{0, 1, 0}) == 0);
        REQUIRE(dot3(Coordinates3{1, 0, 0}, Coordinates3{0, 0, 1}) == 0);
        REQUIRE(dot3(Coordinates3{0, 1, 0}, Coordinates3{1, 0, 0}) == 0);
        REQUIRE(dot3(Coordinates3{0, 1, 0}, Coordinates3{0, 0, 1}) == 0);
        REQUIRE(dot3(Coordinates3{0, 0, 1}, Coordinates3{1, 0, 0}) == 0);
        REQUIRE(dot3(Coordinates3{0, 0, 1}, Coordinates3{0, 1, 0}) == 0);
    }
    SECTION("1") {
        REQUIRE(dot3(Coordinates3{1, 0, 0}, Coordinates3{1, 0, 0}) == 1);
        REQUIRE(dot3(Coordinates3{0, 1, 0}, Coordinates3{0, 1, 0}) == 1);
        REQUIRE(dot3(Coordinates3{0, 0, 1}, Coordinates3{0, 0, 1}) == 1);
    }
    SECTION("sum") {
        REQUIRE(dot3(Coordinates3{1, 1, 1}, Coordinates3{1, 1, 1}) == 3);
        REQUIRE(dot3(Coordinates3{1, 2, 3}, Coordinates3{1, 2, 3}) == 14);
        REQUIRE(dot3(Coordinates3{3, 2, 1}, Coordinates3{1, 2, 3}) == 10);
    }
}

TEST_CASE("mesh intersection - cross product", "[mesh intersection]") {
    SECTION("0") {
        REQUIRE(cross3(Coordinates3{0, 0, 0}, Coordinates3{0, 0, 0}) == Coordinates3{0, 0, 0});
        REQUIRE(cross3(Coordinates3{1, 0, 0}, Coordinates3{1, 0, 0}) == Coordinates3{0, 0, 0});
        REQUIRE(cross3(Coordinates3{0, 1, 0}, Coordinates3{0, 1, 0}) == Coordinates3{0, 0, 0});
        REQUIRE(cross3(Coordinates3{0, 0, 1}, Coordinates3{0, 0, 1}) == Coordinates3{0, 0, 0});
        REQUIRE(cross3(Coordinates3{-1, 0, 0}, Coordinates3{1, 0, 0}) == Coordinates3{0, 0, 0});
        REQUIRE(cross3(Coordinates3{0, -1, 0}, Coordinates3{0, 1, 0}) == Coordinates3{0, 0, 0});
        REQUIRE(cross3(Coordinates3{0, 0, -1}, Coordinates3{0, 0, 1}) == Coordinates3{0, 0, 0});
        REQUIRE(cross3(Coordinates3{1, 0, 0}, Coordinates3{-1, 0, 0}) == Coordinates3{0, 0, 0});
        REQUIRE(cross3(Coordinates3{0, 1, 0}, Coordinates3{0, -1, 0}) == Coordinates3{0, 0, 0});
        REQUIRE(cross3(Coordinates3{0, 0, 1}, Coordinates3{0, 0, -1}) == Coordinates3{0, 0, 0});
    }
    SECTION("1") {
        REQUIRE(cross3(Coordinates3{1, 0, 0}, Coordinates3{0, 1, 0}) == Coordinates3{0, 0, 1});
        REQUIRE(cross3(Coordinates3{0, 1, 0}, Coordinates3{0, 0, 1}) == Coordinates3{1, 0, 0});
        REQUIRE(cross3(Coordinates3{0, 0, 1}, Coordinates3{1, 0, 0}) == Coordinates3{0, 1, 0});
        REQUIRE(cross3(Coordinates3{0, 1, 0}, Coordinates3{1, 0, 0}) == Coordinates3{0, 0, -1});
        REQUIRE(cross3(Coordinates3{0, 0, 1}, Coordinates3{0, 1, 0}) == Coordinates3{-1, 0, 0});
        REQUIRE(cross3(Coordinates3{1, 0, 0}, Coordinates3{0, 0, 1}) == Coordinates3{0, -1, 0});
    }
    SECTION("sum") {
        REQUIRE(cross3(Coordinates3{1, 2, 3}, Coordinates3{3, 2, 1}) == Coordinates3{-4, 8, -4});
    }
}

TEST_CASE("mesh intersection - square length", "[mesh intersection]") {
    SECTION("0") {
        REQUIRE(squareLength(Coordinates3{0, 0, 0}) == 0);
    }
    SECTION("1") {
        REQUIRE(squareLength(Coordinates3{1, 0, 0}) == 1);
        REQUIRE(squareLength(Coordinates3{0, 1, 0}) == 1);
        REQUIRE(squareLength(Coordinates3{0, 0, 1}) == 1);
        REQUIRE(squareLength(Coordinates3{-1, 0, 0}) == 1);
        REQUIRE(squareLength(Coordinates3{0, -1, 0}) == 1);
        REQUIRE(squareLength(Coordinates3{0, 0, -1}) == 1);
    }
    SECTION("square") {
        REQUIRE(squareLength(Coordinates3{2, 0, 0}) == 4);
        REQUIRE(squareLength(Coordinates3{0, 2, 0}) == 4);
        REQUIRE(squareLength(Coordinates3{0, 0, 2}) == 4);
        REQUIRE(squareLength(Coordinates3{1, 2, 3}) == 14);
    }
}

TEST_CASE("mesh intersection - length", "[mesh intersection]") {
    SECTION("0") {
        REQUIRE(squareLength(Coordinates3{0, 0, 0}) == 0);
    }
    SECTION("1") {
        REQUIRE(squareLength(Coordinates3{1, 0, 0}) == 1);
        REQUIRE(squareLength(Coordinates3{0, 1, 0}) == 1);
        REQUIRE(squareLength(Coordinates3{0, 0, 1}) == 1);
        REQUIRE(squareLength(Coordinates3{-1, 0, 0}) == 1);
        REQUIRE(squareLength(Coordinates3{0, -1, 0}) == 1);
        REQUIRE(squareLength(Coordinates3{0, 0, -1}) == 1);
    }
    SECTION("square") {
        REQUIRE(length(Coordinates3{2, 0, 0}) == 2);
        REQUIRE(length(Coordinates3{0, 2, 0}) == 2);
        REQUIRE(length(Coordinates3{0, 0, 2}) == 2);
        REQUIRE(length(Coordinates3{1, 1, 0}) == Approx(std::sqrt(2)).epsilon(0.001f));
        REQUIRE(length(Coordinates3{1, 0, 1}) == Approx(std::sqrt(2)).epsilon(0.001f));
        REQUIRE(length(Coordinates3{0, 1, 1}) == Approx(std::sqrt(2)).epsilon(0.001f));
        REQUIRE(length(Coordinates3{1, 2, 3}) == Approx(std::sqrt(14)).epsilon(0.001f));
    }
}

TEST_CASE("mesh intersection - normalize", "[mesh intersection]") {
    SECTION("0") {
        auto v = normalize(Coordinates3{0, 0, 0});
        REQUIRE(std::isnan(v[0]));
        REQUIRE(std::isnan(v[1]));
        REQUIRE(std::isnan(v[2]));
    }
    SECTION("1") {
        REQUIRE(normalize(Coordinates3{1, 0, 0}) == Coordinates3{1, 0, 0});
        REQUIRE(normalize(Coordinates3{0, 1, 0}) == Coordinates3{0, 1, 0});
        REQUIRE(normalize(Coordinates3{0, 0, 1}) == Coordinates3{0, 0, 1});
        REQUIRE(normalize(Coordinates3{-1, 0, 0}) == Coordinates3{-1, 0, 0});
        REQUIRE(normalize(Coordinates3{0, -1, 0}) == Coordinates3{0, -1, 0});
        REQUIRE(normalize(Coordinates3{0, 0, -1}) == Coordinates3{0, 0, -1});
        REQUIRE(normalize(Coordinates3{2, 0, 0}) == Coordinates3{1, 0, 0});
        REQUIRE(normalize(Coordinates3{0, 2, 0}) == Coordinates3{0, 1, 0});
        REQUIRE(normalize(Coordinates3{0, 0, 2}) == Coordinates3{0, 0, 1});
        REQUIRE(normalize(Coordinates3{-2, 0, 0}) == Coordinates3{-1, 0, 0});
        REQUIRE(normalize(Coordinates3{0, -2, 0}) == Coordinates3{0, -1, 0});
        REQUIRE(normalize(Coordinates3{0, 0, -2}) == Coordinates3{0, 0, -1});
    }
    SECTION("normalize") {
        REQUIRE(normalize(Coordinates3{1, 1, 0}) == Coordinates3{1.0f/std::sqrt(2.0f), 1.0f/std::sqrt(2.0f), 0});
        REQUIRE(normalize(Coordinates3{1, 0, 1}) == Coordinates3{1.0f/std::sqrt(2.0f), 0, 1.0f/std::sqrt(2.0f)});
        REQUIRE(normalize(Coordinates3{0, 1, 1}) == Coordinates3{0, 1.0f/std::sqrt(2.0f), 1.0f/std::sqrt(2.0f)});
        REQUIRE(normalize(Coordinates3{-1, -1, 0}) == Coordinates3{-1.0f/std::sqrt(2.0f),-1.0f/std::sqrt(2.0f), 0});
        REQUIRE(normalize(Coordinates3{-1, 0, -1}) == Coordinates3{-1.0f/std::sqrt(2.0f), 0, -1.0f/std::sqrt(2.0f)});
        REQUIRE(normalize(Coordinates3{0, -1, -1}) == Coordinates3{0, -1.0f/std::sqrt(2.0f), -1.0f/std::sqrt(2.0f)});
        REQUIRE(normalize(Coordinates3{1, 1, 1}) == Coordinates3{1.0f/std::sqrt(3.0f), 1.0f/std::sqrt(3.0f), 1.0f/std::sqrt(3.0f)});
        REQUIRE(normalize(Coordinates3{-1, -1, -1}) == Coordinates3{-1.0f/std::sqrt(3.0f), -1.0f/std::sqrt(3.0f), -1.0f/std::sqrt(3.0f)});
    }
}

TEST_CASE("mesh intersection - ray object", "[mesh intersection]") {
    SECTION("from points") {
        SECTION ("X") {
            auto r = Ray::fromPoints(Coordinates3{1, 1, 1}, Coordinates3{2, 1, 1});
            REQUIRE(r.origin == Coordinates3{1, 1, 1});
            REQUIRE(r.direction == Coordinates3{1, 0, 0});
        }
        SECTION ("Y") {
            auto r = Ray::fromPoints(Coordinates3{1, 1, 1}, Coordinates3{1, 2, 1});
            REQUIRE(r.origin == Coordinates3{1, 1, 1});
            REQUIRE(r.direction == Coordinates3{0, 1, 0});
        }
        SECTION ("Z") {
            auto r = Ray::fromPoints(Coordinates3{1, 1, 1}, Coordinates3{1, 1, 2});
            REQUIRE(r.origin == Coordinates3{1, 1, 1});
            REQUIRE(r.direction == Coordinates3{0, 0, 1});
        }
    }
}

TEST_CASE("mesh intersection - plan object", "[mesh intersection]") {
    SECTION("from triangle") {
        SECTION ("XY") {
            auto p = Plane::fromTriangle(Coordinates3{0, 0, 0}, Coordinates3{1, 0, 0}, Coordinates3{0, 1, 0});
            REQUIRE(p.pointOnPlane == Coordinates3{0, 0, 0});
            REQUIRE(p.normal == Coordinates3{0, 0, 1});
        }
        SECTION ("XZ") {
            auto p = Plane::fromTriangle(Coordinates3{0, 0, 0}, Coordinates3{1, 0, 0}, Coordinates3{0, 0, 1});
            REQUIRE(p.pointOnPlane == Coordinates3{0, 0, 0});
            REQUIRE(p.normal == Coordinates3{0, -1, 0});
        }
        SECTION ("YZ") {
            auto p = Plane::fromTriangle(Coordinates3{0, 0, 0}, Coordinates3{0, 1, 0}, Coordinates3{0, 0, 1});
            REQUIRE(p.pointOnPlane == Coordinates3{0, 0, 0});
            REQUIRE(p.normal == Coordinates3{1, 0, 0});
        }
    }
}

TEST_CASE("mesh intersection - intersection object", "[mesh intersection]") {
    SECTION("invalid") {
        auto invalidIntersection = Intersection::Invalid();
        REQUIRE_FALSE(invalidIntersection.valid);
    }
}

TEST_CASE("mesh intersection - triangle plane intersection object", "[mesh intersection]") {
    SECTION("invalid") {
        auto invalidIntersection = TrianglePlaneIntersection::Invalid();
        REQUIRE(invalidIntersection.peakIndex == 0xffffffff);
        REQUIRE_FALSE(invalidIntersection.intersectToPeak.valid);
        REQUIRE_FALSE(invalidIntersection.intersectFromPeak.valid);
    }
}

TEST_CASE("mesh intersection - triangle triangle intersection object", "[mesh intersection]") {
    SECTION("invalid") {
        auto invalidIntersection = TriangleTriangleIntersection::Invalid();
        REQUIRE_FALSE(invalidIntersection.valid);
    }
}

TEST_CASE("mesh intersection - calculate barycentric coordinate", "[mesh intersection]") {
    SECTION("1") {
        REQUIRE(barycentricCoordinateFromVectors(Coordinates3{0, 0, 0}, Coordinates3{1, 0, 0}, Coordinates3{0, 1, 0}) == Coordinates3{1, 0, 0});
        REQUIRE(barycentricCoordinateFromVectors(Coordinates3{1, 0, 0}, Coordinates3{1, 0, 0}, Coordinates3{0, 0, 1}) == Coordinates3{0, 1, 0});
        REQUIRE(barycentricCoordinateFromVectors(Coordinates3{0, 0, 1}, Coordinates3{0, 1, 0}, Coordinates3{0, 0, 1}) == Coordinates3{0, 0, 1});
    }
    SECTION("equilateral") {
        auto p0 = Coordinates3{-0.866, -0.5, 0};
        auto p1 = Coordinates3{0.866, -0.5, 0};
        auto p2 = Coordinates3{0, 1, 0};
        auto bc = barycentricCoordinateFromVectors(sub3(Coordinates3{0, 0, 0}, p0), sub3(p1, p0), sub3(p2, p0));
        REQUIRE(bc == Coordinates3{1.0f/3.0f, 1.0f/3.0f, 1.0f/3.0f});
    }
}

TEST_CASE("mesh intersection - interpolate barycentric 2D coordinate", "[mesh intersection]") {
    SECTION("1") {
        REQUIRE(interpolateBarycentric2(Coordinates3{1, 0, 0}, Coordinates2{0, 0}, Coordinates2{2, 0}, Coordinates2{0, 2}) == Coordinates2{0, 0});
        REQUIRE(interpolateBarycentric2(Coordinates3{0, 1, 0}, Coordinates2{0, 0}, Coordinates2{2, 0}, Coordinates2{0, 2}) == Coordinates2{2, 0});
        REQUIRE(interpolateBarycentric2(Coordinates3{0, 0, 1}, Coordinates2{0, 0}, Coordinates2{2, 0}, Coordinates2{0, 2}) == Coordinates2{0, 2});
    }
    SECTION("equilateral") {
        auto p0 = Coordinates2{-0.866, -0.5};
        auto p1 = Coordinates2{0.866, -0.5};
        auto p2 = Coordinates2{0, 1};
        auto bc = interpolateBarycentric2(Coordinates3{1.0f/3.0f, 1.0f/3.0f, 1.0f/3.0f}, p0, p1, p2);
        REQUIRE(bc == Coordinates2{0, 0});
    }
}

TEST_CASE("mesh intersection - interpolate barycentric 3D coordinate", "[mesh intersection]") {
    SECTION("1") {
        REQUIRE(interpolateBarycentric3(Coordinates3{1, 0, 0}, Coordinates3{2, 0, 0}, Coordinates3{0, 2, 0}, Coordinates3{0, 0, 2}) == Coordinates3{2, 0, 0});
        REQUIRE(interpolateBarycentric3(Coordinates3{0, 1, 0}, Coordinates3{2, 0, 0}, Coordinates3{0, 2, 0}, Coordinates3{0, 0, 2}) == Coordinates3{0, 2, 0});
        REQUIRE(interpolateBarycentric3(Coordinates3{0, 0, 1}, Coordinates3{2, 0, 0}, Coordinates3{0, 2, 0}, Coordinates3{0, 0, 2}) == Coordinates3{0, 0, 2});
    }
    SECTION("equilateral") {
        SECTION("XY") {
            auto p0 = Coordinates3{-0.866, -0.5, 0};
            auto p1 = Coordinates3{0.866, -0.5, 0};
            auto p2 = Coordinates3{0, 1, 0};
            auto bc = interpolateBarycentric3(Coordinates3{1.0f / 3.0f, 1.0f / 3.0f, 1.0f / 3.0f}, p0, p1, p2);
            REQUIRE(bc == Coordinates3{0, 0, 0});
        }
        SECTION("XZ") {
            auto p0 = Coordinates3{-0.866, 0, -0.5};
            auto p1 = Coordinates3{0.866, 0, -0.5};
            auto p2 = Coordinates3{0, 0, 1};
            auto bc = interpolateBarycentric3(Coordinates3{1.0f / 3.0f, 1.0f / 3.0f, 1.0f / 3.0f}, p0, p1, p2);
            REQUIRE(bc == Coordinates3{0, 0, 0});
        }
        SECTION("YZ") {
            auto p0 = Coordinates3{0, -0.866, -0.5};
            auto p1 = Coordinates3{0, 0.866, -0.5};
            auto p2 = Coordinates3{0, 0, 1};
            auto bc = interpolateBarycentric3(Coordinates3{1.0f / 3.0f, 1.0f / 3.0f, 1.0f / 3.0f}, p0, p1, p2);
            REQUIRE(bc == Coordinates3{0, 0, 0});
        }
    }
}

TEST_CASE("mesh intersection - intersect ray and plane", "[mesh intersection]") {
    SECTION("XY") {
        auto r = Ray{Coordinates3{0, 0, 2}, Coordinates3{0, 0, -1}};
        auto p = Plane{Coordinates3{-4, 0, 0},  Coordinates3{0, 0, 5}};
        auto i = intersectRayAndPlane(r, p);
        REQUIRE(i.valid);
        REQUIRE(i.distance == Approx(2).epsilon(0.001f));
        REQUIRE(i.point == Coordinates3{0, 0, 0});
    }
    SECTION("YZ") {
        auto r = Ray{Coordinates3{1, -4, 1}, Coordinates3{0, 1, 0}};
        auto p = Plane{Coordinates3{0, 0, 5},  Coordinates3{0, -3, 0}};
        auto i = intersectRayAndPlane(r, p);
        REQUIRE(i.valid);
        REQUIRE(i.distance == Approx(4).epsilon(0.001f));
        REQUIRE(i.point == Coordinates3{1, 0, 1});
    }
    SECTION("XZ") {
        auto r = Ray{Coordinates3{7, 1, 0}, Coordinates3{-1, 0, 0}};
        auto p = Plane{Coordinates3{0, 0, 3},  Coordinates3{1, 0, 0}};
        auto i = intersectRayAndPlane(r, p);
        REQUIRE(i.valid);
        REQUIRE(i.distance == Approx(7).epsilon(0.001f));
        REQUIRE(i.point == Coordinates3{0, 1, 0});
    }
    SECTION("diagonal") {
        auto r = Ray{Coordinates3{1, 1, 1}, Coordinates3{1, 1, -1}};
        auto p = Plane{Coordinates3{0, 0, 0},  Coordinates3{0, 0, 1}};
        auto i = intersectRayAndPlane(r, p);
        REQUIRE(i.valid);
        REQUIRE(i.distance == Approx(1).epsilon(0.001f));
        REQUIRE(i.point == Coordinates3{2, 2, 0});
    }
    SECTION("invalid") {
        auto r = Ray{Coordinates3{1, 1, 1}, Coordinates3{1, 1, 0}};
        auto p = Plane{Coordinates3{0, 0, 0},  Coordinates3{0, 0, 1}};
        auto i = intersectRayAndPlane(r, p);
        REQUIRE_FALSE(i.valid);
    }
}

TEST_CASE("mesh intersection - are endless ray and box intersecting", "[mesh intersection]") {
    SECTION("X intersecting") {
        auto r = Ray{ Coordinates3{-2, 0, 0}, Coordinates3{1, 0, 0} };
        REQUIRE(areEndlessRayAndBoxIntersecting(r, Coordinates3{-1, -1, -1}, Coordinates3{1, 1, 1}));
    }
    SECTION("X not intersecting") {
        auto r = Ray{ Coordinates3{-2, 2, 0}, Coordinates3{1, 0, 0} };
        REQUIRE_FALSE(areEndlessRayAndBoxIntersecting(r, Coordinates3{-1, -1, -1}, Coordinates3{1, 1, 1}));
    }
    SECTION("Y intersecting") {
        auto r = Ray{ Coordinates3{0, -2, 0}, Coordinates3{0, 1, 0} };
        REQUIRE(areEndlessRayAndBoxIntersecting(r, Coordinates3{-1, -1, -1}, Coordinates3{1, 1, 1}));
    }
    SECTION("Y not intersecting") {
        auto r = Ray{ Coordinates3{0, -2, 2}, Coordinates3{0, 1, 0} };
        REQUIRE_FALSE(areEndlessRayAndBoxIntersecting(r, Coordinates3{-1, -1, -1}, Coordinates3{1, 1, 1}));
    }
    SECTION("Z intersecting") {
        auto r = Ray{ Coordinates3{0, 0, -2}, Coordinates3{0, 0, 1} };
        REQUIRE(areEndlessRayAndBoxIntersecting(r, Coordinates3{-1, -1, -1}, Coordinates3{1, 1, 1}));
    }
    SECTION("Z not intersecting") {
        auto r = Ray{ Coordinates3{2, 0, -2}, Coordinates3{0, 0, 1} };
        REQUIRE_FALSE(areEndlessRayAndBoxIntersecting(r, Coordinates3{-1, -1, -1}, Coordinates3{1, 1, 1}));
    }
    SECTION("diagonal intersecting") {
        auto r = Ray{ Coordinates3{-2, -2, -2}, Coordinates3{1, 1, 1} };
        REQUIRE(areEndlessRayAndBoxIntersecting(r, Coordinates3{-1, -1, -1}, Coordinates3{1, 1, 1}));
    }
    SECTION("diagonal not intersecting") {
        auto r = Ray{ Coordinates3{-2, -2, -2}, Coordinates3{1, 1, -1} };
        REQUIRE_FALSE(areEndlessRayAndBoxIntersecting(r, Coordinates3{-1, -1, -1}, Coordinates3{1, 1, 1}));
    }
}

TEST_CASE("mesh intersection - is point on same side", "[mesh intersection]") {
    SECTION("XY on same side") {
        REQUIRE(isPointOnSameSide(Coordinates3{0, 2, 0}, Coordinates3{0, 1, 0}, Coordinates3{0, 0, 0}, Coordinates3{1, 0, 0}));
        REQUIRE(isPointOnSameSide(Coordinates3{0, 2, 0}, Coordinates3{0, 1, 0}, Coordinates3{1, 0, 0}, Coordinates3{0, 0, 0}));
    }
    SECTION("XY not on same side") {
        REQUIRE_FALSE(isPointOnSameSide(Coordinates3{0, -2, 0}, Coordinates3{0, 1, 0}, Coordinates3{0, 0, 0}, Coordinates3{1, 0, 0}));
    }
    SECTION("XZ on same side") {
        REQUIRE(isPointOnSameSide(Coordinates3{0, 0, 1}, Coordinates3{0, 0, 1}, Coordinates3{0, 0, 0}, Coordinates3{0, 1, 0}));
        REQUIRE(isPointOnSameSide(Coordinates3{0, 0, 1}, Coordinates3{0, 0, 1}, Coordinates3{0, 1, 0}, Coordinates3{0, 0, 0}));
    }
    SECTION("XZ not on same side") {
        REQUIRE_FALSE(isPointOnSameSide(Coordinates3{0, 0, -1}, Coordinates3{0, 0, 1}, Coordinates3{0, 0, 0}, Coordinates3{0, 1, 0}));
    }
    SECTION("YZ on same side") {
        REQUIRE(isPointOnSameSide(Coordinates3{3, 0, 0}, Coordinates3{1, 0, 0}, Coordinates3{0, 0, 0}, Coordinates3{0, 0, 1}));
        REQUIRE(isPointOnSameSide(Coordinates3{3, 0, 0}, Coordinates3{1, 0, 0}, Coordinates3{1, 0, 0}, Coordinates3{0, 0, 0}));
    }
    SECTION("YZ not on same side") {
        REQUIRE_FALSE(isPointOnSameSide(Coordinates3{-1, 0, 0}, Coordinates3{1, 0, 0}, Coordinates3{0, 0, 0}, Coordinates3{0, 0, 1}));
    }
    SECTION("diagonal on same side") {
        REQUIRE(isPointOnSameSide(Coordinates3{0, 2, 0}, Coordinates3{0, 1, 0}, Coordinates3{0, 0, 0}, Coordinates3{1, 1, 0}));
        REQUIRE(isPointOnSameSide(Coordinates3{0, 2, 0}, Coordinates3{0, 1, 0}, Coordinates3{1, 1, 0}, Coordinates3{0, 0, 0}));
    }
    SECTION("diagonal not on same side") {
        REQUIRE_FALSE(isPointOnSameSide(Coordinates3{0, -2, 0}, Coordinates3{0, 1, 0}, Coordinates3{0, 0, 0}, Coordinates3{1, 1, 0}));
    }
}

TEST_CASE("mesh intersection - is point in triangle", "[mesh intersection]") {
    SECTION("XY in triangle") {
        REQUIRE(isPointInOrOnTriangle(Coordinates3{1, 1, 0}, Coordinates3{0, 0, 0}, Coordinates3{0, 3, 0}, Coordinates3{3, 0, 0}));
    }
    SECTION("XY not in triangle") {
        REQUIRE_FALSE(isPointInOrOnTriangle(Coordinates3{-1, -1, 0}, Coordinates3{0, 0, 0}, Coordinates3{0, 3, 0}, Coordinates3{3, 0, 0}));
        REQUIRE_FALSE(isPointInOrOnTriangle(Coordinates3{1, -1, 0}, Coordinates3{0, 0, 0}, Coordinates3{0, 3, 0}, Coordinates3{3, 0, 0}));
        REQUIRE_FALSE(isPointInOrOnTriangle(Coordinates3{-1, 1, 0}, Coordinates3{0, 0, 0}, Coordinates3{0, 3, 0}, Coordinates3{3, 0, 0}));
    }
    SECTION("XZ in triangle") {
        REQUIRE(isPointInOrOnTriangle(Coordinates3{1, 0, 1}, Coordinates3{0, 0, 0}, Coordinates3{0, 0, 3}, Coordinates3{3, 0, 0}));
    }
    SECTION("XZ not in triangle") {
        REQUIRE_FALSE(isPointInOrOnTriangle(Coordinates3{-1, 0, -1}, Coordinates3{0, 0, 0}, Coordinates3{0, 0, 3}, Coordinates3{3, 0, 0}));
        REQUIRE_FALSE(isPointInOrOnTriangle(Coordinates3{1, 0, -1}, Coordinates3{0, 0, 0}, Coordinates3{0, 0, 3}, Coordinates3{3, 0, 0}));
        REQUIRE_FALSE(isPointInOrOnTriangle(Coordinates3{-1, 0, 1}, Coordinates3{0, 0, 0}, Coordinates3{0, 0, 3}, Coordinates3{3, 0, 0}));
    }
    SECTION("YZ in triangle") {
        REQUIRE(isPointInOrOnTriangle(Coordinates3{0, 1, 1}, Coordinates3{0, 0, 0}, Coordinates3{0, 0, 3}, Coordinates3{0, 3, 0}));
    }
    SECTION("YZ not in triangle") {
        REQUIRE_FALSE(isPointInOrOnTriangle(Coordinates3{0, -1, -1}, Coordinates3{0, 0, 0}, Coordinates3{0, 0, 3}, Coordinates3{0, 3, 0}));
        REQUIRE_FALSE(isPointInOrOnTriangle(Coordinates3{0, 1, -1}, Coordinates3{0, 0, 0}, Coordinates3{0, 0, 3}, Coordinates3{0, 3, 0}));
        REQUIRE_FALSE(isPointInOrOnTriangle(Coordinates3{0, -1, 1}, Coordinates3{0, 0, 0}, Coordinates3{0, 0, 3}, Coordinates3{0, 3, 0}));
    }
    SECTION("XYZ in triangle") {
        REQUIRE(isPointInOrOnTriangle(Coordinates3{0, 0, 0}, Coordinates3{-1, -1, -1}, Coordinates3{-1, -1, 1}, Coordinates3{1, 1, 0}));
    }
    SECTION("XYZ not in triangle") {
        REQUIRE_FALSE(isPointInOrOnTriangle(Coordinates3{0, 0, 2}, Coordinates3{-1, -1, -1}, Coordinates3{-1, -1, 1}, Coordinates3{1, 1, 0}));
        REQUIRE_FALSE(isPointInOrOnTriangle(Coordinates3{2, 2, 0}, Coordinates3{-1, -1, -1}, Coordinates3{-1, -1, 1}, Coordinates3{1, 1, 0}));
        REQUIRE_FALSE(isPointInOrOnTriangle(Coordinates3{-2, -2, 0}, Coordinates3{-1, -1, -1}, Coordinates3{-1, -1, 1}, Coordinates3{1, 1, 0}));
    }
}

TEST_CASE("mesh intersection - intersect ray and triangle", "[mesh intersection]") {
    SECTION("XY intersecting") {
        auto r = Ray{ Coordinates3{0, 0, 1}, Coordinates3{1, 1, -1} };
        auto i = intersectRayAndTriangle(r, Coordinates3{0, 0, 0}, Coordinates3{0, 3, 0}, Coordinates3{3, 0, 0}, 2);
        REQUIRE(i.valid);
        REQUIRE(i.distance == 1);
        REQUIRE(i.point == Coordinates3{1, 1, 0});
    }
    SECTION("XY not intersecting because of distance") {
        auto r = Ray{ Coordinates3{0, 0, 1}, Coordinates3{1, 1, -1} };
        auto i = intersectRayAndTriangle(r, Coordinates3{0, 0, 0}, Coordinates3{0, 3, 0}, Coordinates3{3, 0, 0}, 0.5);
        REQUIRE_FALSE(i.valid);
    }
    SECTION("XY not intersecting because of direction") {
        auto r = Ray{ Coordinates3{0, 0, 1}, Coordinates3{1, 1, 1} };
        auto i = intersectRayAndTriangle(r, Coordinates3{0, 0, 0}, Coordinates3{0, 3, 0}, Coordinates3{3, 0, 0}, 2);
        REQUIRE_FALSE(i.valid);
    }
    SECTION("XZ intersecting") {
        auto r = Ray{ Coordinates3{0, -1, 0}, Coordinates3{1, 1, 1} };
        auto i = intersectRayAndTriangle(r, Coordinates3{0, 0, 0}, Coordinates3{3, 0, 0}, Coordinates3{0, 0, 3}, 2);
        REQUIRE(i.valid);
        REQUIRE(i.distance == 1);
        REQUIRE(i.point == Coordinates3{1, 0, 1});
    }
    SECTION("XZ not intersecting because of distance") {
        auto r = Ray{ Coordinates3{0, -1, 0}, Coordinates3{1, 1, 1} };
        auto i = intersectRayAndTriangle(r, Coordinates3{0, 0, 0}, Coordinates3{3, 0, 0}, Coordinates3{0, 0, 3}, 0.5);
        REQUIRE_FALSE(i.valid);
    }
    SECTION("XZ not intersecting because of direction") {
        auto r = Ray{ Coordinates3{0, -1, 0}, Coordinates3{1, -1, 1} };
        auto i = intersectRayAndTriangle(r, Coordinates3{0, 0, 0}, Coordinates3{3, 0, 0}, Coordinates3{0, 0, 3}, 2);
        REQUIRE_FALSE(i.valid);
    }
    SECTION("YZ intersecting") {
        auto r = Ray{ Coordinates3{1, 0, 0}, Coordinates3{-1, 1, 1} };
        auto i = intersectRayAndTriangle(r, Coordinates3{0, 0, 0}, Coordinates3{0, 3, 0}, Coordinates3{0, 0, 3}, 2);
        REQUIRE(i.valid);
        REQUIRE(i.distance == 1);
        REQUIRE(i.point == Coordinates3{0, 1, 1});
    }
    SECTION("YZ not intersecting because of distance") {
        auto r = Ray{ Coordinates3{1, 0, 0}, Coordinates3{-1, 1, 1} };
        auto i = intersectRayAndTriangle(r, Coordinates3{0, 0, 0}, Coordinates3{0, 3, 0}, Coordinates3{0, 0, 3}, 0.5);
        REQUIRE_FALSE(i.valid);
    }
    SECTION("YZ not intersecting because of direction") {
        auto r = Ray{ Coordinates3{1, 0, 0}, Coordinates3{-1, -1, -1} };
        auto i = intersectRayAndTriangle(r, Coordinates3{0, 0, 0}, Coordinates3{0, 3, 0}, Coordinates3{0, 0, 3}, 2);
        REQUIRE_FALSE(i.valid);
    }
    SECTION("XYZ intersecting") {
        auto r = Ray{ Coordinates3{1, 1, 1}, Coordinates3{-1, -1, -1} };
        auto i = intersectRayAndTriangle(r, Coordinates3{-1, -1, 1}, Coordinates3{-1, 1, -1}, Coordinates3{1, -1, -1}, 2);
        REQUIRE(i.valid);
        REQUIRE(i.distance == Approx(1.33334f).epsilon(0.001f));
        REQUIRE(i.point == Coordinates3{-0.33334f, -0.33334f, -0.33334f});
    }
    SECTION("XYZ not intersecting because of distance") {
        auto r = Ray{ Coordinates3{1, 1, 1}, Coordinates3{-1, -1, -1} };
        auto i = intersectRayAndTriangle(r, Coordinates3{-1, -1, 1}, Coordinates3{-1, 1, -1}, Coordinates3{1, -1, -1}, 1);
        REQUIRE_FALSE(i.valid);
    }
    SECTION("XYZ not intersecting because of direction") {
        auto r = Ray{ Coordinates3{1, 1, 1}, Coordinates3{1, 1, 1} };
        auto i = intersectRayAndTriangle(r, Coordinates3{-1, -1, 1}, Coordinates3{-1, 1, -1}, Coordinates3{1, -1, -1}, 2);
        REQUIRE_FALSE(i.valid);
    }
}

TEST_CASE("mesh intersection - intersect triangle and plane", "[mesh intersection]") {
    SECTION("index 0") {
        auto p = Plane{Coordinates3{0, 0, 0}, Coordinates3{0, 0, 1}};
        auto i = intersectTriangleAndPlane(Coordinates3{0, 0, 1}, Coordinates3{-1, 0, -1}, Coordinates3{1, 0, -1}, p);
        REQUIRE(i.peakIndex == 0);
        REQUIRE(i.intersectFromPeak.valid);
        REQUIRE(i.intersectFromPeak.distance == 0.5f);
        REQUIRE(i.intersectFromPeak.point == Coordinates3{-0.5, 0, 0});
        REQUIRE(i.intersectToPeak.valid);
        REQUIRE(i.intersectToPeak.distance == 0.5f);
        REQUIRE(i.intersectToPeak.point == Coordinates3{0.5, 0, 0});
    }
    SECTION("index 1") {
        auto p = Plane{Coordinates3{0, 0, 0}, Coordinates3{0, 1, 0}};
        auto i = intersectTriangleAndPlane(Coordinates3{0, -1, -1}, Coordinates3{0, 1, 0}, Coordinates3{0, -1, 1}, p);
        REQUIRE(i.peakIndex == 1);
        REQUIRE(i.intersectFromPeak.valid);
        REQUIRE(i.intersectFromPeak.distance == 0.5f);
        REQUIRE(i.intersectFromPeak.point == Coordinates3{0, 0, 0.5f});
        REQUIRE(i.intersectToPeak.valid);
        REQUIRE(i.intersectToPeak.distance == 0.5f);
        REQUIRE(i.intersectToPeak.point == Coordinates3{0, 0, -0.5f});
    }
    SECTION("index 2") {
        auto p = Plane{Coordinates3{0, 0, 0}, Coordinates3{-1, -1, 0}};
        auto i = intersectTriangleAndPlane(Coordinates3{-1, -1, 1}, Coordinates3{-1, -1, -1}, Coordinates3{1, 1, 0}, p);
        REQUIRE(i.peakIndex == 2);
        REQUIRE(i.intersectFromPeak.valid);
        REQUIRE(i.intersectFromPeak.distance == 0.5f);
        REQUIRE(i.intersectFromPeak.point == Coordinates3{0, 0, 0.5f});
        REQUIRE(i.intersectToPeak.valid);
        REQUIRE(i.intersectToPeak.distance == 0.5f);
        REQUIRE(i.intersectToPeak.point == Coordinates3{0, 0, -0.5f});
    }
    SECTION("no intersection because of distance") {
        auto p = Plane{Coordinates3{0, 0, -2}, Coordinates3{0, 0, 1}};
        auto i = intersectTriangleAndPlane(Coordinates3{0, 0, 1}, Coordinates3{-1, 0, -1}, Coordinates3{1, 0, -1}, p);
        REQUIRE(i.peakIndex == 0xffffffff);
        REQUIRE_FALSE(i.intersectFromPeak.valid);
        REQUIRE_FALSE(i.intersectToPeak.valid);
    }
    SECTION("no intersection because of orientation") {
        auto p = Plane{Coordinates3{0, 0, 0}, Coordinates3{0, 0, 1}};
        auto i = intersectTriangleAndPlane(Coordinates3{0, 0, 1}, Coordinates3{1, 0, 1}, Coordinates3{0, 1, 1}, p);
        REQUIRE(i.peakIndex == 0xffffffff);
        REQUIRE_FALSE(i.intersectFromPeak.valid);
        REQUIRE_FALSE(i.intersectToPeak.valid);
    }
}

TEST_CASE("mesh intersection - intersect triangle and triangle", "[mesh intersection]") {
    SECTION("intersection") {
        auto i = intersectTriangles(Coordinates3{0, 0, 1}, Coordinates3{-1, 0, -1}, Coordinates3{1, 0, -1}, Coordinates3{0, -1, 0}, Coordinates3{-1, 1, 0}, Coordinates3{1, 1, 0});
        REQUIRE(i.valid);
        REQUIRE(i.intersectionTriangle0.peakIndex == 0);
        REQUIRE(i.intersectionTriangle0.intersectFromPeak.distance == 0.5f);
        REQUIRE(i.intersectionTriangle0.intersectFromPeak.point == Coordinates3{-0.5, 0, 0});
        REQUIRE(i.intersectionTriangle0.intersectToPeak.valid);
        REQUIRE(i.intersectionTriangle0.intersectToPeak.distance == 0.5f);
        REQUIRE(i.intersectionTriangle0.intersectToPeak.point == Coordinates3{0.5, 0, 0});
        REQUIRE(i.intersectionTriangle1.peakIndex == 0);
        REQUIRE(i.intersectionTriangle1.intersectFromPeak.distance == 0.5f);
        REQUIRE(i.intersectionTriangle1.intersectFromPeak.point == Coordinates3{-0.5, 0, 0});
        REQUIRE(i.intersectionTriangle1.intersectToPeak.valid);
        REQUIRE(i.intersectionTriangle1.intersectToPeak.distance == 0.5f);
        REQUIRE(i.intersectionTriangle1.intersectToPeak.point == Coordinates3{0.5, 0, 0});
    }
    SECTION("no intersection") {
        auto i = intersectTriangles(Coordinates3{0, 0, 1}, Coordinates3{-1, 0, -1}, Coordinates3{1, 0, -1}, Coordinates3{0, -1, 0}, Coordinates3{-1, -2, 0}, Coordinates3{1, -2, 0});
        REQUIRE_FALSE(i.valid);
    }
}

TEST_CASE("mesh intersection - unique indices object", "[mesh intersection]") {
    UniqueIndices uniqueIndices;
    SECTION("get index") {
        REQUIRE(uniqueIndices.getIndex(0, 0, 0) == 0);
        REQUIRE(uniqueIndices.getIndex(1, 0, 0) == 1);
        REQUIRE(uniqueIndices.getIndex(0, 1, 0) == 2);
        REQUIRE(uniqueIndices.getIndex(1, 1, 0) == 3);
        REQUIRE(uniqueIndices.getIndex(0, 0, 1) == 4);
        REQUIRE(uniqueIndices.getIndex(1, 0, 1) == 5);
        REQUIRE(uniqueIndices.getIndex(0, 1, 1) == 6);
        REQUIRE(uniqueIndices.getIndex(1, 1, 1) == 7);
    }
    SECTION("get vertex index") {
        REQUIRE(uniqueIndices.getVertexIndex(Coordinates3{0, 0, 0}) == 0);
        REQUIRE(uniqueIndices.getVertexIndex(Coordinates3{1, 0, 0}) == 1);
        REQUIRE(uniqueIndices.getVertexIndex(Coordinates3{0, 1, 0}) == 2);
        REQUIRE(uniqueIndices.getVertexIndex(Coordinates3{1, 1, 0}) == 3);
        REQUIRE(uniqueIndices.getVertexIndex(Coordinates3{0, 0, 1}) == 4);
        REQUIRE(uniqueIndices.getVertexIndex(Coordinates3{1, 0, 1}) == 5);
        REQUIRE(uniqueIndices.getVertexIndex(Coordinates3{0, 1, 1}) == 6);
        REQUIRE(uniqueIndices.getVertexIndex(Coordinates3{1, 1, 1}) == 7);
        REQUIRE(uniqueIndices.getVertexIndex(Coordinates3{0, 0, 0}) == 0);
        REQUIRE(uniqueIndices.getVertexIndex(Coordinates3{1, 0, 0}) == 1);
        REQUIRE(uniqueIndices.getVertexIndex(Coordinates3{0, 1, 0}) == 2);
        REQUIRE(uniqueIndices.getVertexIndex(Coordinates3{1, 1, 0}) == 3);
        REQUIRE(uniqueIndices.getVertexIndex(Coordinates3{0, 0, 1}) == 4);
        REQUIRE(uniqueIndices.getVertexIndex(Coordinates3{1, 0, 1}) == 5);
        REQUIRE(uniqueIndices.getVertexIndex(Coordinates3{0, 1, 1}) == 6);
        REQUIRE(uniqueIndices.getVertexIndex(Coordinates3{1, 1, 1}) == 7);
    }
    SECTION("create unique index") {
        const float vertices[] = { 0, 0, 0, 1, 0, 0, 1, 1, 1, 0, 1, 1,  1, 1, 1, 0, 1, 1, 0, 0, 2, 1, 0, 2 };
        auto actualUniqueIndices = uniqueIndices.createUniqueIndices(vertices, 8);
        UniqueIndices::IndexMap expectedUniqueIndices{0, 1, 2, 3, 2, 3, 4, 5};
        REQUIRE(actualUniqueIndices.size() == expectedUniqueIndices.size());
        for(uint32_t i = 0; i < actualUniqueIndices.size(); ++i) {
            REQUIRE(actualUniqueIndices[i] == expectedUniqueIndices[i]);
        }
    }
}

TEST_CASE("mesh intersection - triangle object", "[mesh intersection]") {
    SECTION("face normal vector") {
        std::vector<float> vertices{0, 0, 0, 1, 0, 0, 0, 1, 0};
        std::vector<uint32_t> indices{0, 1, 2};
        UniqueIndices::IndexMap indexMap{0, 1, 2};
        Triangle triangle(vertices.data(), indices.data(), indexMap);
        REQUIRE(triangle.faceNormal == Coordinates3{0, 0, 1});
    }
    SECTION("axis aligned bounding box") {
        std::vector<float> vertices{0, 0, 0, 1, 0, 0, 0, 1, 1};
        std::vector<uint32_t> indices{0, 1, 2};
        UniqueIndices::IndexMap indexMap{0, 1, 2};
        Triangle triangle(vertices.data(), indices.data(), indexMap);
        REQUIRE(triangle.aabbMin == Coordinates3{0, 0, 0});
        REQUIRE(triangle.aabbMax == Coordinates3{1, 1, 1});
    }
}

TEST_CASE("mesh intersection - simple meshes", "[mesh intersection]") {
    std::vector<float> verticesMesh0;
    std::vector<uint32_t> indicesMesh0;
    std::vector<float> verticesMesh1;
    std::vector<uint32_t> indicesMesh1;
    ResultMeshData expectedResult;

    SECTION("triangle 2 penetrates triangle 1") {
        verticesMesh0 = std::vector<float>{0, 0, -1, -1, 0, 1, 1, 0, 1};
        indicesMesh0 = std::vector<uint32_t>{0, 1, 2};
        verticesMesh1 = std::vector<float>{0, 0.5, 0, -1, -1.5, 0, 1, -1.5, 0};
        indicesMesh1 = std::vector<uint32_t>{0, 1, 2};
        expectedResult.vertices = std::vector<float>{0, 0, -1, -1, 0, 1, 1, 0, 1, 0.25, 0, 0, -0.25, 0, 0};
        expectedResult.indicesOut = std::vector<uint32_t>{2, 3, 4, 2, 0, 3, 1, 2, 4, 0, 1, 4, };
        expectedResult.indicesIn = std::vector<uint32_t>{0, 3, 4};
    }
    SECTION("triangle 1 penetrates triangle 2") {
        verticesMesh0 = std::vector<float>{0, 0, -1, -1, 0, 1, 1, 0, 1};
        indicesMesh0 = std::vector<uint32_t>{0, 1, 2};
        verticesMesh1 = std::vector<float>{0, 1.5, 0, -1, -0.5, 0, 1, -0.5, 0};
        indicesMesh1 = std::vector<uint32_t>{0, 1, 2};
        expectedResult.vertices = std::vector<float>{0, 0, -1, -1, 0, 1, 1, 0, 1, 0.5, 0, 0, -0.5, 0, 0};
        expectedResult.indicesOut = std::vector<uint32_t>{2, 3, 4, 1, 2, 4};
        expectedResult.indicesIn = std::vector<uint32_t>{0, 3, 4};
    }
    SECTION("shared corner, triangle 2 intersects triangle 1") {
        verticesMesh0 = std::vector<float>{1, 0, -1, -1, 0, 0, 1, 0, 1};
        indicesMesh0 = std::vector<uint32_t>{0, 1, 2};
        verticesMesh1 = std::vector<float>{-1, 0, 0, 0.5, -1, 0, 0.5, 1, 0};
        indicesMesh1 = std::vector<uint32_t>{0, 1, 2};
        expectedResult.vertices = std::vector<float>{1, 0, -1, -1, 0, 0, 1, 0, 1, 0.5, 0, 0};
        expectedResult.indicesOut = std::vector<uint32_t>{2, 3, 1, 2, 0, 3};
        expectedResult.indicesIn = std::vector<uint32_t>{0, 3, 1};
    }
    SECTION("shared corner, triangle 1 intersects triangle 2") {
        verticesMesh0 = std::vector<float>{0.5, 0, -1, -1, 0, 0, 0.5, 0, 1};
        indicesMesh0 = std::vector<uint32_t>{0, 1, 2};
        verticesMesh1 = std::vector<float>{-1, 0, 0, 1, -1, 0, 1, 1, 0};
        indicesMesh1 = std::vector<uint32_t>{0, 1, 2};
        expectedResult.vertices = std::vector<float>{0.5, 0, -1, -1, 0, 0, 0.5, 0, 1, 0.5, 0, 0};
        expectedResult.indicesOut = std::vector<uint32_t>{2, 3, 1};
        expectedResult.indicesIn = std::vector<uint32_t>{0, 3, 1};
    }
    SECTION("shared corner, intersection on touching edges") {
        verticesMesh0 = std::vector<float>{1, 0, -1, -1, 0, 0, 1, 0, 1};
        indicesMesh0 = std::vector<uint32_t>{0, 1, 2};
        verticesMesh1 = std::vector<float>{-1, 0, 0, 1, -1, 0, 1, 1, 0};
        indicesMesh1 = std::vector<uint32_t>{0, 1, 2};
        expectedResult.vertices = std::vector<float>{1, 0, -1, -1, 0, 0, 1, 0, 1, 1, 0, 0};
        expectedResult.indicesOut = std::vector<uint32_t>{2, 3, 1};
        expectedResult.indicesIn = std::vector<uint32_t>{0, 3, 1};
    }
    SECTION("intersection with 2 touching edges") {
        verticesMesh0 = std::vector<float>{0, 0, -1, -1, 0, 1, 1, 0, 1};
        indicesMesh0 = std::vector<uint32_t>{0, 1, 2};
        verticesMesh1 = std::vector<float>{0, 1, 0, -1, -1, 0, 1, -1, 0};
        indicesMesh1 = std::vector<uint32_t>{0, 1, 2};
        expectedResult.vertices = std::vector<float>{0, 0, -1, -1, 0, 1, 1, 0, 1, 0.5, 0, 0, -0.5, 0, 0};
        expectedResult.indicesOut = std::vector<uint32_t>{2, 3, 4, 1, 2, 4};
        expectedResult.indicesIn = std::vector<uint32_t>{0, 3, 4};
    }
    SECTION("corner of triangle 2 touches edge of triangle 1") {
        verticesMesh0 = std::vector<float>{0, 0, -1, -1, 0, 1, 1, 0, 1};
        indicesMesh0 = std::vector<uint32_t>{0, 1, 2};
        verticesMesh1 = std::vector<float>{-0.5, 0, 0, 1, -1, 0, 1, 1, 0};
        indicesMesh1 = std::vector<uint32_t>{0, 1, 2};
        expectedResult.vertices = std::vector<float>{0, 0, -1, -1, 0, 1, 1, 0, 1, 0.5, 0, 0, -0.5, 0, 0};
        expectedResult.indicesOut = std::vector<uint32_t>{2, 3, 4, 1, 2, 4};
        expectedResult.indicesIn = std::vector<uint32_t>{0, 3, 4};
    }
    SECTION("edge of triangle 2 touches edge of triangle 1") {
        verticesMesh0 = std::vector<float>{0, 0, -1, -1, 0, 1, 1, 0, 1};
        indicesMesh0 = std::vector<uint32_t>{0, 1, 2};
        verticesMesh1 = std::vector<float>{1, 0, 0, -0.5, 1, 0, -0.5, -1, 0};
        indicesMesh1 = std::vector<uint32_t>{0, 1, 2};
        expectedResult.vertices = std::vector<float>{0, 0, -1, -1, 0, 1, 1, 0, 1, 0.5, 0, 0, -0.5, 0, 0};
        expectedResult.indicesOut = std::vector<uint32_t>{2, 3, 4, 1, 2, 4};
        expectedResult.indicesIn = std::vector<uint32_t>{0, 3, 4};
    }
    SECTION("mutually corners on edges") {
        verticesMesh0 = std::vector<float>{-1, 0, -1, -1, 0, 1, 1, 0, 0};
        indicesMesh0 = std::vector<uint32_t>{0, 1, 2};
        verticesMesh1 = std::vector<float>{-1, 0, 0, 1, -1, 0, 1, 1, 0};
        indicesMesh1 = std::vector<uint32_t>{0, 1, 2};
        expectedResult.vertices = std::vector<float>{-1, 0, -1, -1, 0, 1, 1, 0, 0, -1, 0, 0, 1, 0, 0};
        expectedResult.indicesOut = std::vector<uint32_t>{1, 4, 3};
        expectedResult.indicesIn = std::vector<uint32_t>{0, 4, 3};
    }
    SECTION("no intersection, shared edge out") {
        verticesMesh0 = std::vector<float>{1, 0, -1, -1, 0, 0, 1, 0, 1};
        indicesMesh0 = std::vector<uint32_t>{0, 1, 2};
        verticesMesh1 = std::vector<float>{1, 0, -1, 1, 1, -1, -1, 0, 0};
        indicesMesh1 = std::vector<uint32_t>{0, 1, 2};
        expectedResult.vertices = std::vector<float>{1, 0, -1, -1, 0, 0, 1, 0, 1};
        expectedResult.indicesOut = std::vector<uint32_t>{0, 1, 2};
        expectedResult.indicesIn = std::vector<uint32_t>{};
    }
    SECTION("no intersection, shared edge in") {
        verticesMesh0 = std::vector<float>{1, 0, -1, -1, 0, 0, 1, 0, 1};
        indicesMesh0 = std::vector<uint32_t>{0, 1, 2};
        verticesMesh1 = std::vector<float>{1, 0, 1, 1, 1, 1, -1, 0, 0};
        indicesMesh1 = std::vector<uint32_t>{0, 1, 2};
        expectedResult.vertices = std::vector<float>{1, 0, -1, -1, 0, 0, 1, 0, 1};
        expectedResult.indicesOut = std::vector<uint32_t>{};
        expectedResult.indicesIn = std::vector<uint32_t>{0, 2, 1};
    }
    SECTION("no intersection, shared corner") {
        verticesMesh0 = std::vector<float>{-1, 0, 0, 1, 0, 0, 1, 0, -1};
        indicesMesh0 = std::vector<uint32_t>{0, 1, 2};
        verticesMesh1 = std::vector<float>{-1, 0, 0, 1, 0, 1, 1, 1, 1};
        indicesMesh1 = std::vector<uint32_t>{0, 1, 2};
        expectedResult.vertices = std::vector<float>{-1, 0, 0, 1, 0, 0, 1, 0, -1};
        expectedResult.indicesOut = std::vector<uint32_t>{};
        expectedResult.indicesIn = std::vector<uint32_t>{0, 2, 1};
    }
    SECTION("no intersection") {
        verticesMesh0 = std::vector<float>{-1, 0, 0, 0, 0, 0.5, 0, 0, -0.5};
        indicesMesh0 = std::vector<uint32_t>{0, 1, 2};
        verticesMesh1 = std::vector<float>{0.1, 0, 0, 1, -0.5, 0, 1, 0.5, 0};
        indicesMesh1 = std::vector<uint32_t>{0, 1, 2};
        expectedResult.vertices = std::vector<float>{};
        expectedResult.indicesOut = std::vector<uint32_t>{};
        expectedResult.indicesIn = std::vector<uint32_t>{};
    }
    SECTION("no intersection, triangle 2 between 2 triangles of mesh 1") {
        verticesMesh0 = std::vector<float>{0.5, 0, 1, 0.5, 0, 0, -0.5, 0, 0, 0.5, 0, -1};
        indicesMesh0 = std::vector<uint32_t>{0, 1, 2, 1, 3, 2};
        verticesMesh1 = std::vector<float>{-1, 0, 0, 1, -1, 0, 1, 1, 0};
        indicesMesh1 = std::vector<uint32_t>{0, 1, 2};
        expectedResult.vertices = std::vector<float>{0.5, 0, 1, 0.5, 0, 0, -0.5, 0, 0, 0.5, 0, -1};
        expectedResult.indicesOut = std::vector<uint32_t>{0, 1, 2};
        expectedResult.indicesIn = std::vector<uint32_t>{1, 2, 3};
    }
    SECTION("intersection triangle by 2 triangles with 90 degrees") {
        verticesMesh0 = std::vector<float>{-1, 0, 0, 1, 0, 1, 1, 0, -1};
        indicesMesh0 = std::vector<uint32_t>{0, 1, 2};
        SECTION("order 1") {
            verticesMesh1 = std::vector<float>{-1, 0, 0.25, 0.5, 1, 0.25, 0.5, -1, 0.25, 0.5, 0, 1.25};
            indicesMesh1 = std::vector<uint32_t>{0, 1, 2, 1, 3, 2};
            expectedResult.vertices = std::vector<float>{-1, 0, 0, 1, 0, 1, 1, 0, -1, -0.5, 0, 0.25, 0.5, 0, 0.25, 0.5, 0, 0.25, 0.5, 0, 0.75};
            expectedResult.indicesOut = std::vector<uint32_t>{0, 3, 4, 1, 4, 6, 2, 0, 4, 1, 2, 4, };
            expectedResult.indicesIn = std::vector<uint32_t>{3, 5, 6, };
        }
        SECTION("order 2") {
            verticesMesh1 = std::vector<float>{0.5, 0, 1.25, 0.5, -1, 0.25, 0.5, 1, 0.25, -1, 0, 0.25};
            indicesMesh1 = std::vector<uint32_t>{0, 1, 2, 1, 3, 2};
            expectedResult.vertices = std::vector<float>{-1, 0, 0, 1, 0, 1, 1, 0, -1, 0.5, 0, 0.25, 0.5, 0, 0.75, -0.5, 0, 0.25, 0.5, 0, 0.25};
            expectedResult.indicesOut = std::vector<uint32_t>{2, 3, 4, 0, 5, 6, 2, 0, 3, 1, 2, 4, };
            expectedResult.indicesIn = std::vector<uint32_t>{4, 5, 6};
        }
    }
    SECTION("intersection quad by 2 quads with 90 degrees") {
        verticesMesh0 = std::vector<float>{-0.5, 0.5, 0, 0.5, 0.5, 0, -0.5, -0.5, 0, 0.5, -0.5, 0};
        indicesMesh0 = std::vector<uint32_t>{0, 2, 1, 2, 3, 1};
        verticesMesh1 = std::vector<float>{
                0.471869558095932, -0.1469084471464157, 0.38414689898490906, 0.471869558095932, -0.1469084471464157, -0.6158531308174133,
                0.471869558095932, 0.8530915379524231, -0.6158531308174133, 0.471869558095932, 0.8530915379524231, 0.38414689898490906,
                -0.5281304121017456, -0.1469084471464157, -0.6158531308174133, 0.471869558095932, -0.1469084471464157, -0.6158531308174133,
                0.471869558095932, -0.1469084471464157, 0.38414689898490906, -0.5281304121017456, -0.1469084471464157, 0.38414689898490906};
        indicesMesh1 = std::vector<uint32_t>{0, 1, 2, 0, 2, 3, 4, 5, 6, 4, 6, 7};
        expectedResult.vertices = std::vector<float>{
                -0.5, 0.5, 0, 0.5, 0.5, 0, -0.5, -0.5, 0, 0.5, -0.5, 0, 0.4718695878982544, 0.4718695878982544, 0, 0.471869558095932,
                0.5, 0, 0.471869558095932, 0.23723840713500977, 0, 0.471869558095932, -0.1469084471464157, 0, -0.15684112906455994,
                -0.1469084620475769, 0, -0.1469084620475769, -0.1469084620475769, 0, -0.5, -0.1469084620475769, 0, -0.15684106945991516,
                -0.1469084620475769, 0, 0.471869558095932, 0.471869558095932, 0, 0.471869558095932, 0.23723845183849335, 0, 0.471869558095932,
                -0.1469084471464157, 0, 0.08772268891334534, -0.1469084471464157, 2.9802322387695312e-8, -0.14690843224525452,
                -0.14690843224525452, 0, -0.03453469276428223, -0.1469084620475769, 0, 0.08772268891334534, -0.1469084471464157,
                2.9802322387695312e-8, -0.034534722566604614, -0.1469084918498993, 0};
        expectedResult.indicesOut = std::vector<uint32_t>{1, 5, 4, 1, 6, 7, 2, 9, 8, 2, 11, 10, 1, 12, 13, 2, 7, 15, 2, 17, 16, 2, 15, 19, 3, 1, 7, 2, 3, 7, };
        expectedResult.indicesIn = std::vector<uint32_t>{5, 11, 0, 5, 9, 8, 4, 9, 5, 0, 11, 10, 6, 14, 15, 12, 17, 16, 13, 17, 12, 6, 18, 19, };
    }
    SECTION("intersect 2 joined triangles by 1 triangle") {
        verticesMesh0 = std::vector<float>{-1, 0, 1.5, 1, 0, 2, -0.5, 0.5, -1, 1.5, 0, 0.5};
        indicesMesh0 = std::vector<uint32_t>{0, 1, 2, 1, 3, 2};
        verticesMesh1 = std::vector<float>{-0.8, -1, 1, 1.2, -0.5, 1, 0.7, 3, 1};
        indicesMesh1 = std::vector<uint32_t>{0, 1, 2};
        expectedResult.vertices = std::vector<float>{
                -1, 0, 1.5, 1, 0, 2, -0.5, 0.5, -1, 1.5, 0, 0.5, 0.5, 0.16666666666666666, 1, -0.3781818303194915,
                0.12484848427050022, 1, 0.5, 0.16666666666666669, 1, 1.1225490591895897, 0.04215685482874876, 1};
        expectedResult.indicesOut = std::vector<uint32_t>{1, 4, 5, 1, 7, 6, 0, 1, 5, 1, 3, 7, 3, 2, 7, 2, 0, 5, };
        expectedResult.indicesIn = std::vector<uint32_t>{2, 4, 5, 2, 7, 6};
    }
    SECTION("issue 1") {
        verticesMesh0 = std::vector<float>{
                0.4774431586265564, 0.10000000149011612, 0.7145441770553589, 0.6076698899269104, 0.10000000149011612, 0.6076698899269104,
                0.41667768359184265, 0.20000000298023224, 0.6236022114753723, 0.5303300619125366, 0.20000000298023224, 0.5303300619125366};
        indicesMesh0 = std::vector<uint32_t>{0, 1, 3, 0, 3, 2};
        verticesMesh1 = std::vector<float>{
                -0.5048903324633537, 0.10855014497974924, -0.130662588701557, 0.49510966753664626, 0.10855014497974924, -0.130662588701557,
                0.49510966753664626, 0.10855014497974924, 0.869337411298443, 0.49510966753664626, 0.10855014497974924, 0.869337411298443,
                0.49510966753664626, 0.10855014497974924, -0.130662588701557, 0.49510966753664626, 1.1085501449797492, -0.130662588701557};
        indicesMesh1 = std::vector<uint32_t>{0, 1, 2, 3, 4, 5};
        expectedResult.vertices = std::vector<float>{
                0.4774431586265564, 0.10000000149011612, 0.7145441770553589, 0.6076698899269104, 0.10000000149011612, 0.6076698899269104,
                0.41667768359184265, 0.20000000298023224, 0.6236022114753723, 0.5303300619125366, 0.20000000298023224, 0.5303300619125366,
                0.49510966753664626, 0.10855014497974924, 0.6880061122889556, 0.48196506467734956, 0.10855014497974924, 0.6987936061171914,
                0.4722476233963507, 0.10855014497974924, 0.7067685086218438, 0.48196506467734956, 0.10855014497974924, 0.6987936061171914,
                0.49510966753664626, 0.10855014497974924, 0.6880061122889556, 0.49510966753664626, 0.13340431914851938, 0.6530087097783838,
                0.49510966753664626, 0.13340431914851938, 0.6530087097783838, 0.49510966753664626, 0.20000000298023224, 0.5592347090517329};
        expectedResult.indicesOut = std::vector<uint32_t>{1, 4, 5, 0, 7, 6, 3, 9, 8, 3, 11, 10, 1, 3, 4, 0, 1, 5, };
        expectedResult.indicesIn = std::vector<uint32_t>{2, 11, 7, 2, 7, 6, 5, 9, 4, 7, 11, 10, };
    }
    SECTION("intersect arc segment and plane") {
        verticesMesh0 = std::vector<float>{
                0, 0, 0, 1, 0, 0, 0.9238795042037964, 2.3432602348612458e-17, -0.3826834261417389, 0.7071067690849304, 4.329780301713277e-17,
                -0.7071067690849304, 0.3826834261417389, 5.657130560897519e-17, -0.9238795042037964, 6.123234262925839e-17, 6.123234262925839e-17,
                -1, -0.3826834261417389, 5.657130560897519e-17, -0.9238795042037964, -0.7071067690849304, 4.329780301713277e-17,
                -0.7071067690849304, -0.9238795042037964, 2.3432602348612458e-17, -0.3826834261417389, -1, 7.49879952078994e-33, -1.2246468525851679e-16};
        indicesMesh0 = std::vector<uint32_t>{0, 1, 2, 0, 2, 3, 0, 3, 4, 0, 4, 5, 0, 5, 6, 0, 6, 7, 0, 7, 8, 0, 8, 9};
        verticesMesh1 = std::vector<float>{
                -0.683296, 0.755462, -0.385157, 0.316704, 0.755462, -0.385157, -0.683296, -0.244538, -0.385157, 0.316704, -0.244538, -0.385157};
        indicesMesh1 = std::vector<uint32_t>{0, 2, 1, 2, 3, 1};
        expectedResult.vertices = std::vector<float>{
                0, 0, 0, 1, 0, 0, 0.9238795042037964, 2.3432602348612458e-17, -0.3826834261417389, 0.7071067690849304, 4.329780301713277e-17,
                -0.7071067690849304, 0.3826834261417389, 5.657130560897519e-17, -0.9238795042037964, 6.123234262925839e-17, 6.123234262925839e-17,
                -1, -0.3826834261417389, 5.657130560897519e-17, -0.9238795042037964, -0.7071067690849304, 4.329780301713277e-17, -0.7071067690849304,
                -0.9238795042037964, 2.3432602348612458e-17, -0.3826834261417389, -1, 7.49879952078994e-33, -1.2246468525851679e-16, 0.15953731061709203,
                2.3584073253307646e-17, -0.3851571334587516, 0.3167040551082888, 2.7755575615628914e-17, -0.38515713345875163, 2.358407356204928e-17,
                2.358407356204928e-17, -0.3851571334587516, 0.15953731061709203, 2.3584073253307643e-17, -0.38515713345875163, -0.15953731061709203,
                2.3584073253307646e-17, -0.3851571334587516, 2.3584073562049282e-17, 2.3584073562049282e-17, -0.38515713345875163, -0.38515713345875163,
                2.3584073048433145e-17, -0.38515713345875163, -0.15953731061709203, 2.3584073253307643e-17, -0.38515713345875163, -0.4387575023482929,
                2.7755575615628914e-17, -0.38515713345875163, -0.6832959448917112, 0, -0.38515713345875163, -0.4387575023482929, 0, -0.38515713345875163,
                -0.3851571334587517, 2.3584073048433145e-17, -0.3851571334587517};
        expectedResult.indicesOut = std::vector<uint32_t>{
                0, 11, 10, 0, 13, 12, 0, 15, 14, 0, 17, 16, 0, 18, 19, 0, 21, 20, 0, 3, 11, 8, 0, 19, 0, 8, 9, 3, 4, 11, 0, 2, 3, 7, 8, 19, 0, 1, 2, };
        expectedResult.indicesIn = std::vector<uint32_t>{6, 17, 7, 5, 15, 6, 4, 13, 5, 4, 11, 10, 5, 13, 12, 6, 15, 14, 7, 17, 16, 7, 18, 19, 7, 21, 18, };
    }

    MeshData mesh0{
            (uint32_t)verticesMesh0.size() / 3, (uint32_t)indicesMesh0.size(),
            verticesMesh0.data(), nullptr, nullptr,
            indicesMesh0.data(),
            getMin(verticesMesh0), getMax(verticesMesh0)
    };
    MeshData mesh1{
            (uint32_t)verticesMesh1.size() / 3, (uint32_t)indicesMesh1.size(),
            verticesMesh1.data(), nullptr, nullptr,
            indicesMesh1.data(),
            getMin(verticesMesh1), getMax(verticesMesh1)
    };
    MeshIntersection intersection(mesh0, mesh1);
    intersection.intersect();
    auto actualResult = intersection.getResult0();

    REQUIRE(actualResult.vertices.size() % 3 == 0);
    REQUIRE(actualResult.indicesOut.size() % 3 == 0);
    REQUIRE(actualResult.indicesIn.size() % 3 == 0);
    auto [verticesEqual, indicesOuEqual, indicesInEqual] = compareMeshData(expectedResult, actualResult);
    REQUIRE(verticesEqual);
    REQUIRE(indicesOuEqual);
    REQUIRE(indicesInEqual);
}

TEST_CASE("mesh intersection - operator", "[mesh intersection]") {
    std::vector<float> verticesMesh0;
    std::vector<uint32_t> indicesMesh0;
    std::vector<float> verticesMesh1;
    std::vector<uint32_t> indicesMesh1;
    ResultMeshData expectedResult;
    MeshIntersection::Operator op = MeshIntersection::Operator::MINUS;

    SECTION("cube and low resolution sphere") {
        verticesMesh0 = std::vector<float>{0, 0, 0.5, 0.5, 0, 0.5, 0.5, -0.5, 0.5, 0, -0.5, 0.5, 0, 0, 0.5, 0, -0.5, 0.5, 0, -0.5, 0, 0, 0, 0, 0, 0, 0, 0.5, 0, 0, 0.5, 0, 0.5, 0, 0, 0.5, 0.5, -0.5, 0, 0, -0.5, 0, 0, 0, 0, 0.5, 0, 0, 0.5, -0.5, 0, 0.5, 0, 0, 0.5, 0, 0.5, 0.5, -0.5, 0.5, 0.5, -0.5, 0, 0, -0.5, 0, 0, -0.5, 0.5, 0.5, -0.5, 0.5, };
        indicesMesh0 = std::vector<uint32_t>{0, 3, 2, 0, 2, 1, 4, 7, 6, 4, 6, 5, 8, 10, 9, 8, 11, 10, 12, 13, 14, 12, 14, 15, 16, 17, 18, 16, 18, 19, 20, 23, 22, 20, 22, 21, };
        verticesMesh1 = std::vector<float>{0.5, -0.5, 0, 0.5, -0.146447, 0.146447, 0.5, 0, 0.5, 0.5, -0.146447, 0.853553, 0.5, -0.5, 1, 0.5, -0.5, 0, 0.25, -0.25, 0.146447, 0.146447, -0.146447, 0.5, 0.25, -0.25, 0.853553, 0.5, -0.5, 1, 0.5, -0.5, 0, 0.146447, -0.5, 0.146447, 0, -0.5, 0.5, 0.146447, -0.5, 0.853553, 0.5, -0.5, 1, 0.5, -0.5, 0, 0.25, -0.75, 0.146447, 0.146447, -0.853553, 0.5, 0.25, -0.75, 0.853553, 0.5, -0.5, 1, 0.5, -0.5, 0, 0.5, -0.853553, 0.146447, 0.5, -1, 0.5, 0.5, -0.853553, 0.853553, 0.5, -0.5, 1, 0.5, -0.5, 0, 0.75, -0.75, 0.146447, 0.853553, -0.853553, 0.5, 0.75, -0.75, 0.853553, 0.5, -0.5, 1, 0.5, -0.5, 0, 0.853553, -0.5, 0.146447, 1, -0.5, 0.5, 0.853553, -0.5, 0.853553, 0.5, -0.5, 1, 0.5, -0.5, 0, 0.75, -0.25, 0.146447, 0.853553, -0.146447, 0.5, 0.75, -0.25, 0.853553, 0.5, -0.5, 1, 0.5, -0.5, 0, 0.5, -0.146447, 0.146447, 0.5, 0, 0.5, 0.5, -0.146447, 0.853553, 0.5, -0.5, 1, };
        indicesMesh1 = std::vector<uint32_t>{0, 5, 1, 5, 6, 1, 1, 6, 2, 6, 7, 2, 2, 7, 3, 7, 8, 3, 3, 8, 4, 8, 9, 4, 5, 10, 6, 10, 11, 6, 6, 11, 7, 11, 12, 7, 7, 12, 8, 12, 13, 8, 8, 13, 9, 13, 14, 9, 10, 15, 11, 15, 16, 11, 11, 16, 12, 16, 17, 12, 12, 17, 13, 17, 18, 13, 13, 18, 14, 18, 19, 14, 15, 20, 16, 20, 21, 16, 16, 21, 17, 21, 22, 17, 17, 22, 18, 22, 23, 18, 18, 23, 19, 23, 24, 19, 20, 25, 21, 25, 26, 21, 21, 26, 22, 26, 27, 22, 22, 27, 23, 27, 28, 23, 23, 28, 24, 28, 29, 24, 25, 30, 26, 30, 31, 26, 26, 31, 27, 31, 32, 27, 27, 32, 28, 32, 33, 28, 28, 33, 29, 33, 34, 29, 30, 35, 31, 35, 36, 31, 31, 36, 32, 36, 37, 32, 32, 37, 33, 37, 38, 33, 33, 38, 34, 38, 39, 34, 35, 40, 36, 40, 41, 36, 36, 41, 37, 41, 42, 37, 37, 42, 38, 42, 43, 38, 38, 43, 39, 43, 44, 39, };
        SECTION("or") {
            op = MeshIntersection::Operator::OR;
            expectedResult.vertices = std::vector<float>{0, 0, 0.5, 0, -0.5, 0.5, 0.146447, -0.146447, 0.5, 0.146447, -0.146447, 0.5, 0.5, 0, 0.5, 0.5, 0, 0, 0.5, 0, 0.5, 0.5, -0.146447, 0.146447, 0.5, -0.5, 0, 0, -0.5, 0, 0.5, -0.5, 0, 0.146447, -0.5, 0.146447, 0, -0.5, 0.5, 0.146447, -0.5, 0.146447, 0, 0, 0.5, 0, -0.5, 0, 0, -0.5, 0.5, 0, 0, 0, 0.5, 0, 0.5, 0.5, 0, 0, 0, 0, 0.5, 0.5, -0.5, 0, 0, -0.5, 0, 0, 0, 0, 0.5, 0, 0, 0, 0, 0, 0.5, 0, 0.5, 0.146447, -0.146447, 0.5, 0.5, -0.146447, 0.853553, 0, -0.5, 0.5, 0.25, -0.25, 0.853553, 0.5, -0.5, 0, 0.25, -0.75, 0.146447, 0.146447, -0.5, 0.146447, 0.5, -0.5, 0, 0.5, -0.146447, 0.146447, 0.75, -0.25, 0.146447, 0.5, 0, 0.5, 0.853553, -0.146447, 0.5, 0.146447, -0.5, 0.853553, 0.146447, -0.853553, 0.5, 0.5, -0.5, 0, 0.5, -0.853553, 0.146447, 0.5, -0.5, 0, 0.853553, -0.5, 0.146447, 0.75, -0.25, 0.853553, 0.5, -0.146447, 0.853553, 0.5, -0.5, 1, 0.5, -0.5, 0, 0.75, -0.75, 0.146447, 0.5, -0.5, 0, 0.5, -0.5, 1, 1, -0.5, 0.5, 0.853553, -0.5, 0.853553, 0.5, -0.5, 1, 0.5, -1, 0.5, 0.853553, -0.853553, 0.5, 0.25, -0.75, 0.853553, 0.5, -0.5, 1, 0.5, -0.5, 1, 0.75, -0.75, 0.853553, 0.5, -0.5, 1, 0.5, -0.853553, 0.853553, 0.5, -0.5, 1, 0.5, -0.5, 1, };
            expectedResult.indicesOut = std::vector<uint32_t>{0, 1, 2, 0, 3, 4, 5, 6, 7, 8, 5, 7, 9, 10, 11, 12, 9, 13, 14, 15, 16, 17, 18, 19, 17, 20, 18, 21, 22, 23, 21, 23, 24, 14, 25, 15, 26, 27, 28, 27, 29, 30, 31, 32, 33, 33, 32, 29, 34, 35, 36, 35, 37, 38, 27, 30, 28, 29, 39, 30, 32, 40, 29, 41, 42, 32, 43, 36, 44, 36, 35, 38, 38, 37, 45, 37, 46, 45, 28, 30, 47, 32, 42, 40, 48, 49, 42, 50, 44, 49, 30, 39, 51, 44, 36, 52, 36, 38, 52, 38, 45, 53, 29, 40, 39, 45, 46, 54, 42, 55, 40, 49, 44, 56, 44, 52, 56, 52, 38, 53, 39, 57, 58, 53, 45, 59, 42, 49, 55, 40, 57, 39, 60, 53, 61, 40, 55, 57, 57, 62, 63, 56, 52, 60, 52, 53, 60, 49, 56, 55, 62, 60, 64, 55, 62, 57, 55, 56, 62, 56, 60, 62, };
        }
        SECTION("and") {
            op = MeshIntersection::Operator::AND;
            expectedResult.vertices = std::vector<float>{0.5, -0.5, 0, 0.5, -0.5, 0.5, 0.5, 0, 0.5, 0.5, -0.5, 0, 0, -0.5, 0.5, 0.5, -0.5, 0.5, 0.5, -0.5, 0.5, 0, -0.5, 0.5, 0.146447, -0.146447, 0.5, 0.146447, -0.146447, 0.5, 0.5, 0, 0.5, 0.5, -0.146447, 0.146447, 0.146447, -0.5, 0.146447, 0.25, -0.25, 0.146447, 0.146447, -0.146447, 0.5, 0.146447, -0.5, 0.146447, 0.5, -0.5, 0, 0.5, -0.146447, 0.146447, 0.5, 0, 0.5, 0.5, -0.5, 0, 0, -0.5, 0.5, };
            expectedResult.indicesOut = std::vector<uint32_t>{0, 2, 1, 3, 5, 4, 6, 8, 7, 6, 10, 9, 2, 0, 11, 3, 4, 12, 13, 15, 14, 16, 13, 17, 17, 13, 18, 13, 14, 18, 19, 15, 13, 15, 20, 14, };
        }
        SECTION("minus") {
            op = MeshIntersection::Operator::MINUS;
            expectedResult.vertices = std::vector<float>{0, 0, 0.5, 0, -0.5, 0.5, 0.146447, -0.146447, 0.5, 0.146447, -0.146447, 0.5, 0.5, 0, 0.5, 0.5, 0, 0, 0.5, 0, 0.5, 0.5, -0.146447, 0.146447, 0.5, -0.5, 0, 0, -0.5, 0, 0.5, -0.5, 0, 0.146447, -0.5, 0.146447, 0, -0.5, 0.5, 0.146447, -0.5, 0.146447, 0, 0, 0.5, 0, -0.5, 0, 0, -0.5, 0.5, 0, 0, 0, 0.5, 0, 0.5, 0.5, 0, 0, 0, 0, 0.5, 0.5, -0.5, 0, 0, -0.5, 0, 0, 0, 0, 0.5, 0, 0, 0, 0, 0, 0.25, -0.25, 0.146447, 0.146447, -0.146447, 0.5, 0.146447, -0.5, 0.146447, 0.5, -0.5, 0, 0.5, -0.146447, 0.146447, 0.5, 0, 0.5, 0.5, -0.5, 0, 0, -0.5, 0.5, };
            expectedResult.indicesOut = std::vector<uint32_t>{0, 1, 2, 0, 3, 4, 5, 6, 7, 8, 5, 7, 9, 10, 11, 12, 9, 13, 14, 15, 16, 17, 18, 19, 17, 20, 18, 21, 22, 23, 21, 23, 24, 14, 25, 15, 26, 27, 28, 29, 30, 26, 30, 31, 26, 26, 31, 27, 32, 26, 28, 28, 27, 33, };
        }
    }

    MeshData mesh0{
            (uint32_t)verticesMesh0.size() / 3, (uint32_t)indicesMesh0.size(),
            verticesMesh0.data(), nullptr, nullptr,
            indicesMesh0.data(),
            getMin(verticesMesh0), getMax(verticesMesh0)
    };
    MeshData mesh1{
            (uint32_t)verticesMesh1.size() / 3, (uint32_t)indicesMesh1.size(),
            verticesMesh1.data(), nullptr, nullptr,
            indicesMesh1.data(),
            getMin(verticesMesh1), getMax(verticesMesh1)
    };
    MeshIntersection intersection(mesh0, mesh1);
    REQUIRE(intersection.operate(op));
    auto actualResult = intersection.getResult0();

    REQUIRE(actualResult.vertices.size() % 3 == 0);
    REQUIRE(actualResult.indicesOut.size() % 3 == 0);
    REQUIRE(actualResult.indicesIn.size() % 3 == 0);
    auto [verticesEqual, indicesOuEqual, indicesInEqual] = compareMeshData(expectedResult, actualResult);
    REQUIRE(verticesEqual);
    REQUIRE(indicesOuEqual);
    REQUIRE(indicesInEqual);
}
