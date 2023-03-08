#include "mesh_utility/meshMath.h"

using namespace mesh;

Point3 mesh::barycentricCoordinateFromVectors(
        const float ax[],
        const float ab[],
        const float ac[]) {
    const auto d00 = dot3(ab, ab);
    const auto d01 = dot3(ab, ac);
    const auto d11 = dot3(ac, ac);
    const auto d20 = dot3(ax, ab);
    const auto d21 = dot3(ax, ac);
    const auto determinant = d00 * d11 - d01 * d01;
    const auto by = (d11 * d20 - d01 * d21) / determinant;
    const auto bz = (d00 * d21 - d01 * d20) / determinant;
    const auto bx = 1 - by - bz;
    return { bx, by, bz };
}

// https://www.scratchapixel.com/lessons/3d-basic-rendering/minimal-ray-tracer-rendering-simple-shapes/ray-box-intersection
// https://gamedev.stackexchange.com/questions/18436/most-efficient-aabb-vs-ray-collision-algorithms
bool mesh::areEndlessRayAndBoxIntersecting(
        const Ray &ray,
        const float aabbMin[],
        const float aabbMax[]) {
    auto tMin = (aabbMin[0] - ray.origin[0]) / ray.direction[0];
    auto tMax = (aabbMax[0] - ray.origin[0]) / ray.direction[0];
    if (tMin > tMax) std::swap(tMin, tMax);

    auto tyMin = (aabbMin[1] - ray.origin[1]) / ray.direction[1];
    auto tyMax = (aabbMax[1] - ray.origin[1]) / ray.direction[1];
    if (tyMin > tyMax) std::swap(tyMin, tyMax);
    if ((tMin > tyMax) || (tyMin > tMax))
        return false;
    if (tyMin > tMin) tMin = tyMin;
    if (tyMax < tMax) tMax = tyMax;

    auto tzMin = (aabbMin[2] - ray.origin[2]) / ray.direction[2];
    auto tzMax = (aabbMax[2] - ray.origin[2]) / ray.direction[2];
    if (tzMin > tzMax) std::swap(tzMin, tzMax);
    if ((tMin > tzMax) || (tzMin > tMax))
        return false;

    //if (tzMin > tMin) tMin = tzMin;
    //if (tzMax < tMax) tMax = tzMax;
    return true;
}

// https://stackoverflow.com/questions/59257678/intersect-a-ray-with-a-triangle-in-glsl-c/59261241#59261241
Intersection mesh::intersectRayAndTriangle(
        const Ray &ray,
        const float p0[],
        const float p1[],
        const float p2[],
        float maximumDistance) {
    auto intersection = intersectRayAndPlane(ray, PlanePtNv::fromTriangle(p0, p1, p2));
    intersection.valid =
            intersection.valid &&
            intersection.distance >= 0 &&
            intersection.distance <= maximumDistance &&
            isPointInOrOnTriangle(intersection.point, p0, p1, p2);
    return intersection;
}
