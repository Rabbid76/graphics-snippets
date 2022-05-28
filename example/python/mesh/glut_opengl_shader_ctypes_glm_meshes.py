import os, sys 
sys.path.append(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..'))

import math 
import ctypes 
import glm 
import time
from PIL import Image
from OpenGL.GLUT import *
from OpenGL.GL import *
from OpenGL.GL.shaders import *
from OpenGL.arrays import *
from utility.glut_utility import GlutNavigation
from utility import triangulated_mesh
from utility.opengl_mesh import SingleMesh, MultiMesh

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
    
class MyWindow:

    __glsl_vert = phong_vert
    __glsl_frag = phong_frag

    def __init__(self, w, h):
        
        self.__caption = 'OpenGL Window'
        
        glutInit()
        glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH | GLUT_MULTISAMPLE)
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
            #triangulated_mesh.Arrow()
        ]

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
        self.__auto_rotate = True
        self.__angle1 = 0
        self.__angle2 = 0
        self.__sceen_shot = False
        self.screenshot_prefix = "glut_opengl_shader_ctypes_glm_meshes_"
        self.vp_size = [0, 0]

        glEnable(GL_DEPTH_TEST)
        glUseProgram(self.__program)
        #glPolygonMode(GL_FRONT_AND_BACK, GL_LINE)

        glutDisplayFunc(self.__mainloop)
        glutKeyboardFunc(self.__key_pressed)

    def run(self):
        self.__starttime = 0
        self.__starttime = self.elapsed_ms()
        glutMainLoop()

    def elapsed_ms(self):
      return glutGet(GLUT_ELAPSED_TIME) - self.__starttime

    def __key_pressed(self, key, x, y):
        if key == b'r':
            self.__auto_rotate = not self.__auto_rotate
        elif key == b's':
            self.__sceen_shot = True    

    def __mainloop(self):

        vp_valid, self.vp_size, view, projection = self.__glut_navigation.update()
        if not vp_valid:
            glViewport(0, 0, *self.vp_size)

        glUniformMatrix4fv(self.___uniform['u_proj'], 1, GL_FALSE, glm.value_ptr(projection))
        glUniformMatrix4fv(self.___uniform['u_view'], 1, GL_FALSE, glm.value_ptr(view))
        
        glClearColor(0.2, 0.3, 0.3, 1.0)
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)

        if self.__auto_rotate:
            self.__angle1 = self.elapsed_ms() * math.pi * 2 / 5000.0
            self.__angle2 = self.elapsed_ms() * math.pi * 2 / 7333.0
        model_matrices = []
        for i, mesh in enumerate(self.__meshes):
            angleY = self.__angle1 + math.pi*2 * i / len(self.__meshes)
            #angleY =  math.pi*2 * i / len(self.__meshes)
            model = glm.mat4(1)
            model = glm.rotate(model, angleY, glm.vec3(0, 0, 1))
            model = glm.translate(model, glm.vec3(self.__diameter/2, 0, 0))
            model = glm.rotate(model, self.__angle2, glm.vec3(0, 1, 0))
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

        if self.__sceen_shot:
            self.__sceen_shot = False
            image_data = glReadPixels(0, 0, *self.vp_size, GL_RGBA, GL_UNSIGNED_BYTE)
            image = Image.frombytes('RGBA', self.vp_size, image_data).transpose(method=Image.FLIP_TOP_BOTTOM)
            image.save(self.screenshot_prefix + time.strftime("_%Y%m%d_%H%M%S") + '.png', 'PNG')


window = MyWindow(1280, 640)
window.run()