import os 
#import math 
#import ctypes 
#import glm 
import numpy as np
from OpenGL.GLUT import *
from OpenGL.GL import *
from OpenGL.GL.shaders import *
from OpenGL.arrays import *

class MyWindow:

    __glsl_vert = """
        #version 450 core

        layout (location = 0) in vec3 a_pos;
        layout (location = 1) in vec3 a_nv;
        layout (location = 2) in vec4 a_col;

        out vec3 v_pos;
        out vec3 v_nv;
        out vec4 v_color;

        uniform ivec3 u_dim;
        uniform mat4 u_proj; 
        uniform mat4 u_view; 
        uniform mat4 u_model; 

        void main()
        {
            vec3 scale = 1.0 / vec3(u_dim);
            //scale = vec3(min(scale.x, min(scale.y, scale.z)));
            ivec3 inx = ivec3(0);
            inx.z = gl_InstanceID / (u_dim.x * u_dim.y);
            inx.y = (gl_InstanceID - inx.z * u_dim.x * u_dim.y) / u_dim.x;
            inx.x = gl_InstanceID - inx.z * u_dim.x * u_dim.y - inx.y * u_dim.x;

            vec3 trans = 2 * scale * (vec3(inx) - vec3(u_dim-1) / 2.0);
            float gap_scale = 0.8;
            mat4 instanceMat = mat4(
                vec4(scale.x * gap_scale, 0.0, 0.0, 0.0),
                vec4(0.0, scale.y * gap_scale, 0.0, 0.0),
                vec4(0.0, 0.0, scale.z * gap_scale, 0.0),
                vec4(trans, 1.0)
            );

            mat4 model_view = u_view * u_model * instanceMat;
            mat3 normal     = transpose(inverse(mat3(model_view)));
            
            vec4 view_pos   = model_view * vec4(a_pos.xyz, 1.0);

            v_pos       = view_pos.xyz;
            v_nv        = normal * a_nv;  
            v_color     = a_col;
            gl_Position = u_proj * view_pos;
        }
    """

    __glsl_frag = """
        #version 450 core
        
        out vec4 frag_color;
        in  vec3 v_pos;
        in  vec3 v_nv;
        in  vec4 v_color;

        void main()
        {
            vec3  N    = normalize(v_nv);
            vec3  V    = -normalize(v_pos);
            float ka   = 0.1;
            float kd   = max(0.0, dot(N, V)) * 0.9;
            frag_color = vec4(v_color.rgb * (ka + kd), v_color.a);
        }
    """

    def __init__(self, w, h):
        
        self.__caption = 'OpenGL Window'
        self.__vp_valid = False
        self.__vp_size = [w, h]

        glutInit()
        glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH)
        glutInitWindowSize(self.__vp_size[0], self.__vp_size[1])
        self.__glut_wnd = glutCreateWindow(self.__caption)

        self.__program = compileProgram( 
            compileShader( self.__glsl_vert, GL_VERTEX_SHADER ),
            compileShader( self.__glsl_frag, GL_FRAGMENT_SHADER ),
        )
        self.___attrib = { a : glGetAttribLocation (self.__program, a) for a in ['a_pos', 'a_nv', 'a_col'] }
        print(self.___attrib)
        self.___uniform = { u : glGetUniformLocation (self.__program, u) for u in ['u_model', 'u_view', 'u_proj', 'u_dim'] }
        print(self.___uniform)

        v = [[-1,-1,1], [1,-1,1], [1,1,1], [-1,1,1], [-1,-1,-1], [1,-1,-1], [1,1,-1], [-1,1,-1]]
        c = [[1.0, 0.0, 0.0], [1.0, 0.5, 0.0], [1.0, 0.0, 1.0], [1.0, 1.0, 0.0], [0.0, 1.0, 0.0], [0.0, 0.0, 1.0]]
        n = [[0,0,1], [1,0,0], [0,0,-1], [-1,0,0], [0,1,0], [0,-1,0]]
        e = [[0,1,2,3], [1,5,6,2], [5,4,7,6], [4,0,3,7], [3,2,6,7], [1,0,4,5]]
        index_array = [si*4+[0, 1, 2, 0, 2, 3][vi] for si in range(6) for vi in range(6)]
        attr_array = []
        for si in range(len(e)):
            for vi in e[si]:
                attr_array += [*v[vi], *n[si], *c[si], 1]
        
        self.__no_vert = len(attr_array) // 10
        self.__no_indices = len(index_array)
        vertex_attributes = np.array(attr_array, dtype=np.float32)
        indices = np.array(index_array, dtype=np.uint32)
        
        self.__vao = glGenVertexArrays(1)
        self.__vbo, self.__ibo = glGenBuffers(2)
        
        glBindVertexArray(self.__vao)

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, self.__ibo)
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices, GL_STATIC_DRAW)

        glBindBuffer(GL_ARRAY_BUFFER, self.__vbo)
        glBufferData(GL_ARRAY_BUFFER, vertex_attributes, GL_STATIC_DRAW)

        float_size = vertex_attributes.itemsize  
        glVertexAttribPointer(0, 3, GL_FLOAT, False, 10*float_size, None)
        glVertexAttribPointer(1, 3, GL_FLOAT, False, 10*float_size, c_void_p(3*float_size))
        glVertexAttribPointer(2, 4, GL_FLOAT, False, 10*float_size, c_void_p(6*float_size))
        glEnableVertexAttribArray(0)
        glEnableVertexAttribArray(1)
        glEnableVertexAttribArray(2)

        glEnable(GL_DEPTH_TEST)
        glUseProgram(self.__program)

        glutReshapeFunc(self.__reshape)
        glutDisplayFunc(self.__mainloop)

    def run(self):
        self.__starttime = 0
        self.__starttime = self.elapsed_ms()
        glutMainLoop()

    def elapsed_ms(self):
      return glutGet(GLUT_ELAPSED_TIME) - self.__starttime

    def __reshape(self, w, h):
        self.__vp_valid = False

    def __mainloop(self):

        if not self.__vp_valid:
            self.__vp_size = [glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT)]
            self.__vp_valid = True
            glViewport(0, 0, self.__vp_size[0], self.__vp_size[1])

        aspect, ta, near, far = self.__vp_size[0]/self.__vp_size[1], np.tan(np.radians(90.0) / 2), 0.1,  10
        proj = np.array(((1/ta/aspect, 0, 0, 0), (0, 1/ta, 0, 0), (0, 0, -(far+near)/(far-near), -1), (0, 0, -2*far*near/(far-near), 0)), np.float32)
        
        view = np.array(((1, 0, 0, 0), (0, 0, -1, 0), (0, 1, 0, 0), (0, 0, -3, 1)), np.float32)
        
        c1, s1 , c2, s2 = (f(self.elapsed_ms() * np.pi * 2 / tf) for tf in [5000.0, 7333.0] for f in [np.cos, np.sin])
        rotMatX = np.array(((1, 0, 0, 0), (0, c1, s1, 0), (0, -s1, c1, 0), (0, 0, 0, 1)), np.float32)
        rotMatY = np.array(((c2, 0, -s2, 0), (0, 1, 0, 0), (s2, 0, c2, 0), (0, 0, 0, 1)), np.float32)
        model = np.matmul(rotMatY, rotMatX)
       
        dim = [5, 3, 4]
        glUniform3i(self.___uniform['u_dim'], *dim)
        glUniformMatrix4fv(self.___uniform['u_proj'], 1, GL_FALSE, proj )
        glUniformMatrix4fv(self.___uniform['u_view'], 1, GL_FALSE, view )
        glUniformMatrix4fv(self.___uniform['u_model'], 1, GL_FALSE, model )

        #glUniformMatrix4fv(self.___uniform['u_model'], 1, GL_FALSE, model.ctypes.data_as(ctypes.POINTER(ctypes.c_void_p)) )

        glClearColor(0.2, 0.3, 0.3, 1.0)
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)
          
        glDrawElementsInstanced(GL_TRIANGLES, self.__no_indices, GL_UNSIGNED_INT, None, dim[0] * dim[1] * dim[2])

        glutSwapBuffers()
        glutPostRedisplay()


window = MyWindow(800, 600)
window.run()