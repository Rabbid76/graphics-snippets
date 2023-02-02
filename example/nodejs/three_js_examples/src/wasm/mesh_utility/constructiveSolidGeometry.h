#ifndef __CONSTRUCTIVE_SOLID_GEOMETRY__H__
#define __CONSTRUCTIVE_SOLID_GEOMETRY__H__

#include "meshSpecification.h"
#include <algorithm>
#include <cmath>
#include <memory>
#include <deque>
#include <vector>

namespace csg {

    using Scalar = double;

    class Vector2 : public std::array<Scalar, 2> {
    public:
        operator const Scalar *() const {
            return data();
        }
    };

    class Vector3 : public std::array<Scalar, 3> {
    public:
        operator const Scalar *() const {
            return data();
        }
    };

    inline Vector3 add3(const Scalar a[], const Scalar b[]) {
        return {a[0] + b[0], a[1] + b[1], a[2] + b[2]};
    }

    inline Vector3 sub3(const Scalar a[], const Scalar b[]) {
        return {a[0] - b[0], a[1] - b[1], a[2] - b[2]};
    }

    inline Scalar dot3(const Scalar a[], const Scalar b[]) {
        return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
    }

    inline Vector3 cross3(const Scalar a[], const Scalar b[]) {
        return {a[1] * b[2] - a[2] * b[1], a[2] * b[0] - a[0] * b[2], a[0] * b[1] - a[1] * b[0]};
    }

    inline Scalar squareLength(const Scalar v[]) {
        return v[0] * v[0] + v[1] * v[1] + v[2] * v[2];
    }

    inline Scalar length(const Scalar v[]) {
        return std::sqrt(squareLength(v));
    }

    inline Vector3 normalize(const Scalar v[]) {
        auto euclideanLength = length(v);
        return {v[0] / euclideanLength, v[1] / euclideanLength, v[2] / euclideanLength};
    }

    inline Vector2 lerp2(const Scalar a[], const Scalar b[], Scalar t) {
        return {a[0] + (b[0] - a[0]) * t, a[1] + (b[1] - a[1]) * t};
    }

    inline Vector3 lerp3(const Scalar a[], const Scalar b[], Scalar t) {
        return {a[0] + (b[0] - a[0]) * t, a[1] + (b[1] - a[1]) * t, a[2] + (b[2] - a[2]) * t};
    }

    constexpr Scalar PLANE_EPSILON = 1.0e-5;

    class Epsilon {
    public:
        explicit Epsilon(Scalar scale);
        ~Epsilon();
    };

    class CSG;
    class Polygon;
    using PolygonIndex = uint32_t;
    using PolygonIndices = std::vector<PolygonIndex>;

    struct VertexIndex {
        uint32_t index;
        bool inverted;
    };
    using VertexIndices = std::vector<VertexIndex>;

    class Vertex {
    public:
        Vector3 vertex{0, 0, 0};
        Vector3 normal{0, 0, 0};
        Vector2 uv{0, 0};

        Vertex & flip() {
            normal[0] = -normal[0];
            normal[1] = -normal[1];
            normal[2] = -normal[2];
            return *this;
        }

        static Vertex interpolate(const Vertex &a, const Vertex &b, Scalar t) {
            return Vertex{lerp3(a.vertex, b.vertex, t), lerp3(a.normal, b.normal, t), lerp2(a.uv, b.uv, t)};
        }
    };

    class Plane {
    public:
        static Scalar epsilon;
        Vector3 normal{0, 0, 0};
        Scalar w{0};

        static Plane fromPoints(const Scalar *p0, const Scalar *p1, const Scalar *p2) {
            auto nv = normalize(cross3(sub3(p1, p0), sub3(p2, p0)));
            return Plane{nv, dot3(nv, p0)};
        }

        Plane & flip() {
            normal[0] = -normal[0];
            normal[1] = -normal[1];
            normal[2] = -normal[2];
            w = -w;
            return *this;
        }

        inline void splitPolygon(CSG &csg, PolygonIndex polygonIndex, PolygonIndices &coplanarFront, PolygonIndices &coplanarBack, PolygonIndices &front, PolygonIndices &back) const;
    };

    class Polygon {
    public:
        VertexIndices vertices;
        Plane plane;

        Polygon & flip() {
            std::reverse(vertices.begin(), vertices.end());
            for (auto &v: vertices) v.inverted = !v.inverted;
            plane.flip();
            return *this;
        }
    };

    class Node {
    public:
        CSG &csg;
        bool planeSet = false;
        Plane plane;
        Node *front = nullptr;
        Node *back = nullptr;
        PolygonIndices polygons;

        static Node constructNode(CSG &csg, const PolygonIndices &nodePolygons) {
            Node node(csg);
            if (!nodePolygons.empty())
                node.build(nodePolygons);
            return node;
        }

        explicit Node(CSG &csg) : csg(csg) {}
        Node(const Node&) = delete;
        Node(Node&&) = default;
        Node& operator = (const Node&) = delete;
        Node& operator = (Node&&) = delete;
        ~Node() {
            delete front;
            delete back;
        }

        [[nodiscard]] PolygonIndices allPolygons() {
            PolygonIndices polygonsCollection;
            allPolygons(polygonsCollection);
            return polygonsCollection;
        }

        inline Node & invert();
        [[nodiscard]] inline PolygonIndices clipPolygons(const PolygonIndices &polygonsToSplit) const;
        inline Node & clipTo(const Node &bsp);
        inline void allPolygons(PolygonIndices &polygonsCollection) const;
        inline void build(const PolygonIndices &newPolygons);
        static inline void build(CSG &csg, Node *startNode, const PolygonIndices &newPolygons);
    };

    class CSG {
    public:
        std::vector<Vertex> vertices;
        std::vector<Polygon> polygons;

        const Scalar* getVertex(uint32_t i) const {
            return (vertices.data() + i)->vertex.data();
        }

        inline uint32_t newVertex(const Vertex &vertex);
        inline PolygonIndex newPolygon(const VertexIndices &vertices);
        PolygonIndices operatorUnion(PolygonIndices &polygonsA, PolygonIndices &polygonsB);
        PolygonIndices operatorSubtract(PolygonIndices &polygonsA, PolygonIndices &polygonsB);
        PolygonIndices operatorIntersect(PolygonIndices &polygonsA, PolygonIndices &polygonsB);
    };

    void Plane::splitPolygon(CSG &csg, PolygonIndex polygonIndex, PolygonIndices &coplanarFront, PolygonIndices &coplanarBack, PolygonIndices &front, PolygonIndices &back) const {
        constexpr uint8_t COPLANAR = 0;
        constexpr uint8_t FRONT = 1;
        constexpr uint8_t BACK = 2;
        constexpr uint8_t SPANNING = 3;

        auto &polygon = csg.polygons[polygonIndex];
        uint8_t polygonType = 0;
        uint32_t noOfVertices = polygon.vertices.size();
        uint8_t *types = new uint8_t[noOfVertices];
        for (uint32_t i = 0; i < noOfVertices; ++i) {
            auto t = dot3(normal, csg.getVertex(polygon.vertices[i].index)) - w;
            types[i] = (t < -epsilon) ? BACK : (t > epsilon) ? FRONT : COPLANAR;
            polygonType |= types[i];
        }

        switch (polygonType) {
            default:
            case COPLANAR: dot3(normal, polygon.plane.normal) > 0 ? coplanarFront.push_back(polygonIndex) : coplanarBack.push_back(polygonIndex); break;
            case FRONT: front.push_back(polygonIndex); break;
            case BACK: back.push_back(polygonIndex); break;
            case SPANNING:
                VertexIndices f, b;
                for (uint32_t i = 0; i <noOfVertices; i++) {
                    uint32_t j = (i + 1) % polygon.vertices.size();
                    auto ti = types[i], tj = types[j];
                    auto vi = polygon.vertices[i], vj = polygon.vertices[j];
                    if (ti != BACK)
                        f.push_back(vi);
                    if (ti != FRONT)
                        b.push_back(vi);
                    if ((ti | tj) == SPANNING) {
                        auto t = (w - dot3(normal, csg.getVertex(vi.index))) / dot3(normal, sub3(csg.getVertex(vj.index), csg.getVertex(vi.index)));
                        auto newVi = vi.inverted == vj.inverted
                                     ? csg.newVertex(Vertex::interpolate(csg.vertices[vi.index], csg.vertices[vj.index], t))
                                     : csg.newVertex(Vertex::interpolate(csg.vertices[vi.index], Vertex(csg.vertices[vj.index]).flip(), t));
                        f.push_back({newVi, vi.inverted});
                        b.push_back({newVi, vi.inverted});
                    }
                }
                /*
                if (f.size() >= 3 && b.size() == 3) {
                    auto lengthSq = squareLength(cross3(
                            sub3(csg.vertices[b[1].index].vertex, csg.vertices[b[0].index].vertex),
                            sub3(csg.vertices[b[2].index].vertex, csg.vertices[b[0].index].vertex)));
                    if (lengthSq < PLANE_EPSILON) {
                        front.push_back(polygonIndex);
                        break;
                    }
                }
                if (f.size() == 3 && b.size() >= 3) {
                    auto lengthSq = squareLength(cross3(
                            sub3(csg.vertices[f[1].index].vertex, csg.vertices[f[0].index].vertex),
                            sub3(csg.vertices[f[2].index].vertex, csg.vertices[f[0].index].vertex)));
                    if (lengthSq < PLANE_EPSILON) {
                        back.push_back(polygonIndex);
                        break;
                    }
                }
                */
                if (f.size() >= 3)
                    front.push_back(csg.newPolygon(f));
                if (b.size() >= 3)
                    back.push_back(csg.newPolygon(b));
                break;
        }
        delete[] types;
    }

    Node & Node::invert() {
        std::deque<Node*> invertNodes{this};
        while (!invertNodes.empty()) {
            auto invertNode = invertNodes.front();
            invertNodes.pop_front();
            for (auto &polygonIndex: invertNode->polygons)
                csg.polygons[polygonIndex].flip();
            if (invertNode->planeSet)
                invertNode->plane.flip();
            std::swap(invertNode->front, invertNode->back);
            if (invertNode->back)
                invertNodes.push_back(invertNode->back);
            if (invertNode->front)
                invertNodes.push_back(invertNode->front);
        }
        return *this;
    }

    PolygonIndices Node::clipPolygons(const PolygonIndices &polygonsToSplit) const {
        if (!planeSet)
            return polygonsToSplit;

        struct ClipPolygons {
            const Node *node;
            uint32_t parent;
            PolygonIndices split;
        };
        std::vector<ClipPolygons> clipList{{this, 0, polygonsToSplit}};
        uint32_t current = 0;
        while (current < clipList.size()) {
            auto &clipNode = clipList[current];
            if (!clipNode.node->planeSet) {
                current++;
                continue;
            }
            PolygonIndices splitFront, splitBack;
            for (auto &polygon: clipNode.split)
                clipNode.node->plane.splitPolygon(csg, polygon, splitFront, splitBack, splitFront, splitBack);
            clipNode.split.clear();
            if (clipList[current].node->back)
                clipList.push_back({clipList[current].node->back, current, std::move(splitBack)});
            if (clipList[current].node->front)
                clipList.push_back({clipList[current].node->front, current, std::move(splitFront)});
            else
                clipList[current].split = std::move(splitFront);
            current++;
        }
        for (uint32_t i = clipList.size() - 1; i > 0; --i) {
            auto &clipNode = clipList[i];
            auto &parentNode = clipList[clipNode.parent];
            parentNode.split.insert(parentNode.split.end(), clipNode.split.begin(), clipNode.split.end());
        }
        return clipList[0].split;
    }

    Node & Node::clipTo(const Node &bsp) {
        std::deque<Node*> clipNodes{this};
        while (!clipNodes.empty()) {
            auto clipNode = clipNodes.front();
            clipNodes.pop_front();
            clipNode->polygons = bsp.clipPolygons(clipNode->polygons);
            if (clipNode->front)
                clipNodes.push_back(clipNode->front);
            if (clipNode->back)
                clipNodes.push_back(clipNode->back);
        }
        return *this;
    }

    void Node::allPolygons(PolygonIndices &polygonsCollection) const {
        std::deque<const Node*> polygonNodes{this};
        while (!polygonNodes.empty()) {
            auto polygonNode = polygonNodes.front();
            polygonNodes.pop_front();
            if (!polygonNode->polygons.empty())
                polygonsCollection.insert(polygonsCollection.end(), polygonNode->polygons.begin(), polygonNode->polygons.end());
            if (polygonNode->front)
                polygonNodes.push_back(polygonNode->front);
            if (polygonNode->back)
                polygonNodes.push_back(polygonNode->back);
        }
    }

    void Node::build(const PolygonIndices &newPolygons) {
        build(csg, this, newPolygons);
    }

    void Node::build(CSG &csg, Node *startNode, const PolygonIndices &newPolygons) {
        constexpr uint8_t COPLANAR = 0;
        constexpr uint8_t FRONT = 1;
        constexpr uint8_t BACK = 2;
        constexpr uint8_t SPANNING = 3;

        if (!startNode || newPolygons.empty())
            return;

        uint32_t maxNoOfType = 16;
        uint8_t *types = new uint8_t[maxNoOfType];
        struct BuildNode {
            Node *node = nullptr;
            PolygonIndices newPolygons;
        };
        std::deque<BuildNode> buildNodes;
        buildNodes.push_back({startNode, newPolygons});
        while (!buildNodes.empty()) {
            auto &node = buildNodes.front();

            if (!node.node->planeSet) {
                node.node->plane = csg.polygons[node.newPolygons[0]].plane;
                node.node->planeSet = true;
            }
            PolygonIndices newFront, newBack;
            for (auto &polygonIndex: node.newPolygons) {
                auto &polygon = csg.polygons[polygonIndex];
                auto &plane = node.node->plane;
                uint8_t polygonType = 0;
                uint32_t noOfVertices = polygon.vertices.size();
                if (maxNoOfType < noOfVertices) {
                    delete[] types;
                    maxNoOfType = noOfVertices;
                    types = new uint8_t[maxNoOfType];
                }
                for (uint32_t i = 0; i < noOfVertices; ++i) {
                    auto t = dot3(plane.normal, csg.getVertex(polygon.vertices[i].index)) - plane.w;
                    types[i] = (t < -Plane::epsilon) ? BACK : (t > Plane::epsilon) ? FRONT : COPLANAR;
                    polygonType |= types[i];
                }

                switch (polygonType) {
                    default:
                    case COPLANAR: node.node->polygons.push_back(polygonIndex); break;
                    case FRONT: newFront.push_back(polygonIndex); break;
                    case BACK: newBack.push_back(polygonIndex); break;
                    case SPANNING:
                        VertexIndices f, b;
                        for (uint32_t i = 0; i <noOfVertices; i++) {
                            uint32_t j = (i + 1) % polygon.vertices.size();
                            auto ti = types[i], tj = types[j];
                            auto vi = polygon.vertices[i], vj = polygon.vertices[j];
                            if (ti != BACK)
                                f.push_back(vi);
                            if (ti != FRONT)
                                b.push_back(vi);
                            if ((ti | tj) == SPANNING) {
                                auto t = (plane.w - dot3(plane.normal, csg.getVertex(vi.index))) / dot3(plane.normal, sub3(csg.getVertex(vj.index), csg.getVertex(vi.index)));
                                auto newVi = vi.inverted == vj.inverted
                                             ? csg.newVertex(Vertex::interpolate(csg.vertices[vi.index], csg.vertices[vj.index], t))
                                             : csg.newVertex(Vertex::interpolate(csg.vertices[vi.index], Vertex(csg.vertices[vj.index]).flip(), t));
                                f.push_back({newVi, vi.inverted});
                                b.push_back({newVi, vi.inverted});
                            }
                        }
                        /*
                        if (f.size() >= 3 && b.size() == 3) {
                            auto lengthSq = squareLength(cross3(
                                    sub3(csg.vertices[b[1].index].vertex, csg.vertices[b[0].index].vertex),
                                    sub3(csg.vertices[b[2].index].vertex, csg.vertices[b[0].index].vertex)));
                            if (lengthSq < PLANE_EPSILON) {
                                newFront.push_back(polygonIndex);
                                break;
                            }
                        }
                        if (f.size() == 3 && b.size() >= 3) {
                            auto lengthSq = squareLength(cross3(
                                    sub3(csg.vertices[f[1].index].vertex, csg.vertices[f[0].index].vertex),
                                    sub3(csg.vertices[f[2].index].vertex, csg.vertices[f[0].index].vertex)));
                            if (lengthSq < PLANE_EPSILON) {
                                newBack.push_back(polygonIndex);
                                break;
                            }
                        }
                        */
                        if (f.size() >= 3)
                            newFront.push_back(csg.newPolygon(f));
                        if (b.size() >= 3)
                            newBack.push_back(csg.newPolygon(b));
                        break;
                }
            }
            if (!newFront.empty()) {
                if (node.node->polygons.empty() && newBack.empty()) {
                    node.node->polygons = newFront;
                } else {
                    if (!node.node->front)
                        node.node->front = new Node(csg);
                    buildNodes.push_back({node.node->front, std::move(newFront)});
                }
            }
            if (!newBack.empty()) {
                if (node.node->polygons.empty() && newFront.empty()) {
                    node.node->polygons = newBack;
                } else {
                    if (!node.node->back)
                        node.node->back = new Node(csg);
                    buildNodes.push_back({node.node->back, std::move(newBack)});
                }
            }
            buildNodes.pop_front();
        }
        delete[] types;
    }

    uint32_t CSG::newVertex(const Vertex &vertex) {
        uint32_t newVertexIndex = vertices.size();
        vertices.push_back(vertex);
        return newVertexIndex;
    }

    PolygonIndex CSG::newPolygon(const VertexIndices &vertexIndices) {
        PolygonIndex newPolygonIndex = polygons.size();
        polygons.push_back({
               vertexIndices,
               Plane::fromPoints(vertices[vertexIndices[0].index].vertex, vertices[vertexIndices[1].index].vertex, vertices[vertexIndices[2].index].vertex)
        });
        return newPolygonIndex;
    }

    PolygonIndices polygonsFromMeshSpecification(CSG &csg, const mesh::MeshData &mesh);
    void polygonsToMesh(const CSG &csg, const PolygonIndices *polygonIndices, mesh::ResultMeshData &mesh);
    PolygonIndices meshOperation(CSG &csg, mesh::Operator op, const mesh::MeshData &meshA, const mesh::MeshData &meshB);
    void meshOperation(mesh::Operator op, const mesh::MeshData &meshA, const mesh::MeshData &meshB, mesh::ResultMeshData &resultMesh);
}

#endif