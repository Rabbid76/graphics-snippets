from OpenGL.GL import *
from OpenGL.GLU import *
import tkinter
import tkinter.ttk
import pyopengltk

attributes = [
#      x       y     z      R  G  B  A
    ((-0.866, -0.75, 0),   (1, 0, 0, 1)), 
    (( 0.866, -0.75, 0),   (1, 1, 0, 1)),
    (( 0,      0.75, 0),   (0, 0, 1, 1))
]

class OpenGLApp(pyopengltk.OpenGLFrame):
    def __init__(self, *args, **kwds):
        super().__init__(*args, kwds) 
        self.__opengl_initialized = False

    def initgl(self):
        if not self.__opengl_initialized:
            self.__opengl_initialized = True
            
            vendor, renderer = glGetString(GL_VENDOR).decode("utf-8"), glGetString(GL_RENDERER).decode("utf-8")
            version, glsl_version = glGetString(GL_VERSION).decode("utf-8"), glGetString(GL_SHADING_LANGUAGE_VERSION).decode("utf-8")
            major, minor = glGetInteger(GL_MAJOR_VERSION), glGetInteger(GL_MINOR_VERSION)
            extensions = [glGetStringi(GL_EXTENSIONS, i) for i in range(glGetInteger(GL_NUM_EXTENSIONS))]
            print(f"\n{vendor} / {renderer}\n  OpenGL: {version}\n  GLSL: {glsl_version}\n  Context {major}.{minor}\n")

            @GLDEBUGPROC
            def __CB_OpenGL_DebugMessage(source, type, id, severity, length, message, userParam):
                msg = message[0:length]
                print(msg.decode("utf-8"))
            glDebugMessageCallback(__CB_OpenGL_DebugMessage, None)
            errors_only = False
            if errors_only:
                glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, None, GL_FALSE)
                glDebugMessageControl(GL_DEBUG_SOURCE_API, GL_DEBUG_TYPE_ERROR, GL_DONT_CARE, 0, None, GL_TRUE)
            else:
                glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, None, GL_TRUE)
            glEnable(GL_DEBUG_OUTPUT)
            glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS)

            glEnable(GL_MULTISAMPLE) # default
            glEnable(GL_DEPTH_TEST)
            glClearColor(0.0, 0.0, 0.0, 0.0)

        glViewport(0, 0, self.width, self.height)
        aspect = self.width / self.height  
        glMatrixMode(GL_PROJECTION)
        glLoadIdentity()
        if aspect >= 1:
            glOrtho(-aspect, aspect, -1, 1, -1, 1)
        else:
            glOrtho(-1, 1, -1/aspect, 1/aspect, -1, 1)   
        glMatrixMode(GL_MODELVIEW)   

    def redraw(self):
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)
        glBegin(GL_TRIANGLES)
        for vertex, color in attributes:
            glColor4f(*color)
            glVertex3f(*vertex)
        glEnd()
   
if __name__ == '__main__':
    root = tkinter.Tk()
    app = OpenGLApp(root, width=640, height=480)
    app.pack(fill=tkinter.BOTH, expand=tkinter.YES)
    app.animate = 1
    app.after(100, app.printContext)
    app.mainloop()
    exit()