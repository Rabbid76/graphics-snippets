# OpenGL.GL.shaders [http://pyopengl.sourceforge.net/pydoc/OpenGL.GL.shaders.html]
# Introduction to Shaders: Uniform Values [http://pyopengl.sourceforge.net/context/tutorials/shader_3.html]

from OpenGL.GLUT import *
from OpenGL.GL import *
from OpenGL.GL.shaders import *
#from OpenGL.arrays import *

class MyWindow:

    __caption = 'OpenGL Window'
    __vp_size = [512, 512]
    __vp_valid = False
    
    __glsl_comp = """
        #version 430
        layout(local_size_x = 1, local_size_y = 1) in;
        layout(rgba32f, binding = 0) uniform image2D img_output;

        void main() {

            ivec2 pixel_coords = ivec2(gl_GlobalInvocationID.xy);
            vec2 uv = gl_GlobalInvocationID.xy / vec2(512.0);
            vec4 pixel = vec4(1.0-uv.xy, uv.x*uv.y, 1.0);

            imageStore(img_output, pixel_coords, pixel);
        }
    """

    __program = None
    __vao = None
    __vbo = None

    def __init__(self, w, h):
        
        self.__vp_size = [w, h]
        width, height = self.__vp_size

        glutInit()
        glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH)
        glutInitWindowSize(self.__vp_size[0], self.__vp_size[1])
        __glut_wnd = glutCreateWindow(self.__caption)

        shaderObj = glCreateShader( GL_COMPUTE_SHADER )
        glShaderSource( shaderObj, self.__glsl_comp )
        glCompileShader( shaderObj )
        result = glGetShaderiv( shaderObj, GL_COMPILE_STATUS )
        if not (result):
            print( glGetShaderInfoLog( shaderObj ) )
            sys.exit()
        
        programObj = glCreateProgram()
        glAttachShader( programObj, shaderObj )
        glLinkProgram( programObj )
        result = glGetProgramiv( programObj, GL_LINK_STATUS )
        if not ( result ):
            print( 'link error:' )
            print( glGetProgramInfoLog( programObj ) )
            sys.exit()
        glUseProgram(programObj)
        self.__prog = programObj
        
        tex_out = glGenTextures( 1 )
        glActiveTexture(GL_TEXTURE0)
        glBindTexture(GL_TEXTURE_2D, tex_out)
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR )
        #glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR )
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, None)
        glBindImageTexture(0, tex_out, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F)
    
        fbo = GLuint(0)
        glGenFramebuffers(1, fbo)
        glBindFramebuffer(GL_FRAMEBUFFER, fbo)
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex_out, 0)
        if glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE:
            print("framebuffer incomplete")
            sys.exit()

        glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo)
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0)
    
        glutReshapeFunc(self.__reshape)
        glutDisplayFunc(self.__mainloop)

    def run(self):
        glutMainLoop()

    def __reshape(self, w, h):
        self.__vp_valid = False

    def __mainloop(self):

        if not self.__vp_valid:
            self.__vp_size = [glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT)]
            self.__vp_valid = True
            glViewport(0, 0, self.__vp_size[0], self.__vp_size[1])
        width, height = self.__vp_size

        glClearColor(0.0, 0.0, 0.0, 1.0)
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)
          
        glDispatchCompute(width, height, 1)
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT)
        #glMemoryBarrier(GL_ALL_BARRIER_BITS); 
        glBlitFramebuffer(0, 0, width, height, 0, 0, width, height, GL_COLOR_BUFFER_BIT, GL_LINEAR)
        
        glutSwapBuffers()
        glutPostRedisplay()


window = MyWindow(512, 512)
window.run()