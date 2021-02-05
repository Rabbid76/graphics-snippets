# Adding Shadows to Parallax Occlusion Map
# https://stackoverflow.com/questions/55089830/adding-shadows-to-parallax-occlusion-map/55091654#55091654

import os 
import math 
import ctypes 
import numpy as np
import glm 
from OpenGL.GLUT import *
from OpenGL.GL import *
from OpenGL.GL.shaders import *
from OpenGL.arrays import *
from ctypes import c_void_p
from PIL import Image
os.chdir(os.path.join(os.path.dirname(os.path.abspath(__file__)), '../../../resource/texture'))

def ReadTexture(filename, textureUnit):
      # PIL can open BMP, EPS, FIG, IM, JPEG, MSP, PCX, PNG, PPM
      # and other file types.  We convert into a texture using GL.
      print('trying to open', filename)
      try:
         image = Image.open(filename)
      except IOError as ex:
         print('IOError: failed to open texture file ', filename)
         sys.exit()
         return -1
      print('opened file: size=', image.size, 'format=', image.format)
      imageData = np.array(list(image.getdata()), np.uint8)
      format = GL_RGB if imageData.shape[1] == 3 else GL_RGBA

      glActiveTexture( GL_TEXTURE0 + textureUnit )
      textureObj = glGenTextures( 1  )
      glPixelStorei(GL_UNPACK_ALIGNMENT, 4)
      glBindTexture(GL_TEXTURE_2D, textureObj)
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0)
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0)
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image.size[0], image.size[1],0, format, GL_UNSIGNED_BYTE, imageData)

      #glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR)
      #glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR)
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR)
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR)
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT)
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT)
      #glGenerateMipmap(GL_TEXTURE_2D)

      image.close()
      return textureObj

