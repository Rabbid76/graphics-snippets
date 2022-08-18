from OpenGL.GL import *
from OpenGL.GLU import *
import math
import pygame

pygame.init()
pygame.display.gl_set_attribute(pygame.GL_MULTISAMPLEBUFFERS, 1)
pygame.display.gl_set_attribute(pygame.GL_MULTISAMPLESAMPLES, 8)
window = pygame.display.set_mode((640, 480), pygame.DOUBLEBUF | pygame.OPENGL | pygame.RESIZABLE)
clock = pygame.time.Clock()   

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

v = [[-1,-1,1], [1,-1,1], [1,1,1], [-1,1,1], [-1,-1,-1], [1,-1,-1], [1,1,-1], [-1,1,-1]]
#t = [[0, 1], [1, 1], [1, 0], [0, 0]]
n = [[0,0,1], [1,0,0], [0,0,-1], [-1,0,0], [0,1,0], [0,-1,0]]
c = [(1, 0, 0), (0, 1, 0), (0, 0, 1), (1, 1, 0), (1, 0, 1), (0, 1, 1)]
e = [[0,1,2,3], [1,5,6,2], [5,4,7,6], [4,0,3,7], [3,2,6,7], [1,0,4,5]]
l = 1/math.sqrt(2)

glEnable(GL_MULTISAMPLE) # default
glEnable(GL_DEPTH_TEST)
glClearColor(0.0, 0.0, 0.0, 0.0)

glEnable(GL_LIGHTING)
glEnable(GL_LIGHT0)
glEnable(GL_COLOR_MATERIAL)
glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE)
glLightfv(GL_LIGHT0, GL_AMBIENT, (0, 0, 0, 1))
glLightfv(GL_LIGHT0, GL_DIFFUSE, (1, 1, 1, 1))

start_time = pygame.time.get_ticks()
aspect = 640 / 480
run = True
while run:
    clock.tick(60)
    for event in pygame.event.get():
        if event.type == pygame.QUIT:
            run = False
        elif event.type == pygame.VIDEORESIZE:
            glViewport(0, 0, event.w, event.h)
            aspect = event.w / event.h

    elapsed_time = (pygame.time.get_ticks() - start_time) / 1000
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)

    glMatrixMode(GL_PROJECTION)
    glLoadIdentity()
    gluPerspective(90, aspect, 0.1, 10) 
    glMatrixMode(GL_MODELVIEW)  
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

    pygame.display.flip()

pygame.quit()
exit()
