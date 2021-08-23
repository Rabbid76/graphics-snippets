from OpenGL.GL import *
from glfw.GLFW import *
import OpenGL.GL.shaders
import numpy as np
from ctypes import c_void_p
import glm
import math
from wavefrontloader import *
from test_scene import *
from glfw_navigate import *
import os
os.chdir(os.path.join(os.path.dirname(os.path.abspath(__file__)), '../../../resource'))

sh_ssao_vert = """
#version 460 core

layout (location = 0) in vec4 a_position;

void main() 
{
    gl_Position = a_position;
}
"""

sh_ssao_frag = """
#version 460 core

layout (binding = 1) uniform sampler2D u_color_texture;
layout (location = 1) uniform vec2 u_resolution; 
out vec4 frag_color;

void main() 
{
    vec2 uv = gl_FragCoord.xy / u_resolution;
    frag_color = texture(u_color_texture, uv).bgra;
}
"""

def create_screenspace_vao():
    attributes = np.array([(-1,-1), (1,-1), (-1,1), (1,1)], dtype=np.float32) 
    vao = glGenVertexArrays(1)
    vbo = glGenBuffers(1)
    glBindVertexArray(vao)
    glBindBuffer(GL_ARRAY_BUFFER, vbo)
    glBufferData(GL_ARRAY_BUFFER, attributes, GL_STATIC_DRAW)
    glVertexAttribPointer(0, 2, GL_FLOAT, False, 2 * attributes.itemsize, None)
    glEnableVertexAttribArray(0)
    glBindVertexArray(0)
    glDeleteBuffers(1, [vbo])
    return vao

def create_frambuffer(width, height, format, internal_format, filter, depth_buffer):
    fbo = glGenFramebuffers(1)
    glBindFramebuffer(GL_FRAMEBUFFER, fbo)

    color_texture = glGenTextures(1)
    glBindTexture(GL_TEXTURE_2D, color_texture)
    glTexImage2D(GL_TEXTURE_2D, 0, internal_format, width, height, 0, format, GL_UNSIGNED_BYTE, None);            
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter)
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, color_texture, 0)

    depth_texture = None
    if depth_buffer:
        depth_texture = glGenTextures(1)
        glBindTexture(GL_TEXTURE_2D, depth_texture)
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, None)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter)       
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depth_texture, 0)
    
    glDrawBuffers(1, [GL_COLOR_ATTACHMENT0])
    status = glCheckFramebufferStatus(GL_FRAMEBUFFER)
    if status != GL_FRAMEBUFFER_COMPLETE:
        print("frambuffer inclomplete")
    glBindFramebuffer(GL_FRAMEBUFFER, 0)
    return fbo, color_texture, depth_texture

scene_fbo, scene_color_texture, scene_depth_texture = None, None, None
def create_frambuffers(vp_size):
    global scene_fbo, scene_color_texture, scene_depth_texture
    delete_textures = []
    delete_buffers = []
    if scene_color_texture: delete_textures.append(scene_color_texture)
    if scene_depth_texture: delete_textures.append(scene_depth_texture)
    if scene_fbo: delete_buffers.append(scene_fbo) 
    glDeleteTextures(len(delete_textures), delete_textures)
    glDeleteBuffers(len(delete_buffers), delete_buffers)
    scene_fbo, scene_color_texture, scene_depth_texture = create_frambuffer(*vp_size, GL_RGBA, GL_RGB8, GL_LINEAR, True)

if glfwInit() == GLFW_FALSE:
    exit()

glfwWindowHint(GLFW_SAMPLES, 8)
window = glfwCreateWindow(800, 600, "OpenGL Window", None, None)
glfwMakeContextCurrent(window)

scene = TestScene('./model/wavefront')
scene.create()
navigate = Navigation(window, glm.vec3(0, -0.5, -3.0), "ssao_simple")
navigate.change_vp_size_callback = create_frambuffers

ssao_program = OpenGL.GL.shaders.compileProgram(
    OpenGL.GL.shaders.compileShader(sh_ssao_vert, GL_VERTEX_SHADER),
    OpenGL.GL.shaders.compileShader(sh_ssao_frag, GL_FRAGMENT_SHADER)
)
screensapce_vao = create_screenspace_vao()
create_frambuffers(navigate.viewport_size)

glEnable(GL_MULTISAMPLE)
glClearColor(0.0, 0.0, 0.0, 0.0)

while not glfwWindowShouldClose(window):
      
    glBindFramebuffer(GL_FRAMEBUFFER, scene_fbo)
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)

    scene.draw(navigate.view_matrix, navigate.projection_matrix)
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0)
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)

    glUseProgram(ssao_program)
    glActiveTexture(GL_TEXTURE1)
    glBindTexture(GL_TEXTURE_2D, scene_color_texture)
    glUniform2fv(1, 1, navigate.viewport_size)
    glBindVertexArray(screensapce_vao)
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4)
    
    glfwSwapBuffers(window)
    glfwPollEvents()
    navigate.handle_post_rehresh_actions()

glfwTerminate()
exit()
   

