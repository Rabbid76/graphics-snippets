#include "mesh_utility/constructiveSolidGeometry.h"
#include "mesh_utility/meshIntersection.h"
#include <emscripten/bind.h>
#include <emscripten/val.h>
#include <algorithm>
#include <iostream>
#include <sstream>
#include <string>

emscripten::val globalIOContext = emscripten::val::null();

namespace
{
    template<typename DATA_TYPE>
    static void copyBufferData(
            std::vector<DATA_TYPE> &data, 
            const emscripten::val heapMemory, 
            const emscripten::val &MeshDataReference, 
            const char *propertyName) {
        emscripten::val buffer = MeshDataReference[propertyName];
        if (buffer.isUndefined()) 
            return;
        unsigned int bufferLength = buffer["length"].as<unsigned int>();
        data.resize(buffer["length"].as<unsigned int>());
        emscripten::val memoryView = buffer["constructor"].new_(heapMemory, reinterpret_cast<uintptr_t>(data.data()), bufferLength);
        memoryView.call<void>("set", buffer);
    }

    mesh::Point3 getMin(const std::vector<float> &vertices) {
        mesh::Point3 p{vertices[0], vertices[1], vertices[2]};
        for (uint32_t i = 3; i < vertices.size(); i += 3) {
            p[0] = std::min(p[0], vertices[i]);
            p[1] = std::min(p[1], vertices[i+1]);
            p[2] = std::min(p[2], vertices[i+2]);
        }
        return p;
    }

    mesh::Point3 getMax(const std::vector<float> &vertices) {
        mesh::Point3 p{vertices[0], vertices[1], vertices[2]};
        for (uint32_t i = 3; i < vertices.size(); i += 3) {
            p[0] = std::max(p[0], vertices[i]);
            p[1] = std::max(p[1], vertices[i+1]);
            p[2] = std::max(p[2], vertices[i+2]);
        }
        return p;
    }

    /*
    void coutMeshData(const mesh::MeshDataReference &MeshDataReference) {
        std::cout << "vertices (" << MeshDataReference.noOfVertices * 3 << "): {";
        for (uint32_t i = 0; i < MeshDataReference.noOfVertices * 3; ++i)
            std::cout << MeshDataReference.vertices[i] << ", ";
        std::cout << "};" << std::endl;
        std::cout << "normals (" << MeshDataReference.noOfVertices * 3 << "): {";
        for (uint32_t i = 0; i < MeshDataReference.noOfVertices * 3; ++i)
            std::cout << MeshDataReference.normals[i] << ", ";
        std::cout << "};" << std::endl;
        std::cout << "uvs (" << MeshDataReference.noOfVertices * 2 << "): {";
        for (uint32_t i = 0; i < MeshDataReference.noOfVertices * 2; ++i)
            std::cout << MeshDataReference.uvs[i] << ", ";
        std::cout << "};" << std::endl;
        std::cout << "indices out (" << MeshDataReference.noOfIndices << "): {";
        for (uint32_t i = 0; i < MeshDataReference.noOfIndices; ++i)
            std::cout << MeshDataReference.indices[i] << ", ";
        std::cout << "};" << std::endl;
    }

    void coutResultMeshData(const mesh::MeshDataInstance &MeshDataReference) {
        std::cout << "vertices (" << MeshDataReference.vertices.size() << "): {";
        for (auto &vertex: MeshDataReference.vertices)
            std::cout << vertex << ", ";
        std::cout << "};" << std::endl;
        std::cout << "normals (" << MeshDataReference.normals.size() << "): {";
        for (auto &vertex: MeshDataReference.normals)
            std::cout << vertex << ", ";
        std::cout << "};" << std::endl;
        std::cout << "uvs (" << MeshDataReference.uvs.size() << "): {";
        for (auto &vertex: MeshDataReference.uvs)
            std::cout << vertex << ", ";
        std::cout << "};" << std::endl;
        std::cout << "indices out (" << MeshDataReference.indicesOut.size() << "): {";
        for (auto &index: MeshDataReference.indicesOut)
            std::cout << index << ", ";
        std::cout << "};" << std::endl;
    }
    */

    void log(const std::string &message)
    {
        if (!globalIOContext.isNull() && globalIOContext.hasOwnProperty("log"))
        {
            globalIOContext.call<void>("log", "LOG: " + message);
        }
        else
        {
            std::cout << "stream-log: " << message << std::endl;
        }
    }

    std::string getVersion()
    {
        log("version: 1.0.0");
        return "1.0.0";
    }

    void setIOContext(emscripten::val ioContext)
    {
        globalIOContext = ioContext;
    }

