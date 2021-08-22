from OpenGL.GL import *
from glfw.GLFW import *
import OpenGL.GL.shaders
import numpy as np
from ctypes import c_void_p
import glm
import math
import os
os.chdir(os.path.join(os.path.dirname(os.path.abspath(__file__)), '../../../resource'))

sh_vert = """
#version 460 core

layout (location = 0) in vec4 a_position;
layout (location = 1) in vec3 a_normal;
layout (location = 2) in vec3 a_uvw;

out vec3 v_pos;
out vec3 v_nv;
out vec3 v_uvw;

layout (location = 0) uniform mat4 u_projection;
layout (location = 1) uniform mat4 u_view;
layout (location = 2) uniform mat4 u_model;

void main() 
{
    v_pos = vec3(u_model * a_position);
    v_nv = inverse(transpose(mat3(u_model))) * a_normal;
    v_uvw = a_uvw;
    gl_Position = u_projection * u_view * u_model * a_position;
}
"""

sh_frag = """
#version 460 core

out vec4 frag_color;

in vec3 v_pos;
in vec3 v_nv;
in vec3 v_uvw;

layout (location = 1) uniform mat4 u_view;
layout (location = 3) uniform vec4 u_k_ads;

vec3 HUEtoRGB(in float H)
{
    float R = abs(H * 6.0 - 3.0) - 1.0;
    float G = 2.0 - abs(H * 6.0 - 2.0);
    float B = 2.0 - abs(H * 6.0 - 4.0);
    return clamp(vec3(R, G, B), 0.0, 1.0);
}

vec3 HSLtoRGB(in vec3 HSL)
{
    if (HSL.y <= 0.0001)
        return vec3(HSL.z);
    vec3 RGB = HUEtoRGB(HSL.x);
    float C = (1.0 - abs(2.0 * HSL.z - 1.0)) * HSL.y;
    return (RGB - 0.5) * C + HSL.z;
}

vec3 light_model(float hue)
{
    vec4  color = vec4(HUEtoRGB(hue), 1.0);
    vec3  L = normalize(vec3(1.0, -1.0, 1.0));
    vec3  eye = inverse(u_view)[3].xyz;
    vec3  V = normalize(eye - v_pos);
    float face = sign(dot(v_nv, V));
    vec3  N = normalize(v_nv) * face;
    vec3  H = normalize(V + L);
    float kd = max(0.0, dot(N, L)) * u_k_ads[1];
    float NdotH = max(0.0, dot(N, H));
    float ks = pow(NdotH, max(1.0, u_k_ads[3])) * u_k_ads[2];
    return color.rgb * (u_k_ads[0] + kd + ks);
}

void main()
{
    vec3 color = u_k_ads.x < 0.0 ? HSLtoRGB(v_uvw.zxy) : light_model(v_uvw.z);
    frag_color = vec4(color.rgb, 1.0);
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
    glVertexAttribPointer(0, 3, GL_FLOAT, False, 9 * attributes.itemsize, None)
    glVertexAttribPointer(1, 3, GL_FLOAT, False, 9 * attributes.itemsize, c_void_p(3 * attributes.itemsize))
    glVertexAttribPointer(2, 3, GL_FLOAT, False, 9 * attributes.itemsize, c_void_p(6 * attributes.itemsize))
    glEnableVertexAttribArray(0)
    glEnableVertexAttribArray(1)
    glEnableVertexAttribArray(2)
    glBindVertexArray(0)
    glDeleteBuffers(2, buffer_objects)
    return vao, indices.size

def create_cube_mesh():
    vertices = [(-1,-1,-1), (1,-1,-1), (1, 1,-1), (-1, 1,-1), (-1,-1, 1), (1,-1, 1), (1, 1, 1), (-1, 1, 1)]
    uv = [(0,0), (1,0), (1, 1), (0,1)]
    faces = [[0,1,2,3], [1,5,6,2], [5,4,7,6], [4,0,3,7], [3,2,6,7], [1,0,4,5]]
    normals = [(0,0,-1), (1,0,0), (0,0,1), (-1,0,0), (0,1,0), (0,-1,0)]

    attributes = []
    indices = []
    for si, f in enumerate(faces):
        for qi, i in enumerate(f):
            attributes.append(list(vertices[i]) + list(normals[si]) + [*uv[qi], si/len(faces)])
        indices.append([4*si, 4*si+1, 4*si+2, 4*si, 4*si+2, 4*si+3])
    attributes = np.array(attributes, dtype=np.float32) 
    indices = np.array(indices, dtype=np.uint32)
    return attributes, indices

def create_background_mesh():
    attributes = np.array([(-1,-1,0, 0,0,0, 0.5,0.1,120/360), (1,-1,0, 0,0,0, 0.5,0.1,120/360), (1,1,0, 0,0,0, 1.0,0.2,240/360), (-1,1,0, 0,0,0, 1.0,0.2,240/360)], dtype=np.float32) 
    indices = np.array([0, 1, 2, 0, 2, 3], dtype=np.uint32)
    return attributes, indices

if glfwInit() == GLFW_FALSE:
    exit()

glfwWindowHint(GLFW_SAMPLES, 8)
window = glfwCreateWindow(400, 300, "OpenGL Window", None, None)
glfwMakeContextCurrent(window)

program = OpenGL.GL.shaders.compileProgram(
    OpenGL.GL.shaders.compileShader(sh_vert, GL_VERTEX_SHADER),
    OpenGL.GL.shaders.compileShader(sh_frag, GL_FRAGMENT_SHADER)
)
glUseProgram(program)

background_vao = create_vao(*create_background_mesh())
cube_vao = create_vao(*create_cube_mesh())

glEnable(GL_MULTISAMPLE) # default

glClearColor(0.0, 0.0, 0.0, 0.0)

start_time_s = glfwGetTime()
while not glfwWindowShouldClose(window):
    current_time_s = glfwGetTime()
    delta_time_s = current_time_s - start_time_s

    vp_size = glfwGetFramebufferSize(window)
    glViewport(0, 0, *vp_size)
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)
    
    identity = glm.mat4(1)
    projection = glm.perspective(glm.radians(60), vp_size[0]/vp_size[1], 0.1, 10.0)
    view = glm.lookAt(glm.vec3(0, 0, 4), glm.vec3(0, 0, 0), glm.vec3(0, 1, 0))
    angle = delta_time_s * math.pi * 2 / 5
    model = glm.rotate(glm.mat4(1), angle * 0.5, glm.vec3(0, 1, 0))
    model = glm.rotate(model, angle, glm.vec3(1, 0, 0))

    glUniformMatrix4fv(0, 1, GL_FALSE, glm.value_ptr(identity))
    glUniformMatrix4fv(1, 1, GL_FALSE, glm.value_ptr(identity))
    glUniformMatrix4fv(2, 1, GL_FALSE, glm.value_ptr(identity))
    glUniform4fv(3, 1, [-1, 0, 0, 0])

    glDisable(GL_DEPTH_TEST)
    glBindVertexArray(background_vao[0])
    glDrawElements(GL_TRIANGLES, background_vao[1], GL_UNSIGNED_INT, None)
    
    glUniformMatrix4fv(0, 1, GL_FALSE, glm.value_ptr(projection))
    glUniformMatrix4fv(1, 1, GL_FALSE, glm.value_ptr(view))
    glUniformMatrix4fv(2, 1, GL_FALSE, glm.value_ptr(model))
    glUniform4fv(3, 1, [0.5, 0.5, 0.1, 100])

    glEnable(GL_DEPTH_TEST)
    glBindVertexArray(cube_vao[0])
    glDrawElements(GL_TRIANGLES, cube_vao[1], GL_UNSIGNED_INT, None)

    glfwSwapBuffers(window)
    glfwPollEvents()

glfwTerminate()
exit()
   

