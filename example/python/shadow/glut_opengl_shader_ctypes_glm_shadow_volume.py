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

shadow_vert = """
#version 460 core

layout (location = 0) in vec3 a_pos;

uniform mat4 u_proj; 
uniform mat4 u_view; 

layout(std430, binding = 1) buffer Draw
{
    mat4 model[];
} draw;

void main()
{
   gl_Position = u_proj * u_view * draw.model[gl_DrawID] * vec4(a_pos, 1.0);
}
"""

shadow_frag = """
#version 460 core

void main()
{
}
"""


phong_vert = """
#version 460 core

layout (location = 0) in vec3 a_pos;
layout (location = 1) in vec3 a_nv;
layout (location = 2) in vec3 a_uvw;

out vec3 v_pos;
out vec3 v_nv;
out vec3 v_uvw;
out vec4 v_shadow_pos;

uniform mat4 u_proj; 
uniform mat4 u_view; 
uniform mat4 u_shadow_proj; 
uniform mat4 u_shadow_view; 

layout(std430, binding = 1) buffer Draw
{
    mat4 model[];
} draw;

void main()
{
    mat4 model      = draw.model[gl_DrawID];
    mat3 normal     = transpose(inverse(mat3(model)));
    vec4 world_pos  = model * vec4(a_pos.xyz, 1.0);

    v_pos        = world_pos.xyz;
    v_nv         = normal * a_nv; 
    v_uvw        = a_uvw; 
    v_shadow_pos = u_shadow_proj * u_shadow_view * world_pos;
    gl_Position  = u_proj * u_view * world_pos;
}
"""

phong_frag = """
#version 460 core

out vec4 frag_color;
in  vec3 v_pos;
in  vec3 v_nv;
in  vec3 v_uvw;
in  vec4 v_shadow_pos;

uniform mat4 u_view;
uniform vec4 u_light_pos; 
uniform vec4 u_light_dir; 
//layout(binding = 1)  uniform sampler2D u_shadow_depth;
layout(binding = 1)  uniform sampler2DShadow u_shadow_depth; 

vec3 HUEtoRGB(in float H)
{
    float R = abs(H * 6.0 - 3.0) - 1.0;
    float G = 2.0 - abs(H * 6.0 - 2.0);
    float B = 2.0 - abs(H * 6.0 - 4.0);
    return clamp( vec3(R,G,B), 0.0, 1.0 );
}

void main()
{
    vec3 camera_pos = inverse(u_view)[3].xyz;

    float shadow = 0.0;
    if (-v_shadow_pos.w < v_shadow_pos.z && v_shadow_pos.z < v_shadow_pos.w)
    {
        vec3 shadow_pos = v_shadow_pos.xyz / v_shadow_pos.w * 0.5 + 0.5;
        //float shadow_depth = texture(u_shadow_depth, shadow_pos.st).x;
        //if (shadow_depth >= shadow_pos.z)
        //    shadow = 1.0;
        shadow = texture(u_shadow_depth, shadow_pos.xyz);
    }

    vec4  color = vec4(HUEtoRGB(v_uvw.z), 1.0);
    vec3  L     = normalize(u_light_pos.xyz - v_pos);
    vec3  N     = normalize(v_nv);
    vec3  V     = normalize(camera_pos - v_pos);
    vec3  H     = normalize(V + L);
    float ka    = 0.1;
    float cone  = dot(normalize(u_light_dir.xyz), -L);
    float cull  = step(u_light_dir.w, cone) * step(0, dot(N, V)) * shadow;
    float kd    = max(0.0, dot(N, L)) * 0.9 * cull;
    float NdotH = max(0.0, dot(N, H));
    float sh    = 100.0;
    float ks    = pow(NdotH, sh) * 0.1 * cull;
    frag_color  = vec4(color.rgb * (ka + kd + ks), color.a);
}
"""        
    
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
        glBindVertexArray(self.__vao)
        glBindBuffer(GL_DRAW_INDIRECT_BUFFER, self.__dbo)
        glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, None, self.__no_of_meshes, 4*5)
        
class MyWindow:

    __shadow_vert = shadow_vert
    __shadow_frag = shadow_frag
    __phong_vert = phong_vert
    __phong_frag = phong_frag

    def __init__(self, w, h):
        
        self.__caption = 'OpenGL Window'
        
        glutInit()
        glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH)
        glutInitWindowSize(w, h)
        self.__glut_wnd = glutCreateWindow(self.__caption)

        mesh_defs = [
            #triangulated_mesh.Tetrahedron(),
            #triangulated_mesh.Cube(),
            #triangulated_mesh.Icosahedron(),
            #triangulated_mesh.Tube(),
            #triangulated_mesh.Cone(),
            #triangulated_mesh.SphereSlice(),
            #triangulated_mesh.SphereTessellated(),
            #triangulated_mesh.Torus(),
            triangulated_mesh.TrefoilKnot(),
            #triangulated_mesh.TorusKnot(),
            #triangulated_mesh.Arrow()
        ]

        distance = 2.5
        self.__diameter = 0 if len(mesh_defs) <= 1 else distance * len(mesh_defs) / math.pi
        camera_distance = 4 if len(mesh_defs) <= 1 else self.__diameter

        mesh_defs = [triangulated_mesh.Quad(self.__diameter+6)] + mesh_defs
        self.__no_of_meshes = len(mesh_defs)

        self.__glut_navigation = GlutNavigation(
            w, h,
            glm.lookAt(glm.vec3(0,-camera_distance*0.75,camera_distance*0.75), glm.vec3(0, 0, 0), glm.vec3(0,0,1)),
            90, 0.1, 100,
            lambda x, y : glReadPixels(x, y, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT))

        self.__shadow_program = compileProgram( 
            compileShader( self.__shadow_vert, GL_VERTEX_SHADER ),
            compileShader( self.__shadow_frag, GL_FRAGMENT_SHADER ),
        )
        self.__shadow_uniform = { u : glGetUniformLocation(self.__shadow_program, u) for u in ['u_view', 'u_proj'] }
        
        self.__phong_program = compileProgram( 
            compileShader( self.__phong_vert, GL_VERTEX_SHADER ),
            compileShader( self.__phong_frag, GL_FRAGMENT_SHADER ),
        )
        self.__phong_uniform = { u : glGetUniformLocation(self.__phong_program, u) for u in [
            'u_view', 'u_proj', 'u_light_pos', 'u_light_dir', 'u_shadow_proj', 'u_shadow_view'] }
        print(self.__phong_uniform)

        self.__ssbo = glGenBuffers(1)
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, self.__ssbo)
        glBufferData(GL_SHADER_STORAGE_BUFFER, 4*16*len(mesh_defs), None, GL_STATIC_DRAW)
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, self.__ssbo)

        self.__multimesh = MultiMesh(mesh_defs, *mesh_defs[0].format)

        self.__shadow_buffer_size = (1024, 1024)
        self.__shadow_depth_to = glGenTextures(1)
        glBindTexture(GL_TEXTURE_2D, self.__shadow_depth_to)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP)
        glTexParameteri(GL_TEXTURE_2D, GL_DEPTH_STENCIL_TEXTURE_MODE, GL_DEPTH_COMPONENT)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE)
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, *self.__shadow_buffer_size, 0, GL_DEPTH_COMPONENT, GL_FLOAT, None)
        
        self.__shadow_fbo = glGenFramebuffers(1)
        glBindFramebuffer(GL_FRAMEBUFFER, self.__shadow_fbo)
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, self.__shadow_depth_to, 0)
        frameBufferStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER)
        if frameBufferStatus != GL_FRAMEBUFFER_COMPLETE:
            print('frame buffer incomplete: {}'.format(frameBufferStatus) )
            sys.exit()
        glBindFramebuffer(GL_FRAMEBUFFER, 0)

        glEnable(GL_DEPTH_TEST)

        glutDisplayFunc(self.__mainloop)

    def run(self):
        self.__starttime = 0
        self.__starttime = self.elapsed_ms()
        glutMainLoop()

    def elapsed_ms(self):
      return glutGet(GLUT_ELAPSED_TIME) - self.__starttime

    def __mainloop(self):

        vp_valid, vp_size, view, projection = self.__glut_navigation.update()
        
        angle1 = self.elapsed_ms() * math.pi * 2 / 5000.0
        angle2 = self.elapsed_ms() * math.pi * 2 / 7333.0
        model_matrices = []
        for i in range(self.__no_of_meshes):
            if i == 0:
                model = glm.mat4(1)
                model = glm.translate(model, glm.vec3(0, 0, -1))
            else:
                angleY = angle1 + math.pi*2 * (i-1) / (self.__no_of_meshes-1)
                model = glm.mat4(1)
                model = glm.rotate(model, angleY, glm.vec3(0, 0, 0.5))
                model = glm.translate(model, glm.vec3(self.__diameter/2, 0, 0.5))
                model = glm.rotate(model, angle2, glm.vec3(0, 1, 0))
            model_matrices.append(model)

        light_pos = glm.vec3(0, 0, 3)
        light_dir = glm.vec3(0, 0, -1)
        light_cone_angle_degree = 60
        light_cone_cos = math.cos(math.radians(light_cone_angle_degree) / 2)
        shadow_proj = glm.perspective(glm.radians(light_cone_angle_degree + 1), 1, 0.1, 100.0)
        shadow_view = glm.lookAt(light_pos, light_pos+light_dir, glm.vec3(0,1,0))

        glBindBuffer(GL_SHADER_STORAGE_BUFFER, self.__ssbo)
        for i, model in enumerate(model_matrices):
            glBufferSubData(GL_SHADER_STORAGE_BUFFER, i*4*16, glm.sizeof(glm.mat4), glm.value_ptr(model)) 

        glBindFramebuffer(GL_FRAMEBUFFER, self.__shadow_fbo)
        glViewport(0, 0, *self.__shadow_buffer_size)
        glClear(GL_DEPTH_BUFFER_BIT)

        glUseProgram(self.__shadow_program)
        glUniformMatrix4fv(self.__shadow_uniform['u_proj'], 1, GL_FALSE, glm.value_ptr(shadow_proj))
        glUniformMatrix4fv(self.__shadow_uniform['u_view'], 1, GL_FALSE, glm.value_ptr(shadow_view))

        glEnable(GL_POLYGON_OFFSET_FILL)
        glPolygonOffset(1.0, 1.0)        
        self.__multimesh.draw()
        glDisable(GL_POLYGON_OFFSET_FILL)

        glBindFramebuffer(GL_FRAMEBUFFER, 0)
        glViewport(0, 0, *vp_size)
        glClearColor(0.2, 0.3, 0.3, 1.0)
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)
        
        glUseProgram(self.__phong_program)
        glUniformMatrix4fv(self.__phong_uniform['u_proj'], 1, GL_FALSE, glm.value_ptr(projection))
        glUniformMatrix4fv(self.__phong_uniform['u_view'], 1, GL_FALSE, glm.value_ptr(view))
        glUniform4fv(self.__phong_uniform['u_light_pos'], 1, glm.value_ptr(glm.vec4(light_pos, 1)))
        glUniform4fv(self.__phong_uniform['u_light_dir'], 1, glm.value_ptr(glm.vec4(light_dir, light_cone_cos)))
        glUniformMatrix4fv(self.__phong_uniform['u_shadow_proj'], 1, GL_FALSE, glm.value_ptr(shadow_proj))
        glUniformMatrix4fv(self.__phong_uniform['u_shadow_view'], 1, GL_FALSE, glm.value_ptr(shadow_view))
        
        glBindTextureUnit(1, self.__shadow_depth_to)
        self.__multimesh.draw()
        
        glutSwapBuffers()
        glutPostRedisplay()


window = MyWindow(800, 600)
window.run()