#include "pch.h"
#include "CppUnitTest.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

// [Write unit tests for C/C++ in Visual Studio](https://docs.microsoft.com/en-us/visualstudio/test/writing-unit-tests-for-c-cpp?view=vs-2019)

#include <mesh/mesh_definition_hexahedron.h>
#include <glm/glm.hpp>

using namespace mesh;

namespace mesh_test
{
    // Hexahedron https://en.wikipedia.org/wiki/Hexahedron
    TEST_CLASS(utility_mesh_definition_hexahedron_test)
    {
    public:

        TEST_METHOD(hexahedron_vertices_test)
        {
            auto hexahedron_definition = MeshDefinitonHexahedron<>();
            auto hexahedron_mesh_data = hexahedron_definition.generate_mesh_data();

            auto [no_of_values, vertex_array] = hexahedron_mesh_data->get_vertex_attributes();
            auto specification = hexahedron_mesh_data->get_specification();
            auto attribute_size = hexahedron_mesh_data->get_attribute_size();

            Assert::AreEqual(static_cast<size_t>(0), no_of_values % attribute_size);
            Assert::AreEqual(attribute_size * 24, no_of_values);
        }

        TEST_METHOD(hexahedron_indices_test)
        {
            auto hexahedron_definition = MeshDefinitonHexahedron<>();
            auto hexahedron_mesh_data = hexahedron_definition.generate_mesh_data();

            auto [no_of_indices, index_array] = hexahedron_mesh_data->get_indices();

            Assert::AreEqual(static_cast<size_t>(36), no_of_indices);
        }

        TEST_METHOD(hexahedron_side_len_test)
        {
            auto hexahedron_definition = MeshDefinitonHexahedron<>();
            auto hexahedron_mesh_data = hexahedron_definition.generate_mesh_data();

            auto [no_of_values, vertex_array] = hexahedron_mesh_data->get_vertex_attributes();
            auto [no_of_indices, index_array] = hexahedron_mesh_data->get_indices();
            auto specification = hexahedron_mesh_data->get_specification();
            auto attribute_size = hexahedron_mesh_data->get_attribute_size();

            float expected_length_1 = 1.0f;
            float expected_length_2 = std::sqrt(2.0f);
            for (int i = 0; i < no_of_indices; i += 3)
            {
                for (int j = 0; j < 3; ++j)
                {
                    int i1 = i + j;
                    int i2 = i + (j + 1) % 3;
                    auto v1 = glm::vec3(
                        vertex_array[index_array[i1] * attribute_size],
                        vertex_array[index_array[i1] * attribute_size + 1],
                        vertex_array[index_array[i1] * attribute_size + 2]);
                    auto v2 = glm::vec3(
                        vertex_array[index_array[i2] * attribute_size],
                        vertex_array[index_array[i2] * attribute_size + 1],
                        vertex_array[index_array[i2] * attribute_size + 2]);
                    auto length = glm::distance(v1, v2);
                    Assert::IsTrue(fabs(expected_length_1 - length) < 0.0001f || fabs(expected_length_2 - length) < 0.0001f);
                }
            }
        }
    };
}