#include "mesh_utility/meshIntersection.h"
#include <algorithm>
#include <utility>

using namespace mesh;

TrianglePlaneIntersection mesh::intersectTriangleAndPlane(
        const float p0[],
        const float p1[],
        const float p2[],
        const PlanePtNv &plane) {
    auto intersect0 = intersectRayAndPlane(Ray::fromPoints(p0, p1), plane);
    auto intersect1 = intersectRayAndPlane(Ray::fromPoints(p1, p2), plane);
    auto intersect2 = intersectRayAndPlane(Ray::fromPoints(p2, p0), plane);
    bool intersect0inBounds = intersect0.valid && intersect0.distance >= -UNIT_FLOAT_EPSILON && intersect0.distance <= 1 + UNIT_FLOAT_EPSILON;
    bool intersect1inBounds = intersect1.valid && intersect1.distance >= -UNIT_FLOAT_EPSILON && intersect1.distance <= 1 + UNIT_FLOAT_EPSILON;
    bool intersect2inBounds = intersect2.valid && intersect2.distance >= -UNIT_FLOAT_EPSILON && intersect2.distance <= 1 + UNIT_FLOAT_EPSILON;
    if (intersect0inBounds && intersect1inBounds && intersect2inBounds) {
        if (intersect0.distance - intersect2.distance > intersect1.distance - intersect0.distance) {
            if (intersect0.distance - intersect2.distance > intersect2.distance - intersect1.distance) {
                return TrianglePlaneIntersection{ 0, intersect2, intersect0 };
            } else {
                return TrianglePlaneIntersection{ 2, intersect1, intersect2 };
            }
            return TrianglePlaneIntersection{ 0, intersect2, intersect0 };
        } else if (intersect2.distance - intersect1.distance > intersect1.distance - intersect0.distance) {
            return TrianglePlaneIntersection{ 2, intersect1, intersect2 };
        } else {
            return TrianglePlaneIntersection{ 1, intersect0, intersect1 };
        }
    } if (intersect0inBounds && intersect1inBounds) {
        return TrianglePlaneIntersection{ 1, intersect0, intersect1 };
    } else if (intersect0inBounds && intersect2inBounds) {
        return TrianglePlaneIntersection{ 0, intersect2, intersect0 };
    } else if (intersect1inBounds && intersect2inBounds) {
        return TrianglePlaneIntersection{ 2, intersect1, intersect2 };
    }
    return TrianglePlaneIntersection::Invalid();
}

TriangleTriangleIntersection mesh::intersectTriangles(
        const float t0p0[],
        const float t0p1[],
        const float t0p2[],
        const float t1p0[],
        const float t1p1[],
        const float t1p2[]) {
    auto planeT1 = PlanePtNv::fromTriangle(t1p0, t1p1, t1p2);
    auto intersectionT0 = intersectTriangleAndPlane(t0p0, t0p1, t0p2, planeT1);
    if (intersectionT0.peakIndex == 0xffffffff)
        return TriangleTriangleIntersection::Invalid();
    auto planeT0 = PlanePtNv::fromTriangle(t0p0, t0p1, t0p2);
    auto intersectionT1 = intersectTriangleAndPlane(t1p0, t1p1, t1p2, planeT0);
    if (intersectionT1.peakIndex == 0xffffffff)
        return TriangleTriangleIntersection::Invalid();
    return { true, intersectionT0, intersectionT1 };
}

uint32_t UniqueIndices::getIndex(float x, float y, float z) {
    auto volumeMapIt = vertexToIndexMap.find(x);
    if (volumeMapIt == vertexToIndexMap.end()) {
        auto [newVolumeItemIt, _added] = vertexToIndexMap.emplace(x, PlaneMap());
        volumeMapIt = newVolumeItemIt;
    }
    auto planeMapIt = volumeMapIt->second.find(y);
    if (planeMapIt == volumeMapIt->second.end()) {
        auto [newPlaneMapIt, _added] = volumeMapIt->second.emplace(y, DistanceMap());
        planeMapIt = newPlaneMapIt;
    }
    auto indexMapIt = planeMapIt->second.find(z);
    if (indexMapIt == planeMapIt->second.end()) {
        auto [newIndexMapIt, _added] = planeMapIt->second.emplace(z, nextIndex++);
        indexMapIt = newIndexMapIt;
    }
    return indexMapIt->second;
}

