#include "pch.h"
#include "CppUnitTest.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

// [Write unit tests for C/C++ in Visual Studio](https://docs.microsoft.com/en-us/visualstudio/test/writing-unit-tests-for-c-cpp?view=vs-2019)

#include <mesh/mesh_definition_dodecahedron.h>
#include <glm/glm.hpp>

using namespace mesh;

namespace mesh_test
{
    // Dodecahedron https://en.wikipedia.org/wiki/Dodecahedron
    TEST_CLASS(utility_mesh_definition_dodecahedron_test)
    {
    public:

        TEST_METHOD(dodecahedron_vertices_test)
        {
            auto dodecahedron_definition = MeshDefinitonDodecahedron<>();
            auto dodecahedron_mesh_data = dodecahedron_definition.generate_mesh_data();

            auto [no_of_values, vertex_array] = dodecahedron_mesh_data->get_vertex_attributes();
            auto specification = dodecahedron_mesh_data->get_specification();
            auto attribute_size = dodecahedron_mesh_data->get_attribute_size();

            Assert::AreEqual(static_cast<size_t>(0), no_of_values % attribute_size);
            Assert::AreEqual(static_cast<size_t>(0), no_of_values % 12);
        }

        TEST_METHOD(dodecahdron_indices_test)
        {
            auto dodecahedron_definition = MeshDefinitonDodecahedron<>();
            auto dodecahedron_mesh_data = dodecahedron_definition.generate_mesh_data();

            auto [no_of_indices, index_array] = dodecahedron_mesh_data->get_indices();

            Assert::AreEqual(static_cast<size_t>(0), no_of_indices % 12);
        }

        TEST_METHOD(dodecahedron_side_len_test)
        {
            auto dodecahedron_definition = MeshDefinitonDodecahedron<>();
            auto dodecahedron_mesh_data = dodecahedron_definition.generate_mesh_data();

            auto [no_of_values, vertex_array] = dodecahedron_mesh_data->get_vertex_attributes();
            auto [no_of_indices, index_array] = dodecahedron_mesh_data->get_indices();
            auto specification = dodecahedron_mesh_data->get_specification();
            auto attribute_size = dodecahedron_mesh_data->get_attribute_size();

            float expected_length_1 = 0.649839f;
            float expected_length_2 = 0.7639319f;
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