#include "mesh_utility/meshSpecification.h"
#include "catch2/catch.hpp"
#include <cmath>
#include <iostream>

using namespace mesh;

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
        REQUIRE(uniqueIndices.getVertexIndex(std::array<float, 3>{0, 0, 0}.data()) == 0);
        REQUIRE(uniqueIndices.getVertexIndex(std::array<float, 3>{1, 0, 0}.data()) == 1);
        REQUIRE(uniqueIndices.getVertexIndex(std::array<float, 3>{0, 1, 0}.data()) == 2);
        REQUIRE(uniqueIndices.getVertexIndex(std::array<float, 3>{1, 1, 0}.data()) == 3);
        REQUIRE(uniqueIndices.getVertexIndex(std::array<float, 3>{0, 0, 1}.data()) == 4);
        REQUIRE(uniqueIndices.getVertexIndex(std::array<float, 3>{1, 0, 1}.data()) == 5);
        REQUIRE(uniqueIndices.getVertexIndex(std::array<float, 3>{0, 1, 1}.data()) == 6);
        REQUIRE(uniqueIndices.getVertexIndex(std::array<float, 3>{1, 1, 1}.data()) == 7);
        REQUIRE(uniqueIndices.getVertexIndex(std::array<float, 3>{0, 0, 0}.data()) == 0);
        REQUIRE(uniqueIndices.getVertexIndex(std::array<float, 3>{1, 0, 0}.data()) == 1);
        REQUIRE(uniqueIndices.getVertexIndex(std::array<float, 3>{0, 1, 0}.data()) == 2);
        REQUIRE(uniqueIndices.getVertexIndex(std::array<float, 3>{1, 1, 0}.data()) == 3);
        REQUIRE(uniqueIndices.getVertexIndex(std::array<float, 3>{0, 0, 1}.data()) == 4);
        REQUIRE(uniqueIndices.getVertexIndex(std::array<float, 3>{1, 0, 1}.data()) == 5);
        REQUIRE(uniqueIndices.getVertexIndex(std::array<float, 3>{0, 1, 1}.data()) == 6);
        REQUIRE(uniqueIndices.getVertexIndex(std::array<float, 3>{1, 1, 1}.data()) == 7);
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