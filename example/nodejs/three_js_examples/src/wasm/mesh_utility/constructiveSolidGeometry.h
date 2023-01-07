#ifndef __CONSTRUCTIVE_SOLID_GEOMETRY__H__
#define __CONSTRUCTIVE_SOLID_GEOMETRY__H__

#include "meshMath.h"
#include "meshSpecification.h"
#include <algorithm>
#include <memory>
#include <vector>

namespace csg {

    using namespace mesh;

    class Polygon;
    using PolygonPtr = std::shared_ptr<Polygon>;
    using Polygons = std::vector<PolygonPtr>;
    class Plane;
    using PlanePtr = std::shared_ptr<Plane>;
    class Node;
    using NodePtr = std::shared_ptr<Node>;

    constexpr float PLANE_EPSILON = 1.0e-5f;

    class Epsilon {
    public:
        Epsilon(float scale);
        ~Epsilon();
    };

    class Vertex {
    public:
        mesh::Vector3 vertex{0, 0, 0};
        mesh::Vector3 normal{0, 0, 0};
        mesh::Vector2 uv{0, 0};

        Vertex & flip() {
            normal[0] = -normal[0];
            normal[1] = -normal[1];
            normal[2] = -normal[2];
            return *this;
        }

        static Vertex interpolate(const Vertex &a, const Vertex &b, float t) {
            return Vertex{lerp3(a.vertex, b.vertex, t), lerp3(a.normal, b.normal, t), lerp2(a.uv, b.uv, t)};
        }
    };

    using Vertices = std::vector<Vertex>;

    class Plane {
    public:
        static float epsilon;
        mesh::Vector3 normal{0, 0, 0};
        float w{0};

        static Plane fromPoints(const float *p0, const float *p1, const float *p2) {
            auto nv = normalize(cross3(sub3(p1, p0), mesh::sub3(p2, p0)));
            return Plane{nv, dot3(nv, p0)};
        }

        static Plane * newPlaneFromPoints(const float *p0, const float *p1, const float *p2) {
            auto nv = normalize(cross3(sub3(p1, p0), mesh::sub3(p2, p0)));
            return new Plane{nv, dot3(nv, p0)};
        }

        Plane & flip() {
            normal[0] = -normal[0];
            normal[1] = -normal[1];
            normal[2] = -normal[2];
            w = -w;
            return *this;
        }

        inline void splitPolygon(const PolygonPtr &polygon, Polygons &coplanarFront, Polygons &coplanarBack, Polygons &front, Polygons &back) const;
    };

    class Polygon {
        public:
        Vertices vertices;
        Plane plane;
        static Polygon* newPolygon(const Vertices &vertices) {
            return new Polygon{vertices, Plane::fromPoints(vertices[0].vertex, vertices[1].vertex, vertices[2].vertex)};
        }
        Polygon & flip() {
            std::reverse(vertices.begin(), vertices.end());
            for (auto &v: vertices) v.flip();
            plane.flip();
            return *this;
        }
    };

    void Plane::splitPolygon(const PolygonPtr &polygon, Polygons &coplanarFront, Polygons &coplanarBack, Polygons &front, Polygons &back) const {
        const uint8_t COPLANAR = 0;
        const uint8_t FRONT = 1;
        const uint8_t BACK = 2;
        const uint8_t SPANNING = 3;

        uint8_t polygonType = 0;
        std::vector<uint8_t> types;
        for (uint32_t i = 0; i < polygon->vertices.size(); ++i) {
            auto t = dot3(normal, polygon->vertices[i].vertex) - w;
            uint8_t type = (t < -epsilon) ? BACK : (t > epsilon) ? FRONT : COPLANAR;
            polygonType |= type;
            types.push_back(type);
        }

        switch (polygonType) {
            case COPLANAR:
                dot3(normal, polygon->plane.normal) > 0 ? coplanarFront.push_back(polygon) : coplanarBack.push_back(polygon);
                break;
            case FRONT:
                front.push_back(polygon);
                break;
            case BACK:
                back.push_back(polygon);
                break;
            case SPANNING:
                Vertices f, b;
                for (uint32_t i = 0; i < polygon->vertices.size(); i++) {
                    uint32_t j = (i + 1) % polygon->vertices.size();
                    auto ti = types[i], tj = types[j];
                    auto &vi = polygon->vertices[i], &vj = polygon->vertices[j];
                    if (ti != BACK)
                        f.push_back(vi);
                    if (ti != FRONT)
                        b.push_back(vi);
                    if ((ti | tj) == SPANNING) {
                        auto t = (w - dot3(normal, vi.vertex)) / dot3(normal, sub3(vj.vertex, vi.vertex));
                        auto v = Vertex::interpolate(vi, vj, t);
                        f.push_back(v);
                        b.push_back(v);
                    }
                }
                if (f.size() >= 3)
                    front.emplace_back(Polygon::newPolygon(f));
                if (b.size() >= 3)
                    back.emplace_back(Polygon::newPolygon(b));
                break;
        }
    }

    class Node {
    public:
        std::shared_ptr<Plane> plane;
        std::shared_ptr<Node> front;
        std::shared_ptr<Node> back;
        Polygons polygons;

        static Node constructNode(const Polygons &polygons) {
            Node node;
            if (!polygons.empty())
                node.build(polygons);
            return node;
        }

        static Node* newNode(const Polygons &polygons) {
            auto node = new Node{};
            if (!polygons.empty())
                node->build(polygons);
            return node;
        }

        Node & invert() {
            for (auto &polygon: polygons)
                polygon->flip();
            if (plane)
                plane->flip();
            if (front)
                front->invert();
            if (back)
                back->invert();
            std::swap(front, back);
            return *this;
        }

        Polygons clipPolygons(const Polygons &polygonsToSplit) const {
            if (!plane)
                return polygonsToSplit;
            Polygons splitFront, splitBack;
            for (auto &polygon: polygonsToSplit)
                plane->splitPolygon(polygon, splitFront, splitBack, splitFront, splitBack);
            if (front)
                splitFront = front->clipPolygons(splitFront);
            if (back)
                splitBack = back->clipPolygons(splitBack);
            else
                splitBack.clear();
            splitFront.insert(splitFront.end(), splitBack.begin(), splitBack.end());
            return splitFront;
        }

        Node & clipTo(const Node &bsp) {
            polygons = bsp.clipPolygons(polygons);
            if (front)
                front->clipTo(bsp);
            if (back)
                back->clipTo(bsp);
            return *this;
        }

        Polygons allPolygons() const {
            auto polygonsCollection = polygons;
            if (front) {
                auto allFrontPolygons = front->allPolygons();
                polygonsCollection.insert(polygonsCollection.end(), allFrontPolygons.begin(), allFrontPolygons.end());
            }
            if (back) {
                auto allBackPolygons = back->allPolygons();
                polygonsCollection.insert(polygonsCollection.end(), allBackPolygons.begin(), allBackPolygons.end());
            }
            return polygonsCollection;
        }

        void build(const Polygons &newPolygons) {
            if (newPolygons.empty())
                return;
            if (!plane)
                plane = std::make_shared<Plane>(newPolygons[0]->plane);
            Polygons newFront, newBack;
            for (auto &polygon: newPolygons) {
                plane->splitPolygon(polygon, polygons, polygons, newFront, newBack);
            }
            if (!newFront.empty()) {
                if (!front)
                    front = NodePtr(new Node());
                front->build(newFront);
            }
            if (!newBack.empty()) {
                if (!back)
                    back = NodePtr(new Node());
                back->build(newBack);
            }
        }
    };

    class CSG {
    public:
        static Polygons operatorUnion(Polygons &polygonsA, Polygons &polygonsB) {
            auto a = Node::constructNode(polygonsA);
            auto b = Node::constructNode(polygonsB);
            a.clipTo(b);
            b.clipTo(a);
            b.invert();
            b.clipTo(a);
            b.invert();
            a.build(b.allPolygons());
            return a.allPolygons();
        }

        static Polygons operatorSubtract(Polygons &polygonsA, Polygons &polygonsB) {
            auto a = Node::constructNode(polygonsA);
            auto b = Node::constructNode(polygonsB);
            a.invert();
            a.clipTo(b);
            b.clipTo(a);
            b.invert();
            b.clipTo(a);
            b.invert();
            a.build(b.allPolygons());
            a.invert();
            return a.allPolygons();
        }

        static Polygons operatorIntersect(Polygons &polygonsA, Polygons &polygonsB) {
            auto a = Node::constructNode(polygonsA);
            auto b = Node::constructNode(polygonsB);
            a.invert();
            b.clipTo(a);
            b.invert();
            a.clipTo(b);
            b.clipTo(a);
            a.build(b.allPolygons());
            a.invert();
            return a.allPolygons();
        }

        static void inverse(Polygons &polygons) {
            for (auto &polygon: polygons)
                polygon->flip();
        }
    };

    inline Polygons *polygonsFromMeshSpecification(const MeshData &mesh) {
        Polygons *polygons = new Polygons();
        for (uint32_t triangleIndex = 0; triangleIndex < mesh.noOfIndices; triangleIndex += 3) {
            if (mesh.indices[triangleIndex] == mesh.indices[triangleIndex + 1] ||
                mesh.indices[triangleIndex] == mesh.indices[triangleIndex + 2] ||
                mesh.indices[triangleIndex + 1] == mesh.indices[triangleIndex + 2]) {
                continue;
            }
            Vertices vertices;
            for (uint32_t i = triangleIndex; i < triangleIndex + 3; ++i) {
                auto index = mesh.indices[i];
                Point3 v{mesh.vertices[index*3], mesh.vertices[index*3 + 1], mesh.vertices[index*3 + 2]};
                Vector3 nv{0, 0, 0};
                if (mesh.normals)
                    nv = {mesh.normals[index*3], mesh.normals[index*3 + 1], mesh.normals[index*3 + 2]};
                Vector2 uv{0, 0};
                if (mesh.uvs)
                    uv = {mesh.uvs[index*2], mesh.uvs[index*2 + 1]};
                vertices.push_back({v, nv, uv});
            }
            auto lengltSq = squareLength(cross3(sub3(vertices[1].vertex, vertices[0].vertex), mesh::sub3(vertices[2].vertex, vertices[0].vertex)));
            if (lengltSq > PLANE_EPSILON)
                polygons->push_back(PolygonPtr(Polygon::newPolygon(vertices)));
        }
        return polygons;
    }

    inline void polygonsToMesh(const Polygons *polygons, ResultMeshData &mesh) {
        if (!polygons)
            return;
        for (auto &polygon: *polygons) {
            uint32_t faceStartIndex = mesh.vertices.size() / 3;
            for (auto & vertex: polygon->vertices) {
                mesh.vertices.insert(mesh.vertices.end(), vertex.vertex.begin(), vertex.vertex.end());
                mesh.normals.insert(mesh.normals.end(), vertex.normal.begin(), vertex.normal.end());
                mesh.uvs.insert(mesh.uvs.end(), vertex.uv.begin(), vertex.uv.end());
            }
            uint32_t faceEndIndex = mesh.vertices.size() / 3;
            if (faceEndIndex > faceStartIndex + 2) {
                for (uint32_t i = 0; i < faceEndIndex - faceStartIndex - 2; ++i) {
                    mesh.indicesOut.push_back(faceStartIndex);
                    mesh.indicesOut.push_back(faceStartIndex + i + 1);
                    mesh.indicesOut.push_back(faceStartIndex + i + 2);
                }
            }
        }
    }

    inline Polygons meshOperation(Operator op, const MeshData &meshA, const MeshData &meshB) {
        auto polygonsA = polygonsFromMeshSpecification(meshA);
        auto polygonsB = polygonsFromMeshSpecification(meshB);
        Polygons resultPolygons;
        switch(op) {
            case Operator::AND: resultPolygons = CSG::operatorUnion(*polygonsA, *polygonsB); break;
            case Operator::OR: resultPolygons = CSG::operatorIntersect(*polygonsA, *polygonsB); break;
            case Operator::MINUS: resultPolygons = CSG::operatorSubtract(*polygonsA, *polygonsB); break;
        }
        delete polygonsA;
        delete polygonsB;
        return resultPolygons;
    }

    inline void meshOperation(Operator op, const MeshData &meshA, const MeshData &meshB, ResultMeshData &resultMesh) {
        Polygons resultPolygons = meshOperation(op, meshA, meshB);
        polygonsToMesh(&resultPolygons, resultMesh);
    }
}

#endif