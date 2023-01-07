#include "mesh_utility/meshMath.h"
#include "meshIntersectionTestHelper.h"
#include "catch2/catch.hpp"

using namespace mesh;

TEST_CASE("mesh intersection - negate vector", "[mesh intersection]") {
    SECTION("0") {
        REQUIRE(negate3(Coordinates3{0, 0, 0}) == Coordinates3{0, 0, 0});
    }
    SECTION("1") {
        REQUIRE(negate3(Coordinates3{1, 0, 0}) == Coordinates3{-1, 0, 0});
        REQUIRE(negate3(Coordinates3{0, 1, 0}) == Coordinates3{0, -1, 0});
        REQUIRE(negate3(Coordinates3{0, 0, 1}) == Coordinates3{0, 0, -1});
        REQUIRE(negate3(Coordinates3{1, 1, 1}) == Coordinates3{-1, -1, -1});
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

TEST_CASE("mesh intersection - lerp", "[mesh intersection]") {
    SECTION("lerp 2") {
        SECTION("0") {
            REQUIRE(lerp2(Coordinates2{0, 0}, Coordinates2{0, 0}, 0) == Coordinates2{0, 0});
            REQUIRE(lerp2(Coordinates2{0, 0}, Coordinates2{0, 0}, 0.5f) == Coordinates2{0, 0});
            REQUIRE(lerp2(Coordinates2{0, 0}, Coordinates2{0, 0}, 1) == Coordinates2{0, 0});
        }
        SECTION("1") {
            REQUIRE(lerp2(Coordinates2{0, 0}, Coordinates2{0, 1}, 0) == Coordinates2{0, 0});
            REQUIRE(lerp2(Coordinates2{0, 0}, Coordinates2{0, 1}, 0.5f) == Coordinates2{0, 0.5});
            REQUIRE(lerp2(Coordinates2{0, 0}, Coordinates2{0, 1}, 1) == Coordinates2{0, 1});
            REQUIRE(lerp2(Coordinates2{1, 0}, Coordinates2{0, 0}, 0) == Coordinates2{1, 0});
            REQUIRE(lerp2(Coordinates2{1, 0}, Coordinates2{0, 0}, 0.5f) == Coordinates2{0.5, 0});
            REQUIRE(lerp2(Coordinates2{1, 0}, Coordinates2{0, 0}, 1) == Coordinates2{0, 0});
        }
    }
    SECTION("lerp 3") {
        SECTION("0") {
            REQUIRE(lerp3(Coordinates3{0, 0, 0}, Coordinates3{0, 0, 0}, 0) == Coordinates3{0, 0, 0});
            REQUIRE(lerp3(Coordinates3{0, 0, 0}, Coordinates3{0, 0, 0}, 0.5f) == Coordinates3{0, 0, 0});
            REQUIRE(lerp3(Coordinates3{0, 0, 0}, Coordinates3{0, 0, 0}, 1) == Coordinates3{0, 0, 0});
        }
        SECTION("1") {
            REQUIRE(lerp3(Coordinates3{0, 0, 0}, Coordinates3{1, 0, 0}, 0) == Coordinates3{0, 0, 0});
            REQUIRE(lerp3(Coordinates3{0, 0, 0}, Coordinates3{0, 1, 0}, 0.5f) == Coordinates3{0, 0.5, 0});
            REQUIRE(lerp3(Coordinates3{0, 0, 0}, Coordinates3{0, 0, 1}, 1) == Coordinates3{0, 0, 1});
            REQUIRE(lerp3(Coordinates3{0, 0, 1}, Coordinates3{0, 0, 0}, 0) == Coordinates3{0, 0, 1});
            REQUIRE(lerp3(Coordinates3{0, 1, 0}, Coordinates3{0, 0, 0}, 0.5f) == Coordinates3{0, 0.5, 0});
            REQUIRE(lerp3(Coordinates3{1, 0, 0}, Coordinates3{0, 0, 0}, 1) == Coordinates3{0, 0, 0});
        }
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
            auto p = PlanePtNv::fromTriangle(Coordinates3{0, 0, 0}, Coordinates3{1, 0, 0}, Coordinates3{0, 1, 0});
            REQUIRE(p.pointOnPlane == Coordinates3{0, 0, 0});
            REQUIRE(p.normal == Coordinates3{0, 0, 1});
        }
        SECTION ("XZ") {
            auto p = PlanePtNv::fromTriangle(Coordinates3{0, 0, 0}, Coordinates3{1, 0, 0}, Coordinates3{0, 0, 1});
            REQUIRE(p.pointOnPlane == Coordinates3{0, 0, 0});
            REQUIRE(p.normal == Coordinates3{0, -1, 0});
        }
        SECTION ("YZ") {
            auto p = PlanePtNv::fromTriangle(Coordinates3{0, 0, 0}, Coordinates3{0, 1, 0}, Coordinates3{0, 0, 1});
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
        auto p = PlanePtNv{Coordinates3{-4, 0, 0},  Coordinates3{0, 0, 5}};
        auto i = intersectRayAndPlane(r, p);
        REQUIRE(i.valid);
        REQUIRE(i.distance == Approx(2).epsilon(0.001f));
        REQUIRE(i.point == Coordinates3{0, 0, 0});
    }
    SECTION("YZ") {
        auto r = Ray{Coordinates3{1, -4, 1}, Coordinates3{0, 1, 0}};
        auto p = PlanePtNv{Coordinates3{0, 0, 5},  Coordinates3{0, -3, 0}};
        auto i = intersectRayAndPlane(r, p);
        REQUIRE(i.valid);
        REQUIRE(i.distance == Approx(4).epsilon(0.001f));
        REQUIRE(i.point == Coordinates3{1, 0, 1});
    }
    SECTION("XZ") {
        auto r = Ray{Coordinates3{7, 1, 0}, Coordinates3{-1, 0, 0}};
        auto p = PlanePtNv{Coordinates3{0, 0, 3},  Coordinates3{1, 0, 0}};
        auto i = intersectRayAndPlane(r, p);
        REQUIRE(i.valid);
        REQUIRE(i.distance == Approx(7).epsilon(0.001f));
        REQUIRE(i.point == Coordinates3{0, 1, 0});
    }
    SECTION("diagonal") {
        auto r = Ray{Coordinates3{1, 1, 1}, Coordinates3{1, 1, -1}};
        auto p = PlanePtNv{Coordinates3{0, 0, 0},  Coordinates3{0, 0, 1}};
        auto i = intersectRayAndPlane(r, p);
        REQUIRE(i.valid);
        REQUIRE(i.distance == Approx(1).epsilon(0.001f));
        REQUIRE(i.point == Coordinates3{2, 2, 0});
    }
    SECTION("invalid") {
        auto r = Ray{Coordinates3{1, 1, 1}, Coordinates3{1, 1, 0}};
        auto p = PlanePtNv{Coordinates3{0, 0, 0},  Coordinates3{0, 0, 1}};
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