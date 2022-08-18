from OpenGL.GL import *
from OpenGL.GLU import *
import math, time
import tkinter
import tkinter.ttk
import pyopengltk

v = [[-1,-1,1], [1,-1,1], [1,1,1], [-1,1,1], [-1,-1,-1], [1,-1,-1], [1,1,-1], [-1,1,-1]]
#t = [[0, 1], [1, 1], [1, 0], [0, 0]]
n = [[0,0,1], [1,0,0], [0,0,-1], [-1,0,0], [0,1,0], [0,-1,0]]
c = [(1, 0, 0), (0, 1, 0), (0, 0, 1), (1, 1, 0), (1, 0, 1), (0, 1, 1)]
e = [[0,1,2,3], [1,5,6,2], [5,4,7,6], [4,0,3,7], [3,2,6,7], [1,0,4,5]]
l = 1/math.sqrt(2)

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
            glDebugMessageInsert(GL_DEBUG_SOURCE_APPLICATION, GL_DEBUG_TYPE_MARKER, 0, GL_DEBUG_SEVERITY_NOTIFICATION, -1, "Starting debug messaging service")

            glEnable(GL_MULTISAMPLE) # default
            glEnable(GL_DEPTH_TEST)
            glClearColor(0.0, 0.0, 0.0, 0.0)

            glEnable(GL_LIGHTING)
            glEnable(GL_LIGHT0)
            glEnable(GL_COLOR_MATERIAL)
            glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE)
            glLightfv(GL_LIGHT0, GL_AMBIENT, (0, 0, 0, 1))
            glLightfv(GL_LIGHT0, GL_DIFFUSE, (1, 1, 1, 1))
            self.start_time = time.time()

        glViewport(0, 0, self.width, self.height)
        glMatrixMode(GL_PROJECTION)
        glLoadIdentity()
        gluPerspective(90, self.width / self.height, 0.1, 10) 
        glMatrixMode(GL_MODELVIEW)  

    def redraw(self):
        elapsed_time = time.time() - self.start_time
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)
    
        glLoadIdentity()
        gluLookAt(0, -3, 0, 0, 0, 0, 0, 0, 1)

        glLight(GL_LIGHT0, GL_POSITION,  (0.5, -1.0, 0.5, 0)) # directional light from the front

        glPushMatrix()
        glRotatef(elapsed_time * 90, 0.5, 0, 1)
        glBegin(GL_QUADS)
        for i, face in enumerate(e):
            glColor3fv(c[i])
            glNormal3fv(n[i])
            for vi in face:
                glVertex3f(v[vi][0]*l, v[vi][1]*l, v[vi][2]*l)
        glEnd()
        glPopMatrix()

if __name__ == '__main__':
    root = tkinter.Tk()
    app = OpenGLApp(root, width=640, height=480)
    app.pack(fill=tkinter.BOTH, expand=tkinter.YES)
    app.animate = 1
    app.after(100, app.printContext)
    app.mainloop()
    exit()
