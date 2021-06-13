from OpenGL.GLUT import *
from OpenGL.GLU import *
from OpenGL.GL import *
import OpenGL.GL.shaders
import numpy as np
from ctypes import c_void_p
import glm
import math

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

vec3 HUEtoRGB(in float H)
{
    float R = abs(H * 6.0 - 3.0) - 1.0;
    float G = 2.0 - abs(H * 6.0 - 2.0);
    float B = 2.0 - abs(H * 6.0 - 4.0);
    return clamp(vec3(R, G, B), 0.0, 1.0);
}

void main()
{
    vec4  color = vec4(HUEtoRGB(v_uvw.z), 1.0);
    vec3  L = normalize(vec3(1.0, -1.0, 1.0));
    vec3  eye = inverse(u_view)[3].xyz;
    vec3  V = normalize(eye - v_pos);
    float face = sign(dot(v_nv, V));
    vec3  N = normalize(v_nv) * face;
    vec3  H = normalize(V + L);
    float ka = 0.5;
    float kd = max(0.0, dot(N, L)) * 0.5;
    float NdotH = max(0.0, dot(N, H));
    float sh = 100.0;
    float ks = pow(NdotH, sh) * 0.1;
    frag_color = vec4(color.rgb * (ka + kd + ks), color.a);
}
"""

def display():
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)
    
    elapsed_ms = glutGet(GLUT_ELAPSED_TIME)

    projection = glm.perspective(glm.radians(60), aspect, 0.1, 10.0)
    view = glm.lookAt(glm.vec3(0, 0, 4), glm.vec3(0, 0, 0), glm.vec3(0, 1, 0))
    angle = elapsed_ms * math.pi * 2 / 5000.0
    model = glm.rotate(glm.mat4(1), angle * 0.5, glm.vec3(0, 1, 0))
    model = glm.rotate(model, angle, glm.vec3(1, 0, 0))
    
    glUniformMatrix4fv(0, 1, GL_FALSE, glm.value_ptr(projection))
    glUniformMatrix4fv(1, 1, GL_FALSE, glm.value_ptr(view))
    glUniformMatrix4fv(2, 1, GL_FALSE, glm.value_ptr(model))

    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, None)

    glutSwapBuffers()
    glutPostRedisplay()

def reshape(width, height):
    global aspect
    glViewport(0, 0, width, height)
    aspect = width / height    

glutInit(sys.argv)
glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH | GLUT_MULTISAMPLE)
glutSetOption(GLUT_MULTISAMPLE, 8)
glutInitWindowSize(640, 480)
glutCreateWindow(b"OpenGL Window")
glutDisplayFunc(display)
glutReshapeFunc(reshape)

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

vao = glGenVertexArrays(1)
vbo = glGenBuffers(1)
ebo = glGenBuffers(1)
glBindVertexArray(vao)
glBindBuffer(GL_ARRAY_BUFFER, vbo)
glBufferData(GL_ARRAY_BUFFER, attributes, GL_STATIC_DRAW)
glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo)
glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices, GL_STATIC_DRAW)
glVertexAttribPointer(0, 3, GL_FLOAT, False, 9 * attributes.itemsize, None)
glVertexAttribPointer(1, 3, GL_FLOAT, False, 9 * attributes.itemsize, c_void_p(3 * attributes.itemsize))
glVertexAttribPointer(2, 3, GL_FLOAT, False, 9 * attributes.itemsize, c_void_p(6 * attributes.itemsize))
glEnableVertexAttribArray(0)
glEnableVertexAttribArray(1)
glEnableVertexAttribArray(2)

program = OpenGL.GL.shaders.compileProgram(
    OpenGL.GL.shaders.compileShader(sh_vert, GL_VERTEX_SHADER),
    OpenGL.GL.shaders.compileShader(sh_frag, GL_FRAGMENT_SHADER)
)
glUseProgram(program)

glEnable(GL_MULTISAMPLE) # default
glEnable(GL_DEPTH_TEST)

glClearColor(0.0, 0.0, 0.0, 0.0)
glutMainLoop()