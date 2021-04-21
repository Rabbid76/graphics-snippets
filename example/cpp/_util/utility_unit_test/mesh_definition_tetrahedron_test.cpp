#include "pch.h"
#include "CppUnitTest.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

// [Write unit tests for C/C++ in Visual Studio](https://docs.microsoft.com/en-us/visualstudio/test/writing-unit-tests-for-c-cpp?view=vs-2019)

#include <mesh/mesh_definition_tetrahedron.h>
#include <glm/glm.hpp>

using namespace mesh;

namespace mesh_test
{
    // Tetrahedron https://en.wikipedia.org/wiki/Tetrahedron
    TEST_CLASS(utility_mesh_definition_tetrahedron_test)
    {
    public:

        TEST_METHOD(tetrahedron_vertices_test)
        {
            auto tetrahedron_definition = MeshDefinitonTetrahedron<>();
            auto tetrahedron_mesh_data = tetrahedron_definition.generate_mesh_data();

            auto [no_of_values, vertex_array] = tetrahedron_mesh_data->get_vertex_attributes();
            auto specification = tetrahedron_mesh_data->get_specification();
            auto attribute_size = tetrahedron_mesh_data->get_attribute_size();

            Assert::AreEqual(static_cast<size_t>(0), no_of_values % attribute_size);
            Assert::AreEqual(attribute_size * 12, no_of_values);
        }

        TEST_METHOD(tetrahedron_indices_test)
        {
            auto tetrahedron_definition = MeshDefinitonTetrahedron<>();
            auto tetrahedron_mesh_data = tetrahedron_definition.generate_mesh_data();

            auto [no_of_indices, index_array] = tetrahedron_mesh_data->get_indices();
            
            Assert::AreEqual(static_cast<size_t>(12), no_of_indices);
        }

        TEST_METHOD(tetrahedron_side_len_test)
        {
            auto tetrahedron_definition = MeshDefinitonTetrahedron<float, unsigned int>(1);
            auto tetrahedron_mesh_data = tetrahedron_definition.generate_mesh_data();

            auto [no_of_values, vertex_array] = tetrahedron_mesh_data->get_vertex_attributes();
            auto [no_of_indices, index_array] = tetrahedron_mesh_data->get_indices();
            auto specification = tetrahedron_mesh_data->get_specification();
            auto attribute_size = tetrahedron_mesh_data->get_attribute_size();

            float expected_length = 4.0f / std::sqrt(6.0f);
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
                    Assert::AreEqual(expected_length, length, 0.0001f);
                }
            }
        }
    };
}