    mesh::MeshDataInstance calculateMeshIntersection(emscripten::val mesh0, emscripten::val mesh1)
    {
        emscripten::val heapMemory = emscripten::val::module_property("HEAPU8")["buffer"];
        if (heapMemory.isUndefined()) {
            mesh::MeshDataInstance result;
            result.error = true;
            return result;
        }

        std::vector<float> verticesMesh0;
        copyBufferData(verticesMesh0, heapMemory, mesh0, "vertices");
        std::vector<float> normalsMesh0;
        copyBufferData(normalsMesh0, heapMemory, mesh0, "normals");
        std::vector<float> uvsMesh0;
        copyBufferData(uvsMesh0, heapMemory, mesh0, "uvs");
        std::vector<uint32_t> indicesMesh0;
        copyBufferData(indicesMesh0, heapMemory, mesh0, "indices");

        std::vector<float> verticesMesh1;
        copyBufferData(verticesMesh1, heapMemory, mesh1, "vertices");
        std::vector<float> normalsMesh1;
        copyBufferData(normalsMesh1, heapMemory, mesh1, "normals");
        std::vector<float> uvsMesh1;
        copyBufferData(uvsMesh1, heapMemory, mesh1, "uvs");
        std::vector<uint32_t> indicesMesh1;
        copyBufferData(indicesMesh1, heapMemory, mesh1, "indices");

        auto meshData0 = mesh::MeshDataReference::creatFromVectors(indicesMesh0, verticesMesh0, normalsMesh0, uvsMesh0, getMin(verticesMesh0), getMax(verticesMesh0));
        auto meshData1 = mesh::MeshDataReference::creatFromVectors(indicesMesh1, verticesMesh1, normalsMesh1, uvsMesh1, getMin(verticesMesh1), getMax(verticesMesh1));
        mesh::MeshIntersection intersection(meshData0, meshData1);
        intersection.intersect();
        auto actualResult = intersection.getResult0();

        return actualResult;
    }

    mesh::MeshDataInstance meshOperator(emscripten::val mesh0, emscripten::val mesh1, mesh::Operator meshOperator)
    {
        emscripten::val heapMemory = emscripten::val::module_property("HEAPU8")["buffer"];
        if (heapMemory.isUndefined()) {
            mesh::MeshDataInstance result;
            result.error = true;
            return result;
        }

        std::vector<float> verticesMesh0;
        copyBufferData(verticesMesh0, heapMemory, mesh0, "vertices");
        std::vector<float> normalsMesh0;
        copyBufferData(normalsMesh0, heapMemory, mesh0, "normals");
        std::vector<float> uvsMesh0;
        copyBufferData(uvsMesh0, heapMemory, mesh0, "uvs");
        std::vector<uint32_t> indicesMesh0;
        copyBufferData(indicesMesh0, heapMemory, mesh0, "indices");

        std::vector<float> verticesMesh1;
        copyBufferData(verticesMesh1, heapMemory, mesh1, "vertices");
        std::vector<float> normalsMesh1;
        copyBufferData(normalsMesh1, heapMemory, mesh1, "normals");
        std::vector<float> uvsMesh1;
        copyBufferData(uvsMesh1, heapMemory, mesh1, "uvs");
        std::vector<uint32_t> indicesMesh1;
        copyBufferData(indicesMesh1, heapMemory, mesh1, "indices");

        auto meshData0 = mesh::MeshDataReference::creatFromVectors(indicesMesh0, verticesMesh0, normalsMesh0, uvsMesh0, getMin(verticesMesh0), getMax(verticesMesh0));
        auto meshData1 = mesh::MeshDataReference::creatFromVectors(indicesMesh1, verticesMesh1, normalsMesh1, uvsMesh1, getMin(verticesMesh1), getMax(verticesMesh1));
        mesh::MeshIntersection intersection(meshData0, meshData1);
        intersection.operate(meshOperator);
        auto resultData = intersection.getResult0();
        return resultData;
    }

