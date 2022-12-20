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
            const emscripten::val &meshData, 
            const char *propertyName) {
        emscripten::val buffer = meshData[propertyName];
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

    mesh::ResultMeshData calculateMeshIntersection(emscripten::val mesh0, emscripten::val mesh1)
    {
        emscripten::val heapMemory = emscripten::val::module_property("HEAPU8")["buffer"];
        if (heapMemory.isUndefined()) {
            mesh::ResultMeshData result;
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
        copyBufferData(uvsMesh0, heapMemory, mesh1, "uvs");
        std::vector<uint32_t> indicesMesh1;
        copyBufferData(indicesMesh1, heapMemory, mesh1, "indices");

        mesh::MeshData meshData0{
            (uint32_t)verticesMesh0.size() / 3, (uint32_t)indicesMesh0.size(),
            verticesMesh0.data(), 
            normalsMesh0.empty() ? nullptr : normalsMesh0.data(), 
            uvsMesh0.empty() ? nullptr : uvsMesh0.data(), 
            indicesMesh0.data(),
            getMin(verticesMesh0), getMax(verticesMesh0)
        };
        mesh::MeshData meshData1{
            (uint32_t)verticesMesh1.size() / 3, (uint32_t)indicesMesh1.size(),
            verticesMesh1.data(), 
            normalsMesh1.empty() ? nullptr : normalsMesh1.data(),
            uvsMesh1.empty() ? nullptr : uvsMesh1.data(), 
            indicesMesh1.data(),
            getMin(verticesMesh1), getMax(verticesMesh1)
        };

        mesh::MeshIntersection intersection(meshData0, meshData1);
        intersection.intersect();
        auto actualResult = intersection.getResult0();

        return actualResult;
    }

    mesh::ResultMeshData meshOperator(emscripten::val mesh0, emscripten::val mesh1, mesh::MeshIntersection::Operator meshOperator)
    {
        emscripten::val heapMemory = emscripten::val::module_property("HEAPU8")["buffer"];
        if (heapMemory.isUndefined()) {
            mesh::ResultMeshData result;
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
        copyBufferData(uvsMesh0, heapMemory, mesh1, "uvs");
        std::vector<uint32_t> indicesMesh1;
        copyBufferData(indicesMesh1, heapMemory, mesh1, "indices");

        mesh::MeshData meshData0{
            (uint32_t)verticesMesh0.size() / 3, (uint32_t)indicesMesh0.size(),
            verticesMesh0.data(), 
            normalsMesh0.empty() ? nullptr : normalsMesh0.data(), 
            uvsMesh0.empty() ? nullptr : uvsMesh0.data(), 
            indicesMesh0.data(),
            getMin(verticesMesh0), getMax(verticesMesh0)
        };
        mesh::MeshData meshData1{
            (uint32_t)verticesMesh1.size() / 3, (uint32_t)indicesMesh1.size(),
            verticesMesh1.data(), 
            normalsMesh1.empty() ? nullptr : normalsMesh1.data(),
            uvsMesh1.empty() ? nullptr : uvsMesh1.data(), 
            indicesMesh1.data(),
            getMin(verticesMesh1), getMax(verticesMesh1)
        };

        mesh::MeshIntersection intersection(meshData0, meshData1);
        intersection.operate(meshOperator);
        auto resultData = intersection.getResult0();
        return resultData;
    }

    mesh::ResultMeshData meshOperatorMinusAB(emscripten::val mesh0, emscripten::val mesh1) {
        return meshOperator(mesh0, mesh1, mesh::MeshIntersection::Operator::MINUS);
    }

    mesh::ResultMeshData meshOperatorOrAB(emscripten::val mesh0, emscripten::val mesh1) {
        return meshOperator(mesh0, mesh1, mesh::MeshIntersection::Operator::OR);
    }

    mesh::ResultMeshData meshOperatorAndAB(emscripten::val mesh0, emscripten::val mesh1) {
        return meshOperator(mesh0, mesh1, mesh::MeshIntersection::Operator::AND);
    }
}

EMSCRIPTEN_BINDINGS(my_module)
{
    emscripten::register_vector<uint32_t>("Uint32Array");
    emscripten::register_vector<float>("Float32Array");

    emscripten::value_object<mesh::ResultMeshData>("ResultMeshData")
        .field<mesh::ResultMeshData, std::vector<float>>("vertices", &mesh::ResultMeshData::vertices)
        .field<mesh::ResultMeshData, std::vector<float>>("normals", &mesh::ResultMeshData::normals)
        .field<mesh::ResultMeshData, std::vector<float>>("uvs", &mesh::ResultMeshData::uvs)
        .field<mesh::ResultMeshData, std::vector<uint32_t>>("indicesOut", &mesh::ResultMeshData::indicesOut)
        .field<mesh::ResultMeshData, std::vector<uint32_t>>("indicesIn", &mesh::ResultMeshData::indicesIn)
        .field<mesh::ResultMeshData, bool>("error", &mesh::ResultMeshData::error);

    emscripten::function<std::string>("getVersion", &getVersion);
    emscripten::function<void, emscripten::val>("setIOContext", &setIOContext);
    emscripten::function<mesh::ResultMeshData, emscripten::val, emscripten::val>("calculateMeshIntersection", &calculateMeshIntersection);
    emscripten::function<mesh::ResultMeshData, emscripten::val, emscripten::val>("meshOperatorMinusAB", &meshOperatorMinusAB);
    emscripten::function<mesh::ResultMeshData, emscripten::val, emscripten::val>("meshOperatorOrAB", &meshOperatorOrAB);
    emscripten::function<mesh::ResultMeshData, emscripten::val, emscripten::val>("meshOperatorAndAB", &meshOperatorAndAB);
}