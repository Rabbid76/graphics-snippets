cmake_minimum_required(VERSION 3.12)
set(CMAKE_CXX_STANDARD 20)
project(MeshUtilityTestSuite)

include_directories (
    "./"
    "../"
    "../../wasm/"
    )

file(GLOB MeshUtilityTestFiles
    "*/*.cpp"
    "*/*.h"
    )

set(
    UtilitySourceFiles
    ../mesh_utility/constructiveSolidGeometry.cpp
    ../mesh_utility/meshMath.cpp
    ../mesh_utility/meshIntersection.cpp
    ../mesh_utility/meshSpecification.cpp
    )

add_executable(
    MeshUtilityTest
    ${UtilitySourceFiles}
    ${MeshUtilityTestFiles}
    )

enable_testing()

add_test(NAME MeshUtilityTest COMMAND MeshUtilityTest "MeshUtilityTest")