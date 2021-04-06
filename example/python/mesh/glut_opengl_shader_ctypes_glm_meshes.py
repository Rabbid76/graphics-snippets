import os, sys 
sys.path.append(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..'))

import math 
import ctypes 
import glm 
from OpenGL.GLUT import *
from OpenGL.GL import *
from OpenGL.GL.shaders import *
from OpenGL.arrays import *
from utility.glut_utility import GlutNavigation
from utility import triangulated_mesh

phong_vert = """
#version 460 core

layout (location = 0) in vec3 a_pos;
layout (location = 1) in vec3 a_nv;
layout (location = 2) in vec3 a_uvw;

out vec3 v_pos;
out vec3 v_nv;
out vec3 v_uvw;

uniform mat4 u_proj; 
uniform mat4 u_view; 

layout(std430, binding = 1) buffer Draw
{
    mat4 model[];
} draw;

void main()
{
    mat4 model  = draw.model[gl_DrawID];
    mat3 normal = transpose(inverse(mat3(model)));
    
    vec4 world_pos = model * vec4(a_pos.xyz, 1.0);

    v_pos       = world_pos.xyz;
    v_nv        = normal * a_nv; 
    v_uvw       = a_uvw; 
    gl_Position = u_proj * u_view * world_pos;
}
"""

phong_frag = """
#version 460 core

out vec4 frag_color;
in  vec3 v_pos;
in  vec3 v_nv;
in  vec3 v_uvw;
uniform mat4 u_view; 

vec3 HUEtoRGB(in float H)
{
    float R = abs(H * 6.0 - 3.0) - 1.0;
    float G = 2.0 - abs(H * 6.0 - 2.0);
    float B = 2.0 - abs(H * 6.0 - 4.0);
    return clamp( vec3(R,G,B), 0.0, 1.0 );
}

void main()
{
    vec4  color = vec4(HUEtoRGB(v_uvw.z), 1.0);
    vec3  L     = normalize(vec3(1.0, -1.0, 1.0));
    vec3  N     = normalize(v_nv);
    vec3  eye   = inverse(u_view)[3].xyz;
    vec3  V     = normalize(eye - v_pos);
    vec3  H     = normalize(V + L);
    float ka    = 0.1;
    float kd    = max(0.0, dot(N, L)) * 0.9;
    float NdotH = max(0.0, dot(N, H));
    float sh    = 100.0;
    float ks    = pow(NdotH, sh) * 0.1;
    frag_color  = vec4(color.rgb * (ka + kd + ks), color.a);
}
"""        
    
class SingleMesh:
    def __init__(self, mesh_specification):
        attr_array = mesh_specification.attributes
        index_array = mesh_specification.indices
        stride, format = mesh_specification.format

        self.__no_indices = len(index_array)
        vertex_attributes = (ctypes.c_float * len(attr_array))(*attr_array)
        indices = (ctypes.c_uint32 * self.__no_indices)(*index_array)
        
        self.__vao = glGenVertexArrays(1)
        self.__vbo, self.__ibo = glGenBuffers(2)
        
        glBindVertexArray(self.__vao)

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, self.__ibo)
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices, GL_STATIC_DRAW)

        glBindBuffer(GL_ARRAY_BUFFER, self.__vbo)
        glBufferData(GL_ARRAY_BUFFER, vertex_attributes, GL_STATIC_DRAW)

        offset = 0
        float_size = ctypes.sizeof(ctypes.c_float)
        for i, attribute_format in enumerate(format):   
            tuple_size = attribute_format[1]
            glVertexAttribPointer(i, tuple_size, GL_FLOAT, False, stride*float_size, c_void_p(offset))
            offset += tuple_size * float_size
            glEnableVertexAttribArray(i)

    def draw(self):
        glBindVertexArray(self.__vao)
        glDrawElements(GL_TRIANGLES, self.__no_indices, GL_UNSIGNED_INT, None)

class MultiMesh:
    def __init__(self, mesh_specifications, stride, format):
        
        attributes = [mesh.attributes for mesh in mesh_specifications]
        indices = [mesh.indices for mesh in mesh_specifications]
        attributes_len = sum(len(a) for a in attributes)
        indices_len = sum(len(i) for i in indices)

        self.__no_of_meshes = len(mesh_specifications)
        draw_indirect_list = []
        first_index = 0
        base_vertex = 0
        for attr_list, index_list in zip(attributes, indices):
            no_of_indices = len(index_list)
            no_of_attributes = len(attr_list) // stride
            draw_indirect_list += [no_of_indices, 1, first_index, base_vertex, 0]
            first_index += no_of_indices
            base_vertex += no_of_attributes

        self.__vao = glGenVertexArrays(1)
        self.__vbo, self.__ibo, self.__dbo = glGenBuffers(3)

        draw_indirect_array = (ctypes.c_uint32 * len(draw_indirect_list))(*draw_indirect_list)
        glBindBuffer(GL_DRAW_INDIRECT_BUFFER, self.__dbo)
        glBufferData(GL_DRAW_INDIRECT_BUFFER, draw_indirect_array, GL_STATIC_DRAW)

        glBindBuffer(GL_ARRAY_BUFFER, self.__vbo)
        glBufferData(GL_ARRAY_BUFFER, attributes_len*4, None, GL_STATIC_DRAW)
        offset = 0
        for attr_list in attributes:
            no_of_values = len(attr_list)
            value_array = (ctypes.c_float * no_of_values)(*attr_list)
            glBufferSubData(GL_ARRAY_BUFFER, offset, no_of_values*4, value_array)
            offset += no_of_values*4

        glBindVertexArray(self.__vao)

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, self.__ibo)
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices_len*4, None, GL_STATIC_DRAW)
        offset = 0
        for index_list in indices:
            no_of_indices = len(index_list)
            index_array = (ctypes.c_uint32 * no_of_indices)(*index_list)
            glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, offset, no_of_indices*4, index_array)
            offset += no_of_indices*4

        offset = 0
        float_size = ctypes.sizeof(ctypes.c_float)
        for i, attribute_format in enumerate(format):   
            tuple_size = attribute_format[1]
            glVertexAttribPointer(i, tuple_size, GL_FLOAT, False, stride*float_size, c_void_p(offset))
            offset += tuple_size * float_size
            glEnableVertexAttribArray(i)

    def draw(self):
        # GLAPI/glDrawElementsIndirect
        # https://www.khronos.org/opengl/wiki/GLAPI/glDrawElementsIndirect
        glBindVertexArray(self.__vao)
        glBindBuffer(GL_DRAW_INDIRECT_BUFFER, self.__dbo)
        glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, None, self.__no_of_meshes, 4*5)
        
class MyWindow:

    __glsl_vert = phong_vert
    __glsl_frag = phong_frag

    def __init__(self, w, h):
        
        self.__caption = 'OpenGL Window'
        
        glutInit()
        glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH)
        glutInitWindowSize(w, h)
        self.__glut_wnd = glutCreateWindow(self.__caption)

        mesh_defs = [
            triangulated_mesh.Tetrahedron(),
            triangulated_mesh.Cube(),
            triangulated_mesh.Octahedron(),
            triangulated_mesh.Dodecahedron(),
            triangulated_mesh.Icosahedron(),
            triangulated_mesh.Tube(),
            triangulated_mesh.Cone(),
            triangulated_mesh.SphereSlice(),
            triangulated_mesh.SphereTessellated(),
            triangulated_mesh.Torus(),
            triangulated_mesh.TrefoilKnot(),
            triangulated_mesh.TorusKnot(),
            triangulated_mesh.Arrow()]

        distance = 2.5
        self.__diameter = distance * len(mesh_defs) / math.pi

        self.__glut_navigation = GlutNavigation(
            w, h,
            glm.lookAt(glm.vec3(0,-self.__diameter,0), glm.vec3(0, 0, 0), glm.vec3(0,0,1)),
            90, 0.1, 100,
            lambda x, y : glReadPixels(x, y, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT))

        self.__program = compileProgram( 
            compileShader( self.__glsl_vert, GL_VERTEX_SHADER ),
            compileShader( self.__glsl_frag, GL_FRAGMENT_SHADER ),
        )
        self.___attrib = { a : glGetAttribLocation (self.__program, a) for a in ['a_pos', 'a_nv', 'a_col'] }
        print(self.___attrib)
        self.___uniform = { u : glGetUniformLocation (self.__program, u) for u in ['u_model', 'u_view', 'u_proj'] }
        print(self.___uniform)

        self.__ssbo = glGenBuffers(1)
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, self.__ssbo)
        glBufferData(GL_SHADER_STORAGE_BUFFER, 4*16*len(mesh_defs), None, GL_STATIC_DRAW)
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, self.__ssbo)

        self.__meshes = [SingleMesh(definition) for definition in mesh_defs] 
        self.__multimesh = MultiMesh(mesh_defs, *mesh_defs[0].format)

        glEnable(GL_DEPTH_TEST)
        glUseProgram(self.__program)
        #glPolygonMode(GL_FRONT_AND_BACK, GL_LINE)

        glutDisplayFunc(self.__mainloop)

    def run(self):
        self.__starttime = 0
        self.__starttime = self.elapsed_ms()
        glutMainLoop()

    def elapsed_ms(self):
      return glutGet(GLUT_ELAPSED_TIME) - self.__starttime

    def __mainloop(self):

        vp_valid, vp_size, view, projection = self.__glut_navigation.update()
        if not vp_valid:
            glViewport(0, 0, *vp_size)

        glUniformMatrix4fv(self.___uniform['u_proj'], 1, GL_FALSE, glm.value_ptr(projection))
        glUniformMatrix4fv(self.___uniform['u_view'], 1, GL_FALSE, glm.value_ptr(view))
        
        glClearColor(0.2, 0.3, 0.3, 1.0)
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)

        angle1 = self.elapsed_ms() * math.pi * 2 / 5000.0
        angle2 = self.elapsed_ms() * math.pi * 2 / 7333.0
        model_matrices = []
        for i, mesh in enumerate(self.__meshes):
            angleY = angle1 + math.pi*2 * i / len(self.__meshes)
            #angleY =  math.pi*2 * i / len(self.__meshes)
            model = glm.mat4(1)
            model = glm.rotate(model, angleY, glm.vec3(0, 0, 1))
            model = glm.translate(model, glm.vec3(self.__diameter/2, 0, 0))
            model = glm.rotate(model, angle2, glm.vec3(0, 1, 0))
            model_matrices.append(model)

        multi_mesh = True
        if multi_mesh:
            glBindBuffer(GL_SHADER_STORAGE_BUFFER, self.__ssbo)
            for i, model in enumerate(model_matrices):
                glBufferSubData(GL_SHADER_STORAGE_BUFFER, i*4*16, glm.sizeof(glm.mat4), glm.value_ptr(model)) 
            self.__multimesh.draw()
        
        else:
            for model, mesh in zip(model_matrices, self.__meshes):
                glBindBuffer(GL_SHADER_STORAGE_BUFFER, self.__ssbo)
                glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, glm.sizeof(glm.mat4), glm.value_ptr(model)) 
                mesh.draw()

        glutSwapBuffers()
        glutPostRedisplay()


window = MyWindow(800, 600)
window.run()