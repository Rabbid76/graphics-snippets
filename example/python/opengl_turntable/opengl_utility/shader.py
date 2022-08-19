from OpenGL.GL import *

def compile_shader(sh_vert, sh_frag):
    program_obj = glCreateProgram()
    for sh_type, sh_code in [(GL_VERTEX_SHADER, sh_vert), (GL_FRAGMENT_SHADER, sh_frag)]:
        shader_obj = glCreateShader(sh_type)
        glShaderSource(shader_obj, sh_code)
        glCompileShader(shader_obj)
        if not glGetShaderiv(shader_obj, GL_COMPILE_STATUS):
            raise Exception(glGetShaderInfoLog(shader_obj).replace(b'\\n', b'\n'))
        glAttachShader(program_obj, shader_obj)
    glLinkProgram(program_obj)
    if not glGetProgramiv(program_obj, GL_LINK_STATUS):
        raise Exception(glGetProgramInfoLog(program_obj).replace(b'\\n', b'\n'))
    return program_obj