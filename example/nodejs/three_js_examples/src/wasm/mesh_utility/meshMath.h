#ifndef __MESH_MATH__H__
#define __MESH_MATH__H__

#include <array>
#include <cmath>
#include <cstdint>

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

    inline Vector3 negate3(const float a[]) {
        return {-a[0], -a[1], -a[2]};
    }

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

    inline Vector2 lerp2(const float a[], const float b[], float t) {
        return {a[0] + (b[0] - a[0]) * t, a[1] + (b[1] - a[1]) * t};
    }

    inline Vector3 lerp3(const float a[], const float b[], float t) {
        return {a[0] + (b[0] - a[0]) * t, a[1] + (b[1] - a[1]) * t, a[2] + (b[2] - a[2]) * t};
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

    class PlanePtNv {
    public:
        Point3 pointOnPlane;
        Vector3 normal;

        static PlanePtNv fromTriangle(const float p0[], const float p1[], const float p2[]) {
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

    inline Intersection intersectRayAndPlane(const Ray &ray, const PlanePtNv &plane) {
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
}

#endif