#include <pch.h>

#include <gl/opengl_include.h>
#include <gl/opengl_mesh_single_separate_attribute.h>

namespace OpenGL::mesh
{
    SingleMeshSeparateAttributeFormat::SingleMeshSeparateAttributeFormat(const ::mesh::MeshDataInterface<float, unsigned int>& definition)
    {
        auto [no_of_values, vertex_array] = definition.get_vertex_attributes();
        auto [no_of_indices, index_array] = definition.get_indices();
        auto specification = definition.get_specification();
        auto attribute_size = definition.get_attribute_size();

        _no_of_vertices = no_of_values / attribute_size;
        _no_of_indices = index_array != nullptr ? no_of_indices : 0;

        GLuint buffer_objects[2];
        glCreateBuffers(_no_of_indices > 0 ? 2 : 1, buffer_objects);

        _vertex_buffer_object = buffer_objects[0];
        glNamedBufferStorage(_vertex_buffer_object, no_of_values * sizeof(GLfloat), vertex_array, 0);

        if (_no_of_indices > 0)
        {
            _index_buffer_object = buffer_objects[1];
            glNamedBufferStorage(_index_buffer_object, no_of_indices * sizeof(GLuint), index_array, 0);
        }

        glCreateVertexArrays(1, &_vertex_array_object);
        glVertexArrayVertexBuffer(_vertex_array_object, 0, _vertex_buffer_object, 0, attribute_size * sizeof(GLfloat));
        if (_no_of_indices > 0)
            glVertexArrayElementBuffer(_vertex_array_object, _index_buffer_object);

        size_t offset = 0;
        for (const auto& [attribute_type, size] : specification)
        {
            auto attribute_index = static_cast<GLuint>(attribute_type);
            glVertexArrayAttribFormat(_vertex_array_object, attribute_index, size, GL_FLOAT, GL_FALSE, offset * sizeof(GLfloat));
            glVertexArrayAttribBinding(_vertex_array_object, attribute_index, 0);
            glEnableVertexArrayAttrib(_vertex_array_object, attribute_index);
            offset += size;
        }
    }

    void SingleMeshSeparateAttributeFormat::destroy(void)
    {
        GLuint buffers[] = { _vertex_buffer_object, _index_buffer_object };
        glDeleteBuffers(2, buffers);
        glDeleteVertexArrays(1, &_vertex_array_object);
    }

    void SingleMeshSeparateAttributeFormat::draw(void) const
    {
        glBindVertexArray(_vertex_array_object);
        if (_no_of_indices > 0)
            glDrawElements(GL_TRIANGLES, _no_of_indices, GL_UNSIGNED_INT, nullptr);
        else
            glDrawArrays(GL_TRIANGLES, 0, _no_of_vertices);
    }
}