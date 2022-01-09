import os
os.chdir(os.path.dirname(os.path.abspath(__file__)))
import setup
setup.build(['build_ext', '--inplace'])

from OpenGL.GL import *
from glfw.GLFW import *
import OpenGL.GL.shaders
import numpy as np
from ctypes import c_void_p
import glm
import math
import os
os.chdir(os.path.join(os.path.dirname(os.path.abspath(__file__)), '../../../resource'))

import mesh_generator

sh_vert = """
#version 460 core

layout (location = 0) in vec4 a_position;

out vec3 v_pos;

layout (location = 0) uniform mat4 u_projection;
layout (location = 1) uniform mat4 u_view;
layout (location = 2) uniform mat4 u_model;

void main() 
{
    v_pos = vec3(u_model * a_position);
    gl_Position = u_projection * u_view * u_model * a_position;
}
"""

sh_frag = """
#version 460 core

out vec4 frag_color;
in vec3 v_pos;
layout (location = 3) uniform vec4 u_color;

void main()
{
     frag_color = u_color;
}
"""

def create_vao(attributes, indices):
    vao = glGenVertexArrays(1)
    buffer_objects = glGenBuffers(2)
    glBindVertexArray(vao)
    glBindBuffer(GL_ARRAY_BUFFER, buffer_objects[0])
    glBufferData(GL_ARRAY_BUFFER, attributes, GL_STATIC_DRAW)
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer_objects[1])
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices, GL_STATIC_DRAW)
    glVertexAttribPointer(0, 3, GL_FLOAT, False, 3 * 4, None)
    glEnableVertexAttribArray(0)
    glBindVertexArray(0)
    return vao

if glfwInit() == GLFW_FALSE:
    exit()

glfwWindowHint(GLFW_SAMPLES, 8)
window = glfwCreateWindow(800, 600, "OpenGL Window", None, None)
glfwMakeContextCurrent(window)

program = OpenGL.GL.shaders.compileProgram(
    OpenGL.GL.shaders.compileShader(sh_vert, GL_VERTEX_SHADER),
    OpenGL.GL.shaders.compileShader(sh_frag, GL_FRAGMENT_SHADER)
)
glUseProgram(program)

no_vertices, vertices, no_inidces, indices = mesh_generator.generate_spiral(1.5, 0.25, 0.03, 0.1, 32, 5)
ctypes_vertices = (ctypes.c_float * no_vertices).from_buffer_copy(vertices)
ctypes_indices = (ctypes.c_uint32 * no_inidces).from_buffer_copy(indices)
print(f"spiral with {no_vertices // 3} vertices and {no_inidces} indices")
cube_vao = create_vao(ctypes_vertices, ctypes_indices), no_inidces

glEnable(GL_MULTISAMPLE) # default
glEnable(GL_DEPTH_TEST)
glPolygonMode(GL_FRONT_AND_BACK, GL_LINE)
glClearColor(0, 0, 0, 0)

start_time_s = glfwGetTime()
while not glfwWindowShouldClose(window):
    current_time_s = glfwGetTime()
    delta_time_s = current_time_s - start_time_s

    vp_size = glfwGetFramebufferSize(window)
    glViewport(0, 0, *vp_size)
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)
    
    identity = glm.mat4(1)
    projection = glm.perspective(glm.radians(25), vp_size[0]/vp_size[1], 0.1, 10.0)
    view = glm.lookAt(glm.vec3(0, 0, 1), glm.vec3(0, 0, 0), glm.vec3(0, 1, 0))
    angle = delta_time_s * math.pi * 2 / 5
    model = glm.rotate(glm.mat4(1), angle * 0.5, glm.vec3(0, 1, 0))
    model = glm.rotate(model, angle, glm.vec3(1, 0, 0))
    
    glUniformMatrix4fv(0, 1, GL_FALSE, glm.value_ptr(projection))
    glUniformMatrix4fv(1, 1, GL_FALSE, glm.value_ptr(view))
    glUniformMatrix4fv(2, 1, GL_FALSE, glm.value_ptr(model))
    glUniform4fv(3, 1, [1, 1, 1, 1])

    glBindVertexArray(cube_vao[0])
    glDrawElements(GL_TRIANGLE_STRIP, cube_vao[1], GL_UNSIGNED_INT, None)

    glfwSwapBuffers(window)
    glfwPollEvents()

glfwTerminate()
exit()
   