    mesh::MeshDataInstance meshCSG(emscripten::val mesh0, emscripten::val mesh1, mesh::Operator meshOperator)
    {
        emscripten::val heapMemory = emscripten::val::module_property("HEAPU8")["buffer"];
        if (heapMemory.isUndefined()) {
            mesh::MeshDataInstance result;
            result.error = true;
            return result;
        }

        std::vector<float> verticesMesh0;
        copyBufferData(verticesMesh0, heapMemory, mesh0, "vertices");
        std::vector<float> normalsMesh0;
        copyBufferData(normalsMesh0, heapMemory, mesh0, "normals");
        std::vector<float> uvsMesh0;
        copyBufferData(uvsMesh0, heapMemory, mesh0, "uvs");
        std::vector<uint32_t> indicesMesh0;
        copyBufferData(indicesMesh0, heapMemory, mesh0, "indices");

        std::vector<float> verticesMesh1;
        copyBufferData(verticesMesh1, heapMemory, mesh1, "vertices");
        std::vector<float> normalsMesh1;
        copyBufferData(normalsMesh1, heapMemory, mesh1, "normals");
        std::vector<float> uvsMesh1;
        copyBufferData(uvsMesh1, heapMemory, mesh1, "uvs");
        std::vector<uint32_t> indicesMesh1;
        copyBufferData(indicesMesh1, heapMemory, mesh1, "indices");

        auto meshData0 = mesh::MeshDataReference::creatFromVectors(indicesMesh0, verticesMesh0, normalsMesh0, uvsMesh0, getMin(verticesMesh0), getMax(verticesMesh0));
        auto meshData1 = mesh::MeshDataReference::creatFromVectors(indicesMesh1, verticesMesh1, normalsMesh1, uvsMesh1, getMin(verticesMesh1), getMax(verticesMesh1));

        //std::cout << "mesh A" << std::endl;
        //coutMeshData(meshData0);
        //std::cout << "mesh B" << std::endl;
        //coutMeshData(meshData1);

        //mesh::MeshDataInstance resultData;
        //csg::meshOperation(meshOperator, meshData0, meshData1, resultData);

        std::vector<uint32_t> triangles0, triangles1;
        mesh::MeshDataReference::findIntersecting(meshData0, meshData1, triangles0, triangles1);
        mesh::MeshDataInstance resultData;
        csg::meshOperation(meshOperator, meshData0, meshData1, triangles0, triangles1, resultData);

        //std::cout << "result" << std::endl;
        //coutResultMeshData(resultData);

        return resultData;
    }

    mesh::MeshDataInstance meshOperatorMinusAB(emscripten::val mesh0, emscripten::val mesh1) {
        return meshOperator(mesh0, mesh1, mesh::Operator::MINUS);
    }

    mesh::MeshDataInstance meshOperatorOrAB(emscripten::val mesh0, emscripten::val mesh1) {
        return meshOperator(mesh0, mesh1, mesh::Operator::OR);
    }

    mesh::MeshDataInstance meshOperatorAndAB(emscripten::val mesh0, emscripten::val mesh1) {
        return meshOperator(mesh0, mesh1, mesh::Operator::AND);
    }

    mesh::MeshDataInstance meshCSGMinusAB(emscripten::val mesh0, emscripten::val mesh1) {
        return meshCSG(mesh0, mesh1, mesh::Operator::MINUS);
    }

    mesh::MeshDataInstance meshCSGOrAB(emscripten::val mesh0, emscripten::val mesh1) {
        return meshCSG(mesh0, mesh1, mesh::Operator::OR);
    }

    mesh::MeshDataInstance meshCSGAndAB(emscripten::val mesh0, emscripten::val mesh1) {
        return meshCSG(mesh0, mesh1, mesh::Operator::AND);
    }
}

EMSCRIPTEN_BINDINGS(my_module)
{
    emscripten::register_vector<uint32_t>("Uint32Array");
    emscripten::register_vector<float>("Float32Array");

    emscripten::value_object<mesh::MeshDataInstance>("MeshDataInstance")
        .field<mesh::MeshDataInstance, std::vector<float>>("vertices", &mesh::MeshDataInstance::vertices)
        .field<mesh::MeshDataInstance, std::vector<float>>("normals", &mesh::MeshDataInstance::normals)
        .field<mesh::MeshDataInstance, std::vector<float>>("uvs", &mesh::MeshDataInstance::uvs)
        .field<mesh::MeshDataInstance, std::vector<uint32_t>>("indicesOut", &mesh::MeshDataInstance::indicesOut)
        .field<mesh::MeshDataInstance, std::vector<uint32_t>>("indicesIn", &mesh::MeshDataInstance::indicesIn)
        .field<mesh::MeshDataInstance, bool>("error", &mesh::MeshDataInstance::error);

    emscripten::function<std::string>("getVersion", &getVersion);
    emscripten::function<void, emscripten::val>("setIOContext", &setIOContext);
    emscripten::function<mesh::MeshDataInstance, emscripten::val, emscripten::val>("calculateMeshIntersection", &calculateMeshIntersection);
    emscripten::function<mesh::MeshDataInstance, emscripten::val, emscripten::val>("meshOperatorMinusAB", &meshOperatorMinusAB);
    emscripten::function<mesh::MeshDataInstance, emscripten::val, emscripten::val>("meshOperatorOrAB", &meshOperatorOrAB);
    emscripten::function<mesh::MeshDataInstance, emscripten::val, emscripten::val>("meshOperatorAndAB", &meshOperatorAndAB);
    emscripten::function<mesh::MeshDataInstance, emscripten::val, emscripten::val>("meshCSGMinusAB", &meshCSGMinusAB);
    emscripten::function<mesh::MeshDataInstance, emscripten::val, emscripten::val>("meshCSGOrAB", &meshCSGOrAB);
    emscripten::function<mesh::MeshDataInstance, emscripten::val, emscripten::val>("meshCSGAndAB", &meshCSGAndAB);
}