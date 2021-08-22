from OpenGL.GL import *
from glfw.GLFW import *
import OpenGL.GL.shaders
import numpy as np
from ctypes import c_void_p
import glm
import math
from wavefrontloader import *

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
layout (location = 3) uniform vec4 u_k_ads = vec4(0.5, 0.5, 0.1, 100.0);
layout (location = 4) uniform vec4 u_light_vec = vec4(0.0, 0.0, 1.0, 0.0);

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
    vec4  color = vec4(hue < 0.0 ? vec3(abs(hue)) : HUEtoRGB(hue), 1.0);
    vec3  L = normalize(u_light_vec.xyz);
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

def create_quad_mesh():
    attributes = np.array([(-1,-1,0, 0,0,1, 0,0,-0.5), (1,-1,0, 0,0,1, 0,0,-0.5), (1,1,0, 0,0,1, 0,0,-0.5), (-1,1,0, 0,0,1, 0,0,-0.5)], dtype=np.float32) 
    indices = np.array([0, 1, 2, 0, 2, 3], dtype=np.uint32)
    return attributes, indices

def create_cylinder_mesh():
    attributes = []
    indices = []
    for i in range(361):
        s = math.sin(glm.radians(i))
        c = math.cos(glm.radians(i))
        nv = glm.normalize(glm.vec3(c, s, 0))
        attributes.append([c, s, -1, *nv, i/360, -1, -0.5])
        attributes.append([c, s, 1, *nv, i/360, 1, -0.5])
        indices.append([i*2, i*2+1])
    attributes = np.array([attributes], dtype=np.float32) 
    indices = np.array(indices, dtype=np.uint32)
    return attributes, indices

def model_mat_ground_center(model_center, model_size):
    model = glm.scale(glm.mat4(1), glm.vec3(1/max(model_size)))
    return glm.translate(model, glm.vec3(0, model_size[1]/2, 0) - glm.vec3(model_center))

class TestScene:
    def __init__(self, model_filepath):
        self.model_filepath = model_filepath

    def create(self):
        self.program = OpenGL.GL.shaders.compileProgram(
            OpenGL.GL.shaders.compileShader(sh_vert, GL_VERTEX_SHADER),
            OpenGL.GL.shaders.compileShader(sh_frag, GL_FRAGMENT_SHADER)
        )

        self.background_vao = create_vao(*create_background_mesh())
        self.rect_vao = create_vao(*create_quad_mesh())
        self.cylinder_vao = create_vao(*create_cylinder_mesh())

        mesh1 = WavefrontMesh(os.path.join(self.model_filepath, 'buddha.obj'))
        mesh2 = WavefrontMesh(os.path.join(self.model_filepath, 'dragon.obj'))
        self.mesh1_vao = create_vao(mesh1.attributes, mesh1.indices)
        self.model1 = glm.translate(glm.mat4(1), glm.vec3(-0.35, 0, -0.2)) * model_mat_ground_center(mesh1.center, mesh1.size)
        self.mesh2_vao = create_vao(mesh2.attributes, mesh2.indices)
        self.model2 = glm.rotate(glm.translate(glm.mat4(1), glm.vec3(0.35, 0, 0)), math.pi/2, glm.vec3(0, 1, 0)) * model_mat_ground_center(mesh2.center, mesh2.size)

    def draw(self, view, projection):
        glUseProgram(self.program)

        identity = glm.mat4(1)
        wall_back_dist = 0.6
        wall_side_dist = 0.65
        wall_height = 1.2
        pillar_height = 1.2

        glUniformMatrix4fv(0, 1, GL_FALSE, glm.value_ptr(identity))
        glUniformMatrix4fv(1, 1, GL_FALSE, glm.value_ptr(identity))
        glUniformMatrix4fv(2, 1, GL_FALSE, glm.value_ptr(identity))
        glUniform4fv(3, 1, [-1, 0, 0, 0])

        glDisable(GL_DEPTH_TEST)
        glBindVertexArray(self.background_vao[0])
        glDrawElements(GL_TRIANGLES, self.background_vao[1], GL_UNSIGNED_INT, None)
        
        glUniformMatrix4fv(0, 1, GL_FALSE, glm.value_ptr(projection))
        glUniformMatrix4fv(1, 1, GL_FALSE, glm.value_ptr(view))
        glUniform4fv(3, 1, [0.5, 0.5, 0.1, 100])
        glUniform4fv(4, 1, [0.0, 0.5, 1.0, 0.0])

        glEnable(GL_DEPTH_TEST)

        glBindVertexArray(self.rect_vao[0])
        ground_model = glm.scale(glm.rotate(glm.mat4(1), -math.pi/2, glm.vec3(1, 0, 0)), glm.vec3(1, 1, 1))
        glUniformMatrix4fv(2, 1, GL_FALSE, glm.value_ptr(ground_model))
        glDrawElements(GL_TRIANGLES, self.rect_vao[1], GL_UNSIGNED_INT, None)
        backwall_model = glm.scale(glm.translate(glm.mat4(1), glm.vec3(0, wall_height/2, -wall_back_dist)), glm.vec3(wall_side_dist, wall_height/2, 1))
        glUniformMatrix4fv(2, 1, GL_FALSE, glm.value_ptr(backwall_model))
        glDrawElements(GL_TRIANGLES, self.rect_vao[1], GL_UNSIGNED_INT, None)

        glBindVertexArray(self.cylinder_vao[0])
        pillar_model = glm.scale(glm.translate(glm.rotate(glm.mat4(1), -math.pi/2, glm.vec3(1, 0, 0)), glm.vec3(0, 0, pillar_height/2)), glm.vec3(0.05, 0.05, pillar_height/2))
        for x in [-1, 1]:
            for y in [-1, -1/3, 1/3, 1]:
                pos_mat = glm.translate(glm.mat4(1), glm.vec3(wall_side_dist * x, 0, wall_back_dist * y))
                glUniformMatrix4fv(2, 1, GL_FALSE, glm.value_ptr(pos_mat * pillar_model))
                glDrawElements(GL_TRIANGLE_STRIP, self.cylinder_vao[1], GL_UNSIGNED_INT, None)
        
        glUniformMatrix4fv(2, 1, GL_FALSE, glm.value_ptr(self.model1))
        glBindVertexArray(self.mesh1_vao[0])
        glDrawElements(GL_TRIANGLES, self.mesh1_vao[1], GL_UNSIGNED_INT, None)

        glUniformMatrix4fv(2, 1, GL_FALSE, glm.value_ptr(self.model2))
        glBindVertexArray(self.mesh2_vao[0])
        glDrawElements(GL_TRIANGLES, self.mesh2_vao[1], GL_UNSIGNED_INT, None)