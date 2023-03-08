#ifndef __MESH_INTERSECTION__H__
#define __MESH_INTERSECTION__H__

#include "meshMath.h"
#include "meshSpecification.h"
#include <array>
#include <cmath>
#include <vector>
#include <unordered_map>
#include <unordered_set>

namespace mesh {

    class TrianglePlaneIntersection {
    public:
        uint32_t peakIndex;
        Intersection intersectToPeak;
        Intersection intersectFromPeak;

        static TrianglePlaneIntersection Invalid() {
            return {
                    .peakIndex = 0xffffffff,
                    .intersectToPeak = Intersection::Invalid(),
                    .intersectFromPeak = Intersection::Invalid(),
            };
        }
    };

    struct TriangleIntersectionPoint {
        uint32_t triangleIndex;
        Intersection intersection;
        float distance;
    };

    class TriangleTriangleIntersection {
    public:
        bool valid;
        TrianglePlaneIntersection intersectionTriangle0;
        TrianglePlaneIntersection intersectionTriangle1;

        static TriangleTriangleIntersection Invalid() {
            return {
                    .valid = false,
                    .intersectionTriangle0 = TrianglePlaneIntersection::Invalid(),
                    .intersectionTriangle1 = TrianglePlaneIntersection::Invalid(),
            };
        }
    };

    TrianglePlaneIntersection intersectTriangleAndPlane( const float p0[], const float p1[], const float p2[], const PlanePtNv &plane);
    TriangleTriangleIntersection intersectTriangles(const float t0p0[], const float t0p1[], const float t0p2[], const float t1p0[], const float t1p1[], const float t1p2[]);

    class Triangle {
    public:
        enum class State {
            UNKNOWN,
            NOT_INTERSECTING,
            INTERSECTING,
            FINAL,
        };

        std::array<uint32_t, 3> indices;
        std::array<uint32_t, 3> uniqueIndices;
        Vector3 faceNormal;
        Point3 aabbMin;
        Point3 aabbMax;
        int winding = 0;
        State state = State::UNKNOWN;

        Triangle(const Triangle &triangle) = default;
        Triangle(Triangle &&triangle) = default;
        Triangle& operator =(const Triangle &triangle) = default;
        Triangle& operator =(Triangle &&triangle) = default;

        Triangle(const float *vertices, const uint32_t *indices, UniqueIndices::IndexMap &indexMap)
                : indices({indices[0], indices[1], indices[2]}) {
            updateFaceNormal(vertices);
            updateAABB(vertices);
            updateUniqueIndices(indexMap);
        }

        Triangle(const float *vertices, const std::array<uint32_t, 3> &indices, const std::array<uint32_t, 3> &uniqueIndices, UniqueIndices::IndexMap &indexMap)
                : indices(indices)
                , uniqueIndices(uniqueIndices) {
            updateFaceNormal(vertices);
            updateAABB(vertices);
            updateUniqueIndices(indexMap);
        }

        bool hasNoArea() const {
            return uniqueIndices[0] == uniqueIndices[1] || uniqueIndices[0] == uniqueIndices[2] || uniqueIndices[1] == uniqueIndices[2];
        }

        bool isFinalTriangle() const {
            return state == State::NOT_INTERSECTING || state == State::FINAL;
        }

        const float * getVertex(const float *vertices, uint32_t triangleCornerIndex) const {
            return  vertices + indices[triangleCornerIndex] * 3;
        }

        Vector3 edge(const float *vertices, uint32_t i) const {
            const auto v0 = vertices + indices[i] * 3;
            const auto v1 = vertices + indices[(i+1) % 3] * 3;
            return sub3(v1, v0);
        }

        Ray createRay(const float *vertices, uint32_t fromTriangleCornerIndex, uint32_t toTriangleCornerIndex) const {
            return Ray::fromPoints(getVertex(vertices, fromTriangleCornerIndex), getVertex(vertices, toTriangleCornerIndex));
        }

        void updateFaceNormal(const float *vertices) {
            const auto v0 = vertices + indices[0] * 3;
            const auto v1 = vertices + indices[1] * 3;
            const auto v2 = vertices + indices[2] * 3;
            faceNormal = cross3(sub3(v1, v0), sub3(v2, v0));
        }

        void updateAABB(const float *vertices) {
            const auto v0 = vertices + indices[0] * 3;
            const auto v1 = vertices + indices[1] * 3;
            const auto v2 = vertices + indices[2] * 3;
            for (uint32_t i = 0; i < 3; ++i) {
                aabbMin[i] = std::min(v0[i], std::min(v1[i], v2[i]));
                aabbMax[i] = std::max(v0[i], std::max(v1[i], v2[i]));
            }
        }

        void updateUniqueIndices(UniqueIndices::IndexMap &indexMap) {
            uniqueIndices = {indexMap[indices[0]], indexMap[indices[1]], indexMap[indices[2]]};
        }

        [[nodiscard]] bool isIntersectingAABB(const Point3 &testAabbMin, const Point3 &testAabbMax) const {
            return aabbMin[0] <= testAabbMax[0] && testAabbMin[0] <= aabbMax[0] &&
                   aabbMin[1] <= testAabbMax[1] && testAabbMin[1] <= aabbMax[1] &&
                   aabbMin[2] <= testAabbMax[2] && testAabbMin[2] <= aabbMax[2];
        }

        [[nodiscard]] bool isIntersectingAABB(const Triangle &otherTriangle) const {
            return isIntersectingAABB(otherTriangle.aabbMin, otherTriangle.aabbMax);
        }
    };

    class MeshIntersection {
    public:
        using TriangleContainer = std::vector<Triangle>;

    private:
        MeshDataReference mesh0;
        MeshDataReference mesh1;
        UniqueIndices::IndexMap uniqueIndexMapOfMesh0;
        UniqueIndices::IndexMap uniqueIndexMapOfMesh1;
        TriangleContainer trianglesOfMesh0;
        TriangleContainer trianglesOfMesh1;
        uint32_t noOfOriginalTrianglesMesh0 = 0;
        uint32_t noOfOriginalTrianglesMesh1 = 0;
        std::vector<uint32_t> intersectingTrianglesOfMesh0;
        std::vector<uint32_t> intersectingTrianglesOfMesh1;
        MeshDataInstance result0;
        UniqueIndices uniqueIndices;
        bool intersectionBoxValid = false;
        Point3 intersectionAABBMin{0, 0, 0};
        Point3 intersectionAABBMax{0, 0, 0};

    public:
        MeshIntersection(const MeshDataReference &mesh0, const MeshDataReference &mesh1);
        [[nodiscard]] const MeshDataInstance &getResult0() const { return result0; }
        bool intersect(float epsilon = DEFAULT_EPSILON);
        bool operate(Operator meshOperator, float epsilon = DEFAULT_EPSILON);
    private:
        bool prepareIntersectionOfTriangles(float epsilon = DEFAULT_EPSILON);
        void intersectTrianglesOfMeshes(float epsilon = DEFAULT_EPSILON);
        void updateWindingOrderOfResultingTriangles();
        static void appendMesh(MeshDataInstance &target, const MeshDataInstance &source, bool outSide, bool invertNormals);
        bool calculateIntersectionBox();
        void createTriangles(const MeshDataReference &mesh, UniqueIndices::IndexMap &indexMap, TriangleContainer &trianglesOfMesh);
        static void setTriangleStates(TriangleContainer &trianglesOfMesh, std::vector<uint32_t> &intersectingTrianglesOfMesh);
        static std::tuple<std::vector<uint32_t>, std::vector<uint32_t>> getSharedCorners(const std::array<uint32_t, 3> &uniqueIndicesTriangle0, const std::array<uint32_t, 3> &uniqueIndicesTriangle1);
        static std::tuple<Vector3, Vector2> calculateNormalAndUVForPointInTriangle(const float p[], const uint32_t triangleIndices[], const MeshDataInstance &mesh);
        uint32_t addVertexToResult(const float p[], uint32_t uniqueIndex, const uint32_t triangleIndices[]);
        void addTriangleToResultTestArea(const Vector3 &intersectionDirection, std::array<uint32_t, 3> indices);
        void addTriangleToResultWithWindingTestArea(int winding, std::array<uint32_t, 3> indices);
        void addTriangleToResult(const float intersectionDirection[], const Triangle &triangleTouchingEdge12, const float *vertices);
        void addTriangleToResult(int winding, const Triangle &triangleTouchingEdge12);
        static std::array<uint32_t, 3> createUniqueIndices(const std::array<uint32_t, 3> &indices, UniqueIndices::IndexMap &indexMap) {
            return {indexMap[indices[0]], indexMap[indices[1]], indexMap[indices[2]]};
        }
        static bool createAndEvaluateUniqueIndices(const std::array<uint32_t, 3> &indices, UniqueIndices::IndexMap &indexMap, std::array<uint32_t, 3> &uniqueIndices) {
            uniqueIndices = {indexMap[indices[0]], indexMap[indices[1]], indexMap[indices[2]]};
            return std::unordered_set<uint32_t>(uniqueIndices.begin(), uniqueIndices.end()).size() == 3;
        }
    };
}

#endif