uint32_t UniqueIndices::getVertexIndex(const float v[], float epsilon) {
    return getIndex(std::round(v[0] / epsilon) * epsilon, std::round(v[1] / epsilon) * epsilon, std::round(v[2] / epsilon) * epsilon);
}

UniqueIndices::IndexMap UniqueIndices::createUniqueIndices(
        const float vertices[],
        uint32_t noOfVertices,
        float epsilon) {
    IndexMap indexMap;
    for (uint32_t index = 0; index < noOfVertices; ++index)
        indexMap.push_back(getVertexIndex(vertices + index*3, epsilon));
    return indexMap;
}

MeshIntersection::MeshIntersection(const MeshData &mesh0, const MeshData &mesh1)
        : mesh0(mesh0)
        , mesh1(mesh1) {
}

void MeshIntersection::appendMesh(
        ResultMeshData &target,
        const ResultMeshData &source,
        bool outSide,
        bool invertNormals) {

    std::unordered_map<uint32_t, uint32_t> indexMap;
    for (auto index : outSide ? source.indicesOut : source.indicesIn) {
        auto it = indexMap.find(index);
        if (it != indexMap.end()) {
            target.indicesOut.push_back(it->second);
            continue;
        }
        uint32_t newIndex = target.vertices.size() / 3;
        indexMap[index] = newIndex;
        target.indicesOut.push_back(newIndex);
        target.vertices.insert(target.vertices.end(), source.vertices.begin() + (long)index*3, source.vertices.begin() + (long)index*3+3);
        if (!source.normals.empty()) {
            if (invertNormals)
                target.normals.insert(target.normals.end(), {-source.normals[index*3], -source.normals[index*3+1], -source.normals[index*3+2]});
            else
                target.normals.insert(target.normals.end(), source.normals.begin() + (long)index*3, source.normals.begin() + (long)index*3+3);
        }
        if (!source.uvs.empty()) {
            target.uvs.insert(target.uvs.end(), source.uvs.begin() + (long)index*2, source.uvs.begin() + (long)index*2+2);
        }
    }
}

bool MeshIntersection::intersect(
        float epsilon) {
    if (!prepareIntersectionOfTriangles(epsilon))
        return false;
    intersectTrianglesOfMeshes(epsilon);
    return true;
}

bool MeshIntersection::operate(
        Operator meshOperator,
        float epsilon) {
    if (!prepareIntersectionOfTriangles(epsilon))
        return false;

    intersectTrianglesOfMeshes(epsilon);
    ResultMeshData resultIntersectAB;
    std::swap(result0, resultIntersectAB);

    std::swap(mesh0, mesh1);
    std::swap(uniqueIndexMapOfMesh0, uniqueIndexMapOfMesh1);
    std::swap(trianglesOfMesh0, trianglesOfMesh1);
    std::swap(noOfOriginalTrianglesMesh0, noOfOriginalTrianglesMesh1);
    std::swap(intersectingTrianglesOfMesh0, intersectingTrianglesOfMesh1);

    intersectTrianglesOfMeshes(epsilon);
    ResultMeshData resultIntersectBA;
    std::swap(result0, resultIntersectBA);

    appendMesh(result0, resultIntersectAB, meshOperator == Operator::MINUS || meshOperator == Operator::OR, false);
    appendMesh(result0, resultIntersectBA, meshOperator == Operator::OR, meshOperator == Operator::MINUS);
    if (meshOperator == Operator::AND) {
        for (uint32_t i = 0; i < result0.indicesOut.size(); i += 3) {
            std::swap(result0.indicesOut[i+1], result0.indicesOut[i+2]);
        }
    }
    return true;
}

