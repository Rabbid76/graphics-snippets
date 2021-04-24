#include <pch.h>

#include <gl/opengl_include.h>
#include <gl/opengl_mesh_single.h>

namespace OpenGL::mesh
{
    SingleMesh::SingleMesh(const ::mesh::MeshDataInterface<float, unsigned int>& definition)
    {
        auto [no_of_values, vertex_array] = definition.get_vertex_attributes();
        auto [no_of_indices, index_array] = definition.get_indices();
        auto specification = definition.get_specification();
        auto attribute_size = definition.get_attribute_size();

        _no_of_vertices = no_of_values / attribute_size;
        _no_of_indices = index_array != nullptr ? no_of_indices : 0;

        glCreateVertexArrays(1, &_vertex_array_object);
        glBindVertexArray(_vertex_array_object);

        GLuint buffer_objects[2];
        glGenBuffers(2, buffer_objects);

        if (_no_of_indices > 0)
        {
            _index_buffer_object = buffer_objects[0];
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _index_buffer_object);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, no_of_indices * sizeof(GLuint), index_array, GL_STATIC_DRAW);
        }

        _vertex_buffer_object = buffer_objects[1];
        glBindBuffer(GL_ARRAY_BUFFER, _vertex_buffer_object);
        glBufferData(GL_ARRAY_BUFFER, no_of_values * sizeof(GLfloat), vertex_array, GL_STATIC_DRAW);

        size_t offset = 0;
        for (const auto& [attribute_type, size] : specification)
        {
            auto attribute_index = static_cast<GLuint>(attribute_type);
            glEnableVertexAttribArray(attribute_index);
            glVertexAttribPointer(attribute_index, size, GL_FLOAT, GL_FALSE,
                attribute_size * sizeof(GLfloat), reinterpret_cast<const void*>(offset * sizeof(GLfloat)));
            offset += size;
        }
    }

    void SingleMesh::destroy(void)
    {
        GLuint buffers[] = { _vertex_buffer_object, _index_buffer_object };
        glDeleteBuffers(2, buffers);
        glDeleteVertexArrays(1, &_vertex_array_object);
    }

    void SingleMesh::draw(void) const
    {
        glBindVertexArray(_vertex_array_object);
        if (_no_of_indices > 0)
            glDrawElements(GL_TRIANGLES, _no_of_indices, GL_UNSIGNED_INT, nullptr);
        else
            glDrawArrays(GL_TRIANGLES, 0, _no_of_vertices);
    }
}