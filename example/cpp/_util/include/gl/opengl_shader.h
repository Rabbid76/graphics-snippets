#ifndef __OPENGL_SHDER__H__
#define __OPENGL_SHDER__H__

#include "opengl_include.h"
#include <initializer_list>

namespace OpenGL
{
    bool CompileStatus(GLuint shader)
    {
        GLint status = GL_TRUE;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
        if (status == GL_FALSE)
        {
            GLint logLen;
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLen);
            std::vector< char >log(logLen);
            GLsizei written;
            glGetShaderInfoLog(shader, logLen, &written, log.data());
            std::cout << "compile error:" << std::endl << log.data() << std::endl;
        }
        return status != GL_FALSE;
    }

    bool LinkStatus(GLuint program)
    {
        GLint status = GL_TRUE;
        glGetProgramiv(program, GL_LINK_STATUS, &status);
        if (status == GL_FALSE)
        {
            GLint logLen;
            glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLen);
            std::vector< char >log(logLen);
            GLsizei written;
            glGetProgramInfoLog(program, logLen, &written, log.data());
            std::cout << "link error:" << std::endl << log.data() << std::endl;
        }
        return status != GL_FALSE;
    }

    GLuint CompileShader(GLenum ahader_type, const char* shader_code)
    {
        auto shader_obj = glCreateShader(ahader_type);
        glShaderSource(shader_obj, 1, &shader_code, nullptr);
        glCompileShader(shader_obj);
        CompileStatus(shader_obj);
        return shader_obj;
    }

    GLuint LinkProgram(std::initializer_list<GLuint> shader_objets)
    {
        auto program_obj = glCreateProgram();
        for (auto shader_object: shader_objets)
        {
            glAttachShader(program_obj, shader_object);
        }
        glLinkProgram(program_obj);
        LinkStatus(program_obj);
        return program_obj;
    }
}

#endif