bool MeshIntersection::prepareIntersectionOfTriangles(
        float epsilon) {
    if (!calculateIntersectionBox())
        return false;

    uniqueIndexMapOfMesh0 = uniqueIndices.createUniqueIndices(mesh0.vertices, mesh0.noOfVertices, epsilon);
    uniqueIndexMapOfMesh1 = uniqueIndices.createUniqueIndices(mesh1.vertices, mesh1.noOfVertices, epsilon);

    createTriangles(mesh0, uniqueIndexMapOfMesh0, trianglesOfMesh0);
    createTriangles(mesh1, uniqueIndexMapOfMesh1, trianglesOfMesh1);
    noOfOriginalTrianglesMesh0 = trianglesOfMesh0.size();
    noOfOriginalTrianglesMesh1 = trianglesOfMesh1.size();
    for (auto &triangle0 : trianglesOfMesh0) {
        if (triangle0.state == Triangle::State::NOT_INTERSECTING)
            continue;
        for (auto &triangle1 : trianglesOfMesh1) {
            if (triangle1.state == Triangle::State::NOT_INTERSECTING)
                continue;
            if (triangle0.isIntersectingAABB(triangle1)) {
                triangle0.state = Triangle::State::INTERSECTING;
                triangle1.state = Triangle::State::INTERSECTING;
            }
        }
    }
    setTriangleStates(trianglesOfMesh0, intersectingTrianglesOfMesh0);
    setTriangleStates(trianglesOfMesh1, intersectingTrianglesOfMesh1);
    return true;
}

void MeshIntersection::intersectTrianglesOfMeshes(
        float epsilon) {
    result0.vertices.insert(result0.vertices.begin(), mesh0.vertices, mesh0.vertices + mesh0.noOfVertices * 3);
    if (mesh0.normals)
        result0.normals.insert(result0.normals.begin(), mesh0.normals, mesh0.normals + mesh0.noOfVertices * 3);
    if (mesh0.uvs)
        result0.uvs.insert(result0.uvs.begin(), mesh0.uvs, mesh0.uvs + mesh0.noOfVertices * 2);

    for (uint32_t iiT0 = 0; iiT0 < intersectingTrianglesOfMesh0.size(); ++iiT0) {
        const auto triangle0 = trianglesOfMesh0[intersectingTrianglesOfMesh0[iiT0]];
        auto winding = triangle0.winding;
        if (trianglesOfMesh0.size() > (mesh0.noOfIndices + mesh1.noOfIndices) * 4) {
            result0.error = true;
            break;
        }

        bool split = false;
        for (uint32_t iiT1 = 0; iiT1 < intersectingTrianglesOfMesh1.size() && intersectingTrianglesOfMesh1[iiT1] < noOfOriginalTrianglesMesh1; ++iiT1) {
            const auto &triangle1 = trianglesOfMesh1[intersectingTrianglesOfMesh1[iiT1]];
            if (!triangle0.isIntersectingAABB(triangle1))
                continue;

            auto intersectionDirection = normalize(cross3(triangle1.faceNormal, triangle0.faceNormal));
            auto uniqueIndicesTriangle0 = triangle0.uniqueIndices;
            auto uniqueIndicesTriangle1 = triangle1.uniqueIndices;
            auto [sharedPoints0, sharedPoints2] = getSharedCorners(uniqueIndicesTriangle0, uniqueIndicesTriangle1);

            if (sharedPoints0.size() >= 2) {
                auto sharedPointDirection = sub3(triangle0.getVertex(result0.vertices.data(), sharedPoints0[1]), triangle0.getVertex(result0.vertices.data(), sharedPoints0[0]));
                if (sharedPoints0[0] == 0 && sharedPoints0[1] == 2)
                    sharedPointDirection = multiply3scalar(sharedPointDirection, -1);
                winding = dot3(intersectionDirection, sharedPointDirection) < 0 ? -1 : 1;
                continue;
            }

            if (sharedPoints0.size() == 1) {
                auto ray = triangle0.createRay(result0.vertices.data(), (sharedPoints0[0]+1) % 3, (sharedPoints0[0]+2) % 3);
                auto triangleIntersect = intersectRayAndTriangle(ray, triangle1.getVertex(mesh1.vertices, 0), triangle1.getVertex(mesh1.vertices, 1), triangle1.getVertex(mesh1.vertices, 2), 1);
                if (triangleIntersect.valid && triangleIntersect.distance > -UNIT_FLOAT_EPSILON && triangleIntersect.distance < 1 + UNIT_FLOAT_EPSILON) {
                    auto uniqueIntersectIndex = uniqueIndices.getVertexIndex(triangleIntersect.point, epsilon);
                    if (uniqueIntersectIndex != uniqueIndicesTriangle0[0] && uniqueIntersectIndex != uniqueIndicesTriangle0[1] && uniqueIntersectIndex != uniqueIndicesTriangle0[2]) {
                        auto intersectIndex = addVertexToResult(triangleIntersect.point, uniqueIntersectIndex, triangle0.indices.data());
                        addTriangleToResultTestArea(intersectionDirection, {triangle0.indices[(sharedPoints0[0]+1) % 3], intersectIndex, triangle0.indices[sharedPoints0[0]]});
                        addTriangleToResultTestArea(intersectionDirection, {triangle0.indices[(sharedPoints0[0]+2) % 3], triangle0.indices[sharedPoints0[0]], intersectIndex});
                        split = true;
                        break;
                    }
                    auto sharedPointDirection = triangleIntersect.distance < 0.5
                            ? sub3(triangleIntersect.point, triangle0.getVertex(result0.vertices.data(), sharedPoints0[0]))
                            : sub3(triangle0.getVertex(result0.vertices.data(), sharedPoints0[0]), triangleIntersect.point);
                    winding = dot3(intersectionDirection, sharedPointDirection) < 0 ? -1 : 1;
                }
                ray = triangle1.createRay(mesh1.vertices, (sharedPoints2[0]+1) % 3, (sharedPoints2[0]+2) % 3);
                triangleIntersect = intersectRayAndTriangle(ray, triangle0.getVertex(result0.vertices.data(), 0), triangle0.getVertex(result0.vertices.data(), 1), triangle0.getVertex(result0.vertices.data(), 2), 1);
                if (triangleIntersect.valid && triangleIntersect.distance > 0 && triangleIntersect.distance < 1) {
                    auto uniqueIntersectIndex = uniqueIndices.getVertexIndex(triangleIntersect.point, epsilon);
                    if (uniqueIntersectIndex != uniqueIndicesTriangle0[0] && uniqueIntersectIndex != uniqueIndicesTriangle0[1] && uniqueIntersectIndex != uniqueIndicesTriangle0[2]) {
                        auto intersectIndex = addVertexToResult(triangleIntersect.point, uniqueIntersectIndex, triangle0.indices.data());
                        addTriangleToResultTestArea(intersectionDirection, {triangle0.indices[(sharedPoints0[0]+1) % 3], intersectIndex, triangle0.indices[sharedPoints0[0]]});
                        addTriangleToResultTestArea(intersectionDirection, {triangle0.indices[(sharedPoints0[0]+2) % 3], triangle0.indices[sharedPoints0[0]], intersectIndex});
                        addTriangleToResultWithWindingTestArea(triangle0.winding, {triangle0.indices[(sharedPoints0[0]+1) % 3], triangle0.indices[(sharedPoints0[0]+2) % 3], intersectIndex});
                        split = true;
                        break;
                    }
                }
                continue;
            }

            auto intersect = intersectTriangles(triangle0.getVertex(result0.vertices.data(), 0), triangle0.getVertex(result0.vertices.data(), 1), triangle0.getVertex(result0.vertices.data(), 2),
                                                triangle1.getVertex(mesh1.vertices, 0), triangle1.getVertex(mesh1.vertices, 1), triangle1.getVertex(mesh1.vertices, 2));
            if (!intersect.valid)
                continue;
            std::vector<TriangleIntersectionPoint> points{
                    { 0, intersect.intersectionTriangle0.intersectToPeak, 0.0f },
                    { 0, intersect.intersectionTriangle0.intersectFromPeak, 0.0f },
                    { 1, intersect.intersectionTriangle1.intersectToPeak, 0.0f },
                    { 1, intersect.intersectionTriangle1.intersectFromPeak, 0.0f },
            };
            auto directionVector = sub3(points[1].intersection.point, points[0].intersection.point);
            if (length(directionVector) < epsilon)
                continue;
            auto referencePoint = points.front().intersection.point;
            std::for_each(points.begin(), points.end(), [&referencePoint, &directionVector](TriangleIntersectionPoint &p) {
                p.distance = dot3(sub3(p.intersection.point, referencePoint), directionVector);
            });
            std::sort(points.begin(), points.end(), [](TriangleIntersectionPoint &a, TriangleIntersectionPoint &b) -> bool {
                return a.distance < b.distance;
            });
            if (points[0].triangleIndex == points[1].triangleIndex ||
                (points[0].triangleIndex == points[2].triangleIndex && points[1].distance == points[2].distance)) {
                continue;
            }

            auto uniqueIndex1 = uniqueIndices.getVertexIndex(points[1].intersection.point, epsilon);
            auto uniqueIndex2 = uniqueIndices.getVertexIndex(points[2].intersection.point, epsilon);
            if (std::find(uniqueIndicesTriangle0.begin(), uniqueIndicesTriangle0.end(), uniqueIndex1) != uniqueIndicesTriangle0.end() &&
                std::find(uniqueIndicesTriangle0.begin(), uniqueIndicesTriangle0.end(), uniqueIndex2) != uniqueIndicesTriangle0.end()) {
                if (uniqueIndex1 != uniqueIndex2) {
                    auto d = dot3(intersectionDirection, sub3(triangle0.getVertex(result0.vertices.data(), (intersect.intersectionTriangle0.peakIndex + 2) % 3), triangle0.getVertex(result0.vertices.data(), (intersect.intersectionTriangle0.peakIndex + 1) % 3)));
                    winding = d < 0 ? -1 : 1;
                }
                continue;
            }

            auto intersectIndex1 = addVertexToResult(points[1].intersection.point, uniqueIndex1, triangle0.indices.data());
            auto intersectIndex2 = addVertexToResult(points[2].intersection.point, uniqueIndex2, triangle0.indices.data());
            std::array<uint32_t, 3> orderedIndices{
                    triangle0.indices[intersect.intersectionTriangle0.peakIndex],
                    triangle0.indices[(intersect.intersectionTriangle0.peakIndex + 1) % 3],
                    triangle0.indices[(intersect.intersectionTriangle0.peakIndex + 2) % 3],
            };
            if (points[0].triangleIndex == 0 && (points[1].distance - points[0].distance) > epsilon)
                addTriangleToResultWithWindingTestArea(triangle0.winding, {orderedIndices[2], orderedIndices[0], intersectIndex1});
            addTriangleToResultTestArea(intersectionDirection, {orderedIndices[0], intersectIndex2, intersectIndex1});
            addTriangleToResultTestArea(intersectionDirection, {orderedIndices[2], intersectIndex1, intersectIndex2});
            addTriangleToResultWithWindingTestArea(triangle0.winding, {orderedIndices[1], orderedIndices[2], intersectIndex2});
            if (points[3].triangleIndex == 0 && (points[3].distance - points[2].distance) > epsilon)
                addTriangleToResultWithWindingTestArea(triangle0.winding, {orderedIndices[0], orderedIndices[1], intersectIndex2});
            split = true;
            break;
        }
        if (!split) {
            auto &triangle = trianglesOfMesh0[intersectingTrianglesOfMesh0[iiT0]];
            triangle.winding = winding;
            triangle.state = Triangle::State::FINAL;
        }
    }

    updateWindingOrderOfResultingTriangles();
}

void MeshIntersection::updateWindingOrderOfResultingTriangles() {
    struct TI {
        std::array<uint32_t, 3> indices;
        std::array<uint32_t, 3> uniqueIndices;
    };
    std::vector<TI> ti;
    ti.reserve(trianglesOfMesh0.size());
    for (auto &triangle: trianglesOfMesh0) {
        if (triangle.winding == 1 && triangle.isFinalTriangle())
            ti.push_back({triangle.indices, triangle.uniqueIndices});
    }
    uint32_t outerEnd = ti.size();
    for (auto &triangle: trianglesOfMesh0) {
        if (triangle.winding == 0 && triangle.isFinalTriangle())
            ti.push_back({triangle.indices, triangle.uniqueIndices});
    }
    uint32_t unknownEnd = ti.size();
    uint32_t outerTestStart = 0;
    uint32_t outerTestEnd = outerEnd;
    while (outerTestStart < outerTestEnd) {
        for (uint32_t i = outerTestEnd; i < unknownEnd; ++i) {
            auto uniqueIndicesI = ti[i].uniqueIndices;
            for (uint32_t j = outerTestStart; j < outerTestEnd; ++j) {
                auto uniqueIndicesJ = ti[j].uniqueIndices;
                auto count = std::count_if(uniqueIndicesI.begin(), uniqueIndicesI.end(), [&](uint32_t ui) -> bool{
                    return std::find(uniqueIndicesJ.begin(), uniqueIndicesJ.end(), ui) != uniqueIndicesJ.end();
                });
                if (count == 2) {
                    std::swap(ti[outerEnd], ti[i]);
                    outerEnd ++;
                    break;
                }
            }
        }
        outerTestStart = outerTestEnd;
        outerTestEnd = outerEnd;
    }

    for (uint32_t i = 0; i < ti.size(); ++ i) {
        if (i < outerEnd)
            result0.indicesOut.insert(result0.indicesOut.end(), ti[i].indices.begin(), ti[i].indices.end());
        else
            result0.indicesIn.insert(result0.indicesIn.end(), {ti[i].indices[0], ti[i].indices[2], ti[i].indices[1]});
    }
    for (auto &triangle: trianglesOfMesh0) {
        if (triangle.winding == -1 && triangle.isFinalTriangle())
            result0.indicesIn.insert(result0.indicesIn.end(), {triangle.indices[0], triangle.indices[2], triangle.indices[1]});
    }
}

bool MeshIntersection::calculateIntersectionBox() {
    intersectionBoxValid =
            mesh0.aabbMin[0] <= mesh1.aabbMax[0] && mesh1.aabbMin[0] <= mesh0.aabbMax[0] &&
            mesh0.aabbMin[1] <= mesh1.aabbMax[1] && mesh1.aabbMin[1] <= mesh0.aabbMax[1] &&
            mesh0.aabbMin[2] <= mesh1.aabbMax[2] && mesh1.aabbMin[2] <= mesh0.aabbMax[2];
    if (intersectionBoxValid) {
        intersectionAABBMin = Point3{std::min(mesh0.aabbMin[0], mesh1.aabbMin[0]),
                                     std::min(mesh0.aabbMin[1], mesh1.aabbMin[1]),
                                     std::min(mesh0.aabbMin[2], mesh1.aabbMin[2])};
        intersectionAABBMax = Point3{std::max(mesh0.aabbMax[0], mesh1.aabbMax[0]),
                                     std::max(mesh0.aabbMax[1], mesh1.aabbMax[1]),
                                     std::max(mesh0.aabbMax[2], mesh1.aabbMax[2])};
    }
    return intersectionBoxValid;
}

void MeshIntersection::createTriangles(
        const MeshData &mesh,
        UniqueIndices::IndexMap &indexMap,
        TriangleContainer &trianglesOfMesh) {
    for (uint32_t i = 0; i < mesh.noOfIndices; i += 3) {
        Triangle triangle(mesh.vertices, mesh.indices + i, indexMap);
        if (triangle.hasNoArea())
            continue;
        if (!triangle.isIntersectingAABB(intersectionAABBMin, intersectionAABBMax))
            triangle.state = Triangle::State::NOT_INTERSECTING;
        trianglesOfMesh.emplace_back(triangle);
    }
}

void MeshIntersection::setTriangleStates(
        TriangleContainer &trianglesOfMesh,
        std::vector<uint32_t> &intersectingTrianglesOfMesh) {
    for (uint32_t i = 0; i < trianglesOfMesh.size(); ++i) {
        if (trianglesOfMesh[i].state == Triangle::State::UNKNOWN)
            trianglesOfMesh[i].state = Triangle::State::NOT_INTERSECTING;
        if (trianglesOfMesh[i].state != Triangle::State::NOT_INTERSECTING)
            intersectingTrianglesOfMesh.emplace_back(i);
    }
}

std::tuple<std::vector<uint32_t>, std::vector<uint32_t>> MeshIntersection::getSharedCorners(
        const std::array<uint32_t, 3> &uniqueIndicesTriangle0,
        const std::array<uint32_t, 3> &uniqueIndicesTriangle1) {
    std::vector<uint32_t> sharedPoints0, sharedPoints1;
    for (uint32_t i = 0; i < 3; ++ i) {
        auto findIt = std::find(uniqueIndicesTriangle1.begin(), uniqueIndicesTriangle1.end(), uniqueIndicesTriangle0[i]);
        if (findIt != uniqueIndicesTriangle1.end()) {
            sharedPoints0.emplace_back(i);
            sharedPoints1.emplace_back(findIt - uniqueIndicesTriangle1.begin());
        }
    }
    return {sharedPoints0, sharedPoints1};
}

std::tuple<Vector3, Vector2> MeshIntersection::calculateNormalAndUVForPointInTriangle(
        const float p[],
        const uint32_t triangleIndices[],
        const ResultMeshData &mesh) {
    const auto v0 = mesh.vertices.data() + triangleIndices[0] * 3;
    const auto v1 = mesh.vertices.data() + triangleIndices[1] * 3;
    const auto v2 = mesh.vertices.data() + triangleIndices[2] * 3;
    auto barycentricCoordinates = barycentricCoordinateFromVectors(sub3(p, v0), sub3(v1, v0), sub3(v2, v0));
    Vector3 nv;
    if (!mesh.normals.empty()) {
        const auto n0 = mesh.normals.data() + triangleIndices[0] * 3;
        const auto n1 = mesh.normals.data() + triangleIndices[1] * 3;
        const auto n2 = mesh.normals.data() + triangleIndices[2] * 3;
        nv = normalize(interpolateBarycentric3(barycentricCoordinates, n0, n1, n2));
    }
    Vector2 uv;
    if (!mesh.uvs.empty()) {
        const auto uv0 = mesh.uvs.data() + triangleIndices[0] * 2;
        const auto uv1 = mesh.uvs.data() + triangleIndices[1] * 2;
        const auto uv2 = mesh.uvs.data() + triangleIndices[2] * 2;
        uv = interpolateBarycentric2(barycentricCoordinates, uv0, uv1, uv2);
    }
    return {nv, uv};
}

uint32_t MeshIntersection::addVertexToResult(
        const float p[],
        uint32_t uniqueIndex,
        const uint32_t triangleIndices[]) {
    auto [nv, uv] = calculateNormalAndUVForPointInTriangle(p, triangleIndices, result0);
    auto index = result0.vertices.size() / 3;
    uniqueIndexMapOfMesh0.push_back(uniqueIndex);
    result0.vertices.insert(result0.vertices.end(), p, p + 3);
    if (!result0.normals.empty())
        result0.normals.insert(result0.normals.end(), nv.begin(), nv.end());
    if (!result0.uvs.empty())
        result0.uvs.insert(result0.uvs.end(), uv.begin(), uv.end());
    return index;
}

void MeshIntersection::addTriangleToResultTestArea(
        const Vector3 &intersectionDirection,
        std::array<uint32_t, 3> indices) {
    std::array<uint32_t, 3> uniqueTriangleIndices;
    if (createAndEvaluateUniqueIndices(indices, uniqueIndexMapOfMesh0, uniqueTriangleIndices))
        addTriangleToResult(intersectionDirection, Triangle(result0.vertices.data(), indices.data(), uniqueIndexMapOfMesh0), result0.vertices.data());
}

void MeshIntersection::addTriangleToResultWithWindingTestArea(
        int winding,
        std::array<uint32_t, 3> indices) {
    std::array<uint32_t, 3> uniqueTriangleIndices;
    if (createAndEvaluateUniqueIndices(indices, uniqueIndexMapOfMesh0, uniqueTriangleIndices))
        addTriangleToResult(winding, Triangle(result0.vertices.data(), indices, uniqueTriangleIndices, uniqueIndexMapOfMesh0));
}

void MeshIntersection::addTriangleToResult(
        const float intersectionDirection[],
        const Triangle &triangleTouchingEdge12,
        const float *vertices) {
    intersectingTrianglesOfMesh0.emplace_back(trianglesOfMesh0.size());
    trianglesOfMesh0.push_back(triangleTouchingEdge12);
    auto d = dot3(intersectionDirection, trianglesOfMesh0.back().edge(vertices, 1));
    trianglesOfMesh0.back().winding = d < 0 ? -1 : 1;
}

void MeshIntersection::addTriangleToResult(
        int winding,
        const Triangle &triangleTouchingEdge12) {
    intersectingTrianglesOfMesh0.emplace_back(trianglesOfMesh0.size());
    trianglesOfMesh0.push_back(triangleTouchingEdge12);
    trianglesOfMesh0.back().winding = winding;
}
