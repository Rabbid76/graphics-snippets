[Visualizing a 3D NumPy array with PyOpenGL](https://stackoverflow.com/questions/18085655/visualizing-a-3d-numpy-array-with-pyopengl)

This task is perfectly suited for [Instancing](https://www.khronos.org/opengl/wiki/Vertex_Rendering#Instancing). With instancing an object can be rendered multiple times.

In this case instancing is used to render a cube for ach element of a 3d [NumPy](http://www.numpy.org/) array.

Lets assume we've the following 3D array (`array3d`) of random values in the range [0, 1]:

<!-- language: py -->

    shape = [5, 4, 6]
    number_of = shape[0] * shape[1] * shape[2]  
    array3d = np.array(np.random.rand(number_of), dtype=np.float32).reshape(shape)

For each element of the array an instance of a mesh (cube) has to be rendered:

e.g.

<!-- language: py -->

    number_of = array3d.shape[0] * array3d.shape[1] * array3d.shape[2]  
    glDrawElementsInstanced(GL_TRIANGLES, self.__no_indices, GL_UNSIGNED_INT, None, number_of)

The array can be loaded to a 3D texture ([`glTexImage3D`](https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glTexImage3D.xhtml)):

<!-- language: py -->

    glActiveTexture(GL_TEXTURE1)
    tex3DObj = glGenTextures(1)
    glBindTexture(GL_TEXTURE_3D, tex3DObj)
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAX_LEVEL, 0)
    glTexImage3D(GL_TEXTURE_3D, 0, GL_R16F, *array3d.shape, 0, GL_RED, GL_FLOAT, array3d)

In the vertex shader for a single cube, a instance transformation matrix can be computes by the dimension of the 3D texture (which is equal the shape of the 3D array) and the [`gl_InstanceID`](https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/gl_InstanceID.xhtml) of the element cube.  
The element cube is further scaled by the value of the element in the 3D texture.

Assuming a vertex shader with a §D texture sampler uniform `u_array3D` and a vertex coordinate attribute `a_pos`:

<!-- language: glsl -->

    in vec3 a_pos;
    uniform sampler3D u_array3D;

The dimension of the texture can be get by [`textureSize`](https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/textureSize.xhtml):

<!-- language: glsl -->

    ivec3 dim = textureSize(u_array3D, 0);

With the dimension and the [`gl_InstanceID`](https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/gl_InstanceID.xhtml), the index of the element can be computed:

<!-- language: glsl -->

    ivec3 inx = ivec3(0);
    inx.z = gl_InstanceID / (dim.x * dim.y);
    inx.y = (gl_InstanceID - inx.z * dim.x * dim.y) / dim.x;
    inx.x = gl_InstanceID - inx.z * dim.x * dim.y - inx.y * dim.x;

and the value of the element can be fetched ([`texelFetch`](https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/texelFetch.xhtml)):

<!-- language: glsl -->

    float value = texelFetch(u_array3D, inx, 0).x;

Finally a instance transformation matrix dependent on the element index and element value can be calculated: 

<!-- language: glsl -->

    vec3 scale = 1.0 / vec3(dim);
    scale = vec3(min(scale.x, min(scale.y, scale.z)));
    vec3 trans = 2 * scale * (vec3(inx) - vec3(dim-1) / 2.0);
    mat4 instanceMat = mat4(
        vec4(scale.x * cube_scale, 0.0, 0.0, 0.0),
        vec4(0.0, scale.y * cube_scale, 0.0, 0.0),
        vec4(0.0, 0.0, scale.z * cube_scale, 0.0),
        vec4(trans, 1.0)
    );

    vec4 instance_pos = instanceMat * vec4(a_pos, 1.0);

The value can be additionally visualized by the color of the cube. For this the floating point value in the range [0.0, 1.0] is transformed to a RGB color in the [HSV](https://en.wikipedia.org/wiki/HSL_and_HSV#From_HSV) color range:

[![][1]][1]

<!-- language: glsl -->

    vec3 HUEtoRGB(in float H)
    {
        float R = abs(H * 6.0 - 3.0) - 1.0;
        float G = 2.0 - abs(H * 6.0 - 2.0);
        float B = 2.0 - abs(H * 6.0 - 4.0);
        return clamp( vec3(R,G,B), 0.0, 1.0 );
    }

<!-- language: glsl -->

    vec3 color = HUEtoRGB(0.66 * (1-0 - value));

See the pure [NumPy](http://www.numpy.org/) / [PyOpenGL](http://pyopengl.sourceforge.net/) example program. The values of the array are changed randomly:

[![][2]][2]

<!-- language: py -->

    import numpy as np
    from OpenGL.GLUT import *
    from OpenGL.GL import *
    from OpenGL.GL.shaders import *

    class MyWindow:

        __glsl_vert = """
            #version 450 core

            layout (location = 0) in vec3 a_pos;
            layout (location = 1) in vec3 a_nv;
            layout (location = 2) in vec4 a_col;

            out vec3 v_pos;
            out vec3 v_nv;
            out vec4 v_color;

            layout (binding = 1) uniform sampler3D u_array3D;

            uniform mat4 u_proj; 
            uniform mat4 u_view; 
            uniform mat4 u_model; 

            vec3 HUEtoRGB(in float H)
            {
                float R = abs(H * 6.0 - 3.0) - 1.0;
                float G = 2.0 - abs(H * 6.0 - 2.0);
                float B = 2.0 - abs(H * 6.0 - 4.0);
                return clamp( vec3(R,G,B), 0.0, 1.0 );
            }

            void main()
            {
                ivec3 dim = textureSize(u_array3D, 0);

                vec3 scale = 1.0 / vec3(dim);
                scale = vec3(min(scale.x, min(scale.y, scale.z)));
                
                ivec3 inx = ivec3(0);
                inx.z = gl_InstanceID / (dim.x * dim.y);
                inx.y = (gl_InstanceID - inx.z * dim.x * dim.y) / dim.x;
                inx.x = gl_InstanceID - inx.z * dim.x * dim.y - inx.y * dim.x;
                float value = texelFetch(u_array3D, inx, 0).x;

                vec3 trans = 2 * scale * (vec3(inx) - vec3(dim-1) / 2.0);
                mat4 instanceMat = mat4(
                    vec4(scale.x * value, 0.0, 0.0, 0.0),
                    vec4(0.0, scale.y * value, 0.0, 0.0),
                    vec4(0.0, 0.0, scale.z * value, 0.0),
                    vec4(trans, 1.0)
                );

                mat4 model_view = u_view * u_model * instanceMat;
                mat3 normal     = transpose(inverse(mat3(model_view)));
                
                vec4 view_pos   = model_view * vec4(a_pos.xyz, 1.0);

                v_pos       = view_pos.xyz;
                v_nv        = normal * a_nv;  
                v_color     = vec4(HUEtoRGB(0.66 * (1-0 - value)), 1.0);
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
            self.___uniform = { u : glGetUniformLocation (self.__program, u) for u in ['u_model', 'u_view', 'u_proj'] }
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

            shape = [5, 4, 6]
            number_of = shape[0] * shape[1] * shape[2]  
            self.array3d = np.array(np.random.rand(number_of), dtype=np.float32).reshape(shape)

            glActiveTexture(GL_TEXTURE1)
            self.tex3DObj = glGenTextures(1)
            glBindTexture(GL_TEXTURE_3D, self.tex3DObj)
            glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAX_LEVEL, 0)
            glTexImage3D(GL_TEXTURE_3D, 0, GL_R16F, *self.array3d.shape, 0, GL_RED, GL_FLOAT, self.array3d)

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

            number_of = self.array3d.shape[0] * self.array3d.shape[1] * self.array3d.shape[2]  
            rand = (np.random.rand(number_of) - 0.5) * 0.05
            self.array3d = np.clip(np.add(self.array3d, rand.reshape(self.array3d.shape)), 0, 1)
            glTexSubImage3D(GL_TEXTURE_3D, 0, 0, 0, 0, *self.array3d.shape, GL_RED, GL_FLOAT, self.array3d)

            if not self.__vp_valid:
                self.__vp_size = [glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT)]
                self.__vp_valid = True
                glViewport(0, 0, self.__vp_size[0], self.__vp_size[1])

            aspect, ta, near, far = self.__vp_size[0]/self.__vp_size[1], np.tan(np.radians(90.0) / 2), 0.1, 10
            proj = np.array(((1/ta/aspect, 0, 0, 0), (0, 1/ta, 0, 0), (0, 0, -(far+near)/(far-near), -1), (0, 0, -2*far*near/(far-near), 0)), np.float32)
            
            view = np.array(((1, 0, 0, 0), (0, 0, -1, 0), (0, 1, 0, 0), (0, 0, -2, 1)), np.float32)
            c, s = (f(np.radians(30.0)) for f in [np.cos, np.sin])
            viewRotX = np.array(((1, 0, 0, 0), (0, c, s, 0), (0, -s, c, 0), (0, 0, 0, 1)), np.float32)
            view = np.matmul(viewRotX, view)
            
            c1, s1, c2, s2, c3, s3 = (f(self.elapsed_ms() * np.pi * 2 / tf) for tf in [5000.0, 7333.0, 10000.0] for f in [np.cos, np.sin])
            rotMatZ = np.array(((c3, s3, 0, 0), (-s3, c3, 0, 0), (0, 0, 1, 0), (0, 0, 0, 1)), np.float32)
            model = rotMatZ
        
            glUniformMatrix4fv(self.___uniform['u_proj'], 1, GL_FALSE, proj )
            glUniformMatrix4fv(self.___uniform['u_view'], 1, GL_FALSE, view )
            glUniformMatrix4fv(self.___uniform['u_model'], 1, GL_FALSE, model )

            glClearColor(0.2, 0.3, 0.3, 1.0)
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)
            
            glDrawElementsInstanced(GL_TRIANGLES, self.__no_indices, GL_UNSIGNED_INT, None, number_of)

            glutSwapBuffers()
            glutPostRedisplay()

    window = MyWindow(800, 600)
    window.run()


  [1]: https://i.stack.imgur.com/Ukh2s.png
  [2]: https://i.stack.imgur.com/twWaF.gif