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
    using NodeIndex = int32_t;

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
        bool planeSet = false;
        Plane plane;
        NodeIndex front = -1;
        NodeIndex back = -1;
        PolygonIndices polygons;

        static inline NodeIndex constructNode(CSG &csg, const PolygonIndices &nodePolygons);

        explicit Node() = default;
        Node(const Node&) = delete;
        Node(Node&&) = default;
        Node& operator = (const Node&) = delete;
        Node& operator = (Node&&) = delete;
    };

    class CSG {
    public:
        std::vector<Vertex> vertices;
        std::vector<Polygon> polygons;
        std::vector<Node> nodes;

        [[nodiscard]] const Scalar* getVertex(uint32_t i) const {
            return (vertices.data() + i)->vertex.data();
        }

        [[nodiscard]] Node* getNode(int32_t i) {
            return (nodes.data() + i);
        }

        [[nodiscard]] const Node* getNode(int32_t i) const  {
            return (nodes.data() + i);
        }

        [[nodiscard]] PolygonIndices allPolygons(NodeIndex startNodeIndex) const {
            PolygonIndices polygonsCollection;
            allPolygons(startNodeIndex, polygonsCollection);
            return polygonsCollection;
        }

        inline uint32_t newVertex(const Vertex &vertex);
        inline PolygonIndex newPolygon(const VertexIndices &vertexIndices);
        inline NodeIndex newNode();
        inline void allPolygons(NodeIndex startNodeIndex, PolygonIndices &polygonsCollection) const;
        inline void invert(NodeIndex startNodeIndex);
        [[nodiscard]] inline PolygonIndices clipPolygons(NodeIndex startNodeIndex, const PolygonIndices &polygonsToSplit);
        inline void clipTo(NodeIndex startNodeIndex, NodeIndex bsp);
        inline void build(NodeIndex startNodeIndex, const PolygonIndices &newPolygons);
        PolygonIndices operatorUnion(PolygonIndices &polygonsA, PolygonIndices &polygonsB);
        PolygonIndices operatorSubtract(PolygonIndices &polygonsA, PolygonIndices &polygonsB);
        PolygonIndices operatorIntersect(PolygonIndices &polygonsA, PolygonIndices &polygonsB);
    };

    void Plane::splitPolygon(CSG &csg, PolygonIndex polygonIndex, PolygonIndices &coplanarFront, PolygonIndices &coplanarBack, PolygonIndices &front, PolygonIndices &back) const {
        constexpr uint8_t COPLANAR_POLYGON = 0;
        constexpr uint8_t FRONT_POLYGON = 1;
        constexpr uint8_t BACK_POLYGON = 2;
        constexpr uint8_t SPANNING_POLYGON = 3;

        auto &polygon = csg.polygons[polygonIndex];
        uint8_t polygonType = 0;
        uint32_t noOfVertices = polygon.vertices.size();
        auto types = new uint8_t[noOfVertices];
        for (uint32_t i = 0; i < noOfVertices; ++i) {
            auto t = dot3(normal, csg.getVertex(polygon.vertices[i].index)) - w;
            types[i] = (t < -epsilon) ? BACK_POLYGON : (t > epsilon) ? FRONT_POLYGON : COPLANAR_POLYGON;
            polygonType |= types[i];
        }

        switch (polygonType) {
            default:
            case COPLANAR_POLYGON: dot3(normal, polygon.plane.normal) > 0 ? coplanarFront.push_back(polygonIndex) : coplanarBack.push_back(polygonIndex); break;
            case FRONT_POLYGON: front.push_back(polygonIndex); break;
            case BACK_POLYGON: back.push_back(polygonIndex); break;
            case SPANNING_POLYGON:
                VertexIndices f, b;
                for (uint32_t i = 0; i <noOfVertices; i++) {
                    uint32_t j = (i + 1) % polygon.vertices.size();
                    auto ti = types[i], tj = types[j];
                    auto vi = polygon.vertices[i], vj = polygon.vertices[j];
                    if (ti != BACK_POLYGON)
                        f.push_back(vi);
                    if (ti != FRONT_POLYGON)
                        b.push_back(vi);
                    if ((ti | tj) == SPANNING_POLYGON) {
                        auto t = (w - dot3(normal, csg.getVertex(vi.index))) / dot3(normal, sub3(csg.getVertex(vj.index), csg.getVertex(vi.index)));
                        auto newVi = vi.inverted == vj.inverted
                                     ? csg.newVertex(Vertex::interpolate(csg.vertices[vi.index], csg.vertices[vj.index], t))
                                     : csg.newVertex(Vertex::interpolate(csg.vertices[vi.index], Vertex(csg.vertices[vj.index]).flip(), t));
                        f.push_back({newVi, vi.inverted});
                        b.push_back({newVi, vi.inverted});
                    }
                }
                if (f.size() >= 3)
                    front.push_back(csg.newPolygon(f));
                if (b.size() >= 3)
                    back.push_back(csg.newPolygon(b));
                break;
        }
        delete[] types;
    }

    NodeIndex Node::constructNode(CSG &csg, const PolygonIndices &nodePolygons) {
        NodeIndex nodeIndex = csg.newNode();
        if (!nodePolygons.empty())
            csg.build(nodeIndex, nodePolygons);
        return nodeIndex;
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

    NodeIndex CSG::newNode() {
        auto newNodeIndex = static_cast<NodeIndex>(nodes.size());
        nodes.emplace_back();
        return newNodeIndex;
    }

    void CSG::allPolygons(NodeIndex startNodeIndex, PolygonIndices &polygonsCollection) const {
        std::deque<NodeIndex> polygonNodes{startNodeIndex};
        while (!polygonNodes.empty()) {
            auto polygonNode = getNode(polygonNodes.front());
            polygonNodes.pop_front();
            if (!polygonNode->polygons.empty())
                polygonsCollection.insert(polygonsCollection.end(), polygonNode->polygons.begin(), polygonNode->polygons.end());
            if (polygonNode->front >= 0)
                polygonNodes.push_back(polygonNode->front);
            if (polygonNode->back >= 0)
                polygonNodes.push_back(polygonNode->back);
        }
    }

    void CSG::invert(NodeIndex startNodeIndex) {
        std::deque<NodeIndex> invertNodes{startNodeIndex};
        while (!invertNodes.empty()) {
            auto invertNode = getNode(invertNodes.front());
            invertNodes.pop_front();
            for (auto &polygonIndex: invertNode->polygons)
                polygons[polygonIndex].flip();
            if (invertNode->planeSet)
                invertNode->plane.flip();
            std::swap(invertNode->front, invertNode->back);
            if (invertNode->back >= 0)
                invertNodes.push_back(invertNode->back);
            if (invertNode->front >= 0)
                invertNodes.push_back(invertNode->front);
        }
    }

    PolygonIndices CSG::clipPolygons(NodeIndex startNodeIndex, const PolygonIndices &polygonsToSplit) {
        if (!getNode(startNodeIndex)->planeSet)
            return polygonsToSplit;

        struct ClipPolygons {
            NodeIndex nodeIndex;
            uint32_t parent;
            PolygonIndices split;
        };
        std::vector<ClipPolygons> clipList{{startNodeIndex, 0, polygonsToSplit}};
        uint32_t current = 0;
        while (current < clipList.size()) {
            auto &clipNode = clipList[current];
            if (!getNode(clipNode.nodeIndex)->planeSet) {
                current++;
                continue;
            }
            PolygonIndices splitFront, splitBack;
            for (auto &polygon: clipNode.split)
                getNode(clipNode.nodeIndex)->plane.splitPolygon(*this, polygon, splitFront, splitBack, splitFront, splitBack);
            clipNode.split.clear();
            if (getNode(clipList[current].nodeIndex)->back >= 0)
                clipList.push_back({getNode(clipList[current].nodeIndex)->back, current, std::move(splitBack)});
            if (getNode(clipList[current].nodeIndex)->front >= 0)
                clipList.push_back({getNode(clipList[current].nodeIndex)->front, current, std::move(splitFront)});
            else
                clipList[current].split = std::move(splitFront);
            current++;
        }
        for (uint32_t i = clipList.size() - 1; i > 0; --i) {
            auto parentI = clipList[i].parent;
            clipList[parentI].split.insert(clipList[parentI].split.end(), clipList[i].split.begin(), clipList[i].split.end());
        }
        return clipList[0].split;
    }

    void CSG::clipTo(NodeIndex startNodeIndex, NodeIndex bsp) {
        std::deque<NodeIndex> clipNodes{startNodeIndex};
        while (!clipNodes.empty()) {
            auto clipNode = getNode(clipNodes.front());
            clipNodes.pop_front();
            clipNode->polygons = clipPolygons(bsp, clipNode->polygons);
            if (clipNode->front >= 0)
                clipNodes.push_back(clipNode->front);
            if (clipNode->back >= 0)
                clipNodes.push_back(clipNode->back);
        }
    }

    void CSG::build(NodeIndex startNodeIndex, const PolygonIndices &newPolygons) {
        constexpr uint8_t COPLANAR = 0;
        constexpr uint8_t FRONT = 1;
        constexpr uint8_t BACK = 2;
        constexpr uint8_t SPANNING = 3;

        if (startNodeIndex < 0 || newPolygons.empty())
            return;

        uint32_t maxNoOfType = 16;
        auto types = new uint8_t[maxNoOfType];
        struct BuildNode {
            NodeIndex node = -1;
            PolygonIndices newPolygons;
        };
        std::deque<BuildNode> buildNodes;
        buildNodes.push_back({startNodeIndex, newPolygons});
        while (!buildNodes.empty()) {
            auto &node = buildNodes.front();
            auto nodeNode = getNode(node.node);

            if (!nodeNode->planeSet) {
                nodeNode->plane = polygons[node.newPolygons[0]].plane;
                nodeNode->planeSet = true;
            }
            PolygonIndices newFront, newBack;
            for (auto &polygonIndex: node.newPolygons) {
                auto &polygon = polygons[polygonIndex];
                auto &plane = nodeNode->plane;
                uint8_t polygonType = 0;
                uint32_t noOfVertices = polygon.vertices.size();
                if (maxNoOfType < noOfVertices) {
                    delete[] types;
                    maxNoOfType = noOfVertices;
                    types = new uint8_t[maxNoOfType];
                }
                for (uint32_t i = 0; i < noOfVertices; ++i) {
                    auto t = dot3(plane.normal, getVertex(polygon.vertices[i].index)) - plane.w;
                    types[i] = (t < -Plane::epsilon) ? BACK : (t > Plane::epsilon) ? FRONT : COPLANAR;
                    polygonType |= types[i];
                }

                switch (polygonType) {
                    default:
                    case COPLANAR: nodeNode->polygons.push_back(polygonIndex); break;
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
                                auto t = (plane.w - dot3(plane.normal, getVertex(vi.index))) / dot3(plane.normal, sub3(getVertex(vj.index), getVertex(vi.index)));
                                auto newVi = vi.inverted == vj.inverted
                                             ? newVertex(Vertex::interpolate(vertices[vi.index], vertices[vj.index], t))
                                             : newVertex(Vertex::interpolate(vertices[vi.index], Vertex(vertices[vj.index]).flip(), t));
                                f.push_back({newVi, vi.inverted});
                                b.push_back({newVi, vi.inverted});
                            }
                        }
                        if (f.size() >= 3)
                            newFront.push_back(newPolygon(f));
                        if (b.size() >= 3)
                            newBack.push_back(newPolygon(b));
                        break;
                }
            }
            if (!newFront.empty()) {
                auto frontNode = nodeNode->front;
                if (frontNode < 0)
                    getNode(buildNodes.front().node)->front = frontNode = newNode();
                buildNodes.push_back({frontNode, std::move(newFront)});
            }
            if (!newBack.empty()) {
                auto backNode = nodeNode->back;
                if (backNode < 0)
                    getNode(buildNodes.front().node)->back = backNode = newNode();
                buildNodes.push_back({backNode, std::move(newBack)});
            }
            buildNodes.pop_front();
        }
        delete[] types;
    }

    PolygonIndices polygonsFromMeshSpecification(CSG &csg, const mesh::MeshData &mesh);
    void polygonsToMesh(const CSG &csg, const PolygonIndices *polygonIndices, mesh::ResultMeshData &mesh);
    PolygonIndices meshOperation(CSG &csg, mesh::Operator op, const mesh::MeshData &meshA, const mesh::MeshData &meshB);
    void meshOperation(mesh::Operator op, const mesh::MeshData &meshA, const mesh::MeshData &meshB, mesh::ResultMeshData &resultMesh);
}

#endif