class MyWindow:

    __caption = 'OpenGL Window'
    __vp_size = [800, 600]
    __vp_valid = False
    __glut_wnd = None
    
    __glsl_vert = """
        #version 450 core

        layout (location = 0) in vec3 a_pos;
        layout (location = 1) in vec3 a_nv;
        layout (location = 2) in vec3 a_tv;
        layout (location = 3) in vec2 a_uv;
        layout (location = 4) in vec4 a_col;

        out vec3 o_worldPos;
        out vec2 o_texCoord;
        out vec4 o_color;
        out vec3 world_normal;
        out vec3 world_tangent;

        uniform mat4 u_proj; 
        uniform mat4 u_view; 
        uniform mat4 u_model; 

        void main()
        {
            mat4 model_view = u_model;
            mat3 normal     = transpose(inverse(mat3(model_view)));
            
            vec4 view_pos   = model_view * vec4(a_pos.xyz, 1.0);

            o_worldPos    = view_pos.xyz;
            world_normal  = normal * a_nv;
            world_tangent = normal * a_tv;
            o_texCoord    = a_uv;  
            o_color       = a_col;
            gl_Position   = u_proj * u_view * view_pos;
        }
    """

    __glsl_frag = """
        #version 450 core
        
        out vec4 fragColor;
        in  vec3 o_worldPos;
        in  vec2 o_texCoord;
        in  vec4 o_color;
        in  vec3 world_normal;
        in  vec3 world_tangent;

        layout (binding  = 0) uniform sampler2D diffuseMap;
        layout (binding  = 1) uniform sampler2D normalMap;
        layout (binding  = 2) uniform sampler2D heightMap;

        uniform vec3 light_pos;
        uniform vec3 viewPosition;
        uniform float heightScale;

        void main()
        {
            vec4 color = texture(diffuseMap, o_texCoord);

            vec3  N   = normalize(world_normal);
            vec3  L   = normalize(light_pos- o_worldPos);
            vec3  V   = -normalize(o_worldPos - viewPosition);
            float ka  = 0.1;
            float kd  = max(0.0, dot(N, L)) * 0.9;
            fragColor = vec4(color.rgb * (ka + kd), color.a);
        }
    """

    __glsl_parallax_frag = """
        #version 450 core
        
        out vec4 fragColor;
        in  vec3 o_worldPos;
        in  vec2 o_texCoord;
        in  vec4 o_color;
        in  vec3 world_normal;
        in  vec3 world_tangent;

        layout (binding  = 0) uniform sampler2D diffuseMap;
        layout (binding  = 1) uniform sampler2D normalMap;
        layout (binding  = 2) uniform sampler2D heightMap;

        uniform vec3 light_pos;
        uniform vec3 viewPosition;
        uniform float heightScale;

        vec2 ParallaxMapping (vec2 texCoord, vec3 viewDir)
        {
            if ( viewDir.z >= 0.0 )
               return texCoord;

            float minLayers = 1;
            float maxLayers = 32;
            float numLayers = mix(maxLayers, minLayers, abs(dot(vec3(0.0, 0.0, 1.0), viewDir)));
            float layerDepth = 1.0 / numLayers;

            float currentLayerDepth = 0;

            vec2 P = viewDir.xy / viewDir.z * heightScale;
            vec2 deltaTexCoords = P / numLayers;
            vec2 currentTexCoords = texCoord;

            float currentDepthMapValue = texture(heightMap, currentTexCoords).r;

            while (currentLayerDepth < currentDepthMapValue)
            {
                currentTexCoords -= deltaTexCoords;
                currentDepthMapValue = texture(heightMap, currentTexCoords).r;
                currentLayerDepth += layerDepth;
            }

            vec2 prevTexCoords = currentTexCoords + deltaTexCoords;
            float afterDepth = currentDepthMapValue - currentLayerDepth;
            float beforeDepth = texture(heightMap, prevTexCoords).r - currentLayerDepth + layerDepth;

            float weight = afterDepth / (afterDepth - beforeDepth);

            vec2 finalTexCoords = prevTexCoords * weight + currentTexCoords * (1.0 - weight);

            return finalTexCoords;
        }

        // FUNCTION I ADDED FOR SHADOW CALCULATION
        float ShadowCalc(vec2 texCoord, vec3 lightDir)
        {
            if ( lightDir.z >= 0.0 )
               return 0.0;

            float minLayers = 1;
            float maxLayers = 32;
            float numLayers = mix(maxLayers, minLayers, abs(dot(vec3(0.0, 0.0, 1.0), lightDir)));

            vec2 currentTexCoords = texCoord;
            float currentDepthMapValue = texture(heightMap, currentTexCoords).r;
            float currentLayerDepth = currentDepthMapValue;

            float layerDepth = 1.0 / numLayers;
            vec2 P = lightDir.xy / lightDir.z * heightScale;
            vec2 deltaTexCoords = P / numLayers;

            while (currentLayerDepth <= currentDepthMapValue && currentLayerDepth > 0.0)
            {
                currentTexCoords += deltaTexCoords;
                currentDepthMapValue = texture(heightMap, currentTexCoords).r;
                currentLayerDepth -= layerDepth;
            }

            float r = currentLayerDepth > currentDepthMapValue ? 0.0 : 1.0;
            return r;
        }

        void main()
        {
            mat3 TBN_norm = transpose(mat3(normalize(world_tangent),
                                          normalize(cross(world_normal, world_tangent)),
                                          normalize(world_normal)));
            
            vec3 viewDir = TBN_norm * normalize(o_worldPos - viewPosition);
            vec3 lightDir = TBN_norm * normalize(o_worldPos - light_pos);
            
            vec2 currentTex = ParallaxMapping(o_texCoord, viewDir);
            if (currentTex.x > 1.0 || currentTex.y > 1.0 || currentTex.x < 0.0 || currentTex.y < 0.0)
                discard;
            
            vec3 normal = texture(normalMap, currentTex).rgb;
            normal = normalize(normal * 2.0 - 1.0);
            float dc = max(0.0, dot(-lightDir, normal));

            // STUFF I ADDED FOR SHADOWS
            float shadow = dc > 0.0 ? ShadowCalc(currentTex, lightDir) : 0.0;
            
            fragColor = shadow * dc * texture(diffuseMap, currentTex);
        }
    """

    __program = None
    __vao = None
    __vbo = None
    __no_vert = 0

    def __init__(self, w, h):
        
        self.__vp_size = [w, h]

        glutInit()
        glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH)
        glutInitWindowSize(self.__vp_size[0], self.__vp_size[1])
        __glut_wnd = glutCreateWindow(self.__caption)

        texObj = ReadTexture('woodtiles.jpg', 0)
        nmTexObj = ReadTexture('toy_box_normal.png', 1);   
        hmTexObj = ReadTexture('toy_box_disp.png', 2);  
        print(texObj, nmTexObj, hmTexObj)
        
        self.__program = compileProgram( 
            compileShader( self.__glsl_vert, GL_VERTEX_SHADER ),
            compileShader( self.__glsl_parallax_frag, GL_FRAGMENT_SHADER ),
        )

        self.___attrib = { a : glGetAttribLocation (self.__program, a) for a in ['a_pos', 'a_nv', 'a_col'] }
        print(self.___attrib)

        self.___uniform = { u : glGetUniformLocation (self.__program, u) for u in ['u_model', 'u_view', 'u_proj', 'viewPosition', 'light_pos', 'heightScale'] }
        print(self.___uniform)

        v  = [ -1,-1,1,  1,-1,1,  1,1,1, -1,1,1, -1,-1,-1,  1,-1,-1,  1,1,-1, -1,1,-1 ]
        c  = [ 1.0, 0.0, 0.0,   1.0, 0.5, 0.0,    1.0, 0.0, 1.0,   1.0, 1.0, 0.0,   0.0, 1.0, 0.0,   0.0, 0.0, 1.0 ]
        nv = [ 0,0,1, 1,0,0, 0,0,-1, -1,0,0, 0,1,0, 0,-1,0 ]
        tv = [ 1,0,0, 0,0,-1, -1,0,0, 0,0,1, 1,0,0, -1,0,0 ]
        t  = [ 0, 0, 1, 0, 1, 1, 0, 1]
        e  = [ 0,1,2,3, 1,5,6,2, 5,4,7,6, 4,0,3,7, 3,2,6,7, 1,0,4,5 ]
        attr_array = []
        index_array = []
        for si in range(6):
            temp = []
            for vi in range(6):
                ci = [0, 1, 2, 0, 2, 3][vi]
                i = si*4+ci
                temp.append(i)
            index_array.append(temp)
            for ci in range(4):
                i = si*4+ci
                attr_array.extend( [ v[e[i]*3], v[e[i]*3+1], v[e[i]*3+2] ] )
                attr_array.extend( [ nv[si*3], nv[si*3+1], nv[si*3+2] ] )
                attr_array.extend( [ tv[si*3], tv[si*3+1], tv[si*3+2] ] )
                attr_array.extend( [ t[ci*2], t[ci*2+1] ] )
                attr_array.extend( [ c[si*3], c[si*3+1], c[si*3+2], 1 ] ); 
        self.__no_vert = len(attr_array) // 10

        vertex_attributes = np.array(attr_array, dtype=np.float32)
        #vertex_attributes = (ctypes.c_float * len(attr_array))(*attr_array)

        self.n_indices = len(index_array)*6
        #indices = (ctypes.c_uint32 * self.n_indices)(*index_array)
        indices = np.array(index_array, dtype=np.uint32)

        self.__vbo = glGenBuffers(2)
        glBindBuffer(GL_ARRAY_BUFFER, self.__vbo[0])
        glBufferData(GL_ARRAY_BUFFER, vertex_attributes, GL_STATIC_DRAW)
        
        self.__vao = glGenVertexArrays(1)
        glBindVertexArray(self.__vao)

        self.__ibo = glGenBuffers(1)
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, self.__ibo)
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices, GL_STATIC_DRAW)

        glVertexAttribPointer(0, 3, GL_FLOAT, False, 15*vertex_attributes.itemsize, None)
        glEnableVertexAttribArray(0)
        glVertexAttribPointer(1, 3, GL_FLOAT, False, 15*vertex_attributes.itemsize, c_void_p(3*4))
        glEnableVertexAttribArray(1)
        glVertexAttribPointer(2, 3, GL_FLOAT, False, 15*vertex_attributes.itemsize, c_void_p(6*4))
        glEnableVertexAttribArray(2)
        glVertexAttribPointer(3, 2, GL_FLOAT, False, 15*vertex_attributes.itemsize, c_void_p(9*4))
        glEnableVertexAttribArray(3)
        glVertexAttribPointer(4, 4, GL_FLOAT, False, 15*vertex_attributes.itemsize, c_void_p(11*4))
        glEnableVertexAttribArray(4)

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

        proj  = glm.mat4(1)
        view  = glm.mat4(1)
        model = glm.mat4(1)

        aspect = self.__vp_size[0]/self.__vp_size[1]
        proj = glm.perspective(glm.radians(75.0), aspect, 0.1, 10.0)

        heightScale = 0.1
        light_pos = glm.vec3(-30,-10,0)
        view_pos = glm.vec3(0,-3,0)
        view = glm.lookAt(view_pos, glm.vec3(0, 0, 0), glm.vec3(0,0,1))
        
        speed = 0.2
        angle1 = self.elapsed_ms() * speed * math.pi * 2 / 5000.0
        angle2 = self.elapsed_ms() * speed * math.pi * 2 / 7333.0
        #model = glm.rotate(model, angle1, glm.vec3(1, 0, 0))
        #model = glm.rotate(model, angle2, glm.vec3(0, 1, 0))
        model = glm.rotate(model, angle2, glm.vec3(0, 0, 1))
        angle = 82 + 10 * math.sin(self.elapsed_ms() / 250.0)
        #model = glm.rotate(model, glm.radians(angle), glm.vec3(0, 0, 1))
        model = glm.rotate(model, glm.radians(10), glm.vec3(0, 1, 0))

   
        glUniformMatrix4fv(self.___uniform['u_proj'], 1, GL_FALSE, glm.value_ptr(proj) )
        glUniformMatrix4fv(self.___uniform['u_view'], 1, GL_FALSE, glm.value_ptr(view) )
        glUniformMatrix4fv(self.___uniform['u_model'], 1, GL_FALSE, glm.value_ptr(model) )
        glUniform3fv(self.___uniform['viewPosition'], 1, glm.value_ptr(view_pos) )
        glUniform3fv(self.___uniform['light_pos'], 1, glm.value_ptr(light_pos) )
        glUniform1f(self.___uniform['heightScale'], heightScale )

        glClearColor(0.2, 0.3, 0.3, 1.0)
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)
          
        #glDrawArrays(GL_TRIANGLES, 0, self.__no_vert)
        glDrawElements(GL_TRIANGLES, self.n_indices, GL_UNSIGNED_INT, None)

        glutSwapBuffers()
        glutPostRedisplay()


window = MyWindow(800, 600)
window.run()