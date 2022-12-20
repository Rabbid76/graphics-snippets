#ifndef __MESH_INTERSECTION__H__
#define __MESH_INTERSECTION__H__

#include <array>
#include <cmath>
#include <vector>
#include <unordered_map>
#include <unordered_set>

namespace mesh {

    constexpr float DEFAULT_EPSILON = 0.0001f;
    constexpr float UNIT_FLOAT_EPSILON = 1.0e-6;

    class Coordinates2 : public std::array<float, 2> {
    public:
        operator const float *() const {
            return data();
        }
    };

    class Coordinates3 : public std::array<float, 3> {
    public:
        operator const float *() const {
            return data();
        }
    };

    using Point3 = Coordinates3;
    using Vector2 = Coordinates2;
    using Vector3 = Coordinates3;

    inline Vector3 add3(const float a[], const float b[]) {
        return {a[0] + b[0], a[1] + b[1], a[2] + b[2]};
    }

    inline Vector3 sub3(const float a[], const float b[]) {
        return {a[0] - b[0], a[1] - b[1], a[2] - b[2]};
    }

    inline Vector3 multiply3scalar(const float a[], float s) {
        return {a[0] * s, a[1] * s, a[2] * s};
    }

    inline float dot3(const float a[], const float b[]) {
        return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
    }

    inline Vector3 cross3(const float a[], const float b[]) {
        return {a[1] * b[2] - a[2] * b[1], a[2] * b[0] - a[0] * b[2], a[0] * b[1] - a[1] * b[0]};
    }

    inline float squareLength(const float v[]) {
        return v[0] * v[0] + v[1] * v[1] + v[2] * v[2];
    }

    inline float length(const float v[]) {
        return std::sqrt(squareLength(v));
    }

    inline Vector3 normalize(const float v[]) {
        auto euclideanLength = length(v);
        return {v[0] / euclideanLength, v[1] / euclideanLength, v[2] / euclideanLength};
    }

    class Ray {
    public:
        Point3 origin;
        Vector3 direction;
        static Ray fromPoints(const float p0[], const float p1[]) {
            return {
                    .origin = {p0[0], p0[1], p0[2]},
                    .direction = sub3(p1, p0)
            };
        }
    };

    class Plane {
    public:
        Point3 pointOnPlane;
        Vector3 normal;

        static Plane fromTriangle(const float p0[], const float p1[], const float p2[]) {
            return {
                    .pointOnPlane = {p0[0], p0[1], p0[2]},
                    .normal = cross3(sub3(p1, p0), sub3(p2, p0))
            };
        }
    };

    class Intersection {
    public:
        bool valid;
        float distance;
        Point3 point;

        static Intersection Invalid() {
            return {
                    .valid = false,
                    .distance = 0,
                    .point = Point3()
            };
        }
    };

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

    Point3 barycentricCoordinateFromVectors(const float ax[], const float ab[], const float ac[]);

    inline Vector2 interpolateBarycentric2(const float barycentricCoordinate[], const float p0[], const float p1[], const float p2[]) {
        return {
                p0[0] * barycentricCoordinate[0] + p1[0] * barycentricCoordinate[1] + p2[0] * barycentricCoordinate[2],
                p0[1] * barycentricCoordinate[0] + p1[1] * barycentricCoordinate[1] + p2[1] * barycentricCoordinate[2],
        };
    }

    inline Vector3 interpolateBarycentric3(const float barycentricCoordinate[], const float p0[], const float p1[], const float p2[]) {
        return {
                p0[0] * barycentricCoordinate[0] + p1[0] * barycentricCoordinate[1] + p2[0] * barycentricCoordinate[2],
                p0[1] * barycentricCoordinate[0] + p1[1] * barycentricCoordinate[1] + p2[1] * barycentricCoordinate[2],
                p0[2] * barycentricCoordinate[0] + p1[2] * barycentricCoordinate[1] + p2[2] * barycentricCoordinate[2],
        };
    }

    inline Intersection intersectRayAndPlane(const Ray &ray, const Plane &plane) {
        if (std::fabs(dot3(ray.direction, plane.normal)) < 1.0e-6)
            return Intersection::Invalid();
        auto t = dot3(sub3(plane.pointOnPlane, ray.origin), plane.normal) / dot3(ray.direction, plane.normal);
        return {
                .valid = true,
                .distance = t,
                .point = add3(ray.origin, multiply3scalar(ray.direction, t))
        };
    }

    bool areEndlessRayAndBoxIntersecting(const Ray &ray, const float aabbMin[], const float aabbMax[]);

    inline bool isPointOnSameSide(const float px[], const float p0[], const float a[], const float b[]) {
        auto cp1 = cross3(sub3(b, a), sub3(px, a));
        auto cp2 = cross3(sub3(b, a), sub3(p0, a));
        return dot3(cp1, cp2) >= 0;
    }

    inline bool isPointInOrOnTriangle(const float px[], const float p0[], const float p1[], const float p2[]) {
        return
                isPointOnSameSide(px, p0, p1, p2) &&
                isPointOnSameSide(px, p1, p2, p0) &&
                isPointOnSameSide(px, p2, p0, p1);
    }

    Intersection intersectRayAndTriangle(const Ray &ray, const float p0[], const float p1[], const float p2[], float maximumDistance);
    TrianglePlaneIntersection intersectTriangleAndPlane( const float p0[], const float p1[], const float p2[], const Plane &plane);
    TriangleTriangleIntersection intersectTriangles(const float t0p0[], const float t0p1[], const float t0p2[], const float t1p0[], const float t1p1[], const float t1p2[]);

    class MeshData {
    public:
        uint32_t noOfVertices;
        uint32_t noOfIndices;
        const float *vertices = nullptr;
        const float *normals = nullptr;
        const float *uvs = nullptr;
        const uint32_t *indices = nullptr;
        Point3 aabbMin;
        Point3 aabbMax;
    };

    class ResultMeshData {
    public:
        std::vector<float> vertices;
        std::vector<float> normals;
        std::vector<float> uvs;
        std::vector<uint32_t> indicesOut;
        std::vector<uint32_t> indicesIn;
        bool error = false;
    };

    class UniqueIndices {
    public:
        using DistanceMap = std::unordered_map<float, uint32_t>;
        using PlaneMap = std::unordered_map<float, DistanceMap>;
        using VolumeMap = std::unordered_map<float, PlaneMap>;
        using IndexMap = std::vector<uint32_t>;

        uint32_t nextIndex = 0;
        VolumeMap vertexToIndexMap;

        uint32_t getIndex(float x, float y, float z);
        uint32_t getVertexIndex(const float v[], float epsilon = DEFAULT_EPSILON);
        IndexMap createUniqueIndices(const float *vertices, uint32_t noOfVertices, float epsilon = DEFAULT_EPSILON);
    };

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
        enum class Operator { MINUS, OR, AND };

    private:
        MeshData mesh0;
        MeshData mesh1;
        UniqueIndices::IndexMap uniqueIndexMapOfMesh0;
        UniqueIndices::IndexMap uniqueIndexMapOfMesh1;
        TriangleContainer trianglesOfMesh0;
        TriangleContainer trianglesOfMesh1;
        uint32_t noOfOriginalTrianglesMesh0 = 0;
        uint32_t noOfOriginalTrianglesMesh1 = 0;
        std::vector<uint32_t> intersectingTrianglesOfMesh0;
        std::vector<uint32_t> intersectingTrianglesOfMesh1;
        ResultMeshData result0;
        UniqueIndices uniqueIndices;
        bool intersectionBoxValid = false;
        Point3 intersectionAABBMin{0, 0, 0};
        Point3 intersectionAABBMax{0, 0, 0};

    public:
        MeshIntersection(const MeshData &mesh0, const MeshData &mesh1);
        [[nodiscard]] const ResultMeshData &getResult0() const { return result0; }
        bool intersect(float epsilon = DEFAULT_EPSILON);
        bool operate(Operator meshOperator, float epsilon = DEFAULT_EPSILON);
    private:
        bool prepareIntersectionOfTriangles(float epsilon = DEFAULT_EPSILON);
        void intersectTrianglesOfMeshes(float epsilon = DEFAULT_EPSILON);
        void updateWindingOrderOfResultingTriangles();
        static void appendMesh(ResultMeshData &target, const ResultMeshData &source, bool outSide, bool invertNormals);
        bool calculateIntersectionBox();
        void createTriangles(const MeshData &mesh, UniqueIndices::IndexMap &indexMap, TriangleContainer &trianglesOfMesh);
        static void setTriangleStates(TriangleContainer &trianglesOfMesh, std::vector<uint32_t> &intersectingTrianglesOfMesh);
        static std::tuple<std::vector<uint32_t>, std::vector<uint32_t>> getSharedCorners(const std::array<uint32_t, 3> &uniqueIndicesTriangle0, const std::array<uint32_t, 3> &uniqueIndicesTriangle1);
        static std::tuple<Vector3, Vector2> calculateNormalAndUVForPointInTriangle(const float p[], const uint32_t triangleIndices[], const ResultMeshData &mesh);
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