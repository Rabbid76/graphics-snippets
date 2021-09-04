# Can't display texture in pyopengl(without opengl context)
# https://stackoverflow.com/questions/69055234/cant-display-texture-in-pyopenglwithout-opengl-context
#
# Vertex/fragment shaders for a OpenGL firsrt-person shooter view?
# https://stackoverflow.com/questions/59896103/vertex-fragment-shaders-for-a-opengl-firsrt-person-shooter-view

from OpenGL.GL import *
from OpenGL.GLU import *
from OpenGL.GLUT import *

vertex_shader = """
void main()
{
    gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
    gl_FrontColor = gl_Color;
}
"""

fragment_shader = """
void main()
{
    gl_FragColor = gl_Color;
}
"""

vertices = [(-1,-1,-1), ( 1,-1,-1), ( 1, 1,-1), (-1, 1,-1), (-1,-1, 1), ( 1,-1, 1), ( 1, 1, 1), (-1, 1, 1)]
faces = [[0,1,2,3], [1,5,6,2], [5,4,7,6], [4,0,3,7], [3,2,6,7], [1,0,4,5]]

def display():
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)

    glRotate(1, 0, 1, 0)
    
    for i, face in enumerate(faces):
        h = i / len(faces)
        r, g, b = abs(h * 6 - 3) - 1, 2 - abs(h * 6 - 2), 2 - abs(h * 6 - 4)
        glColor3f(max(0, min(1, r)), max(0, min(1, g)), max(0, min(1, b)))
        glBegin(GL_POLYGON)
        for vertex in face:
            glVertex3fv(vertices[vertex])
        glEnd()

    glutSwapBuffers()
    glutPostRedisplay()

def reshape(width, height):
    glViewport(0, 0, width, height)
    glMatrixMode(GL_PROJECTION)
    glLoadIdentity()
    gluPerspective(45, width / height, 0.1, 10.0)
    glMatrixMode(GL_MODELVIEW)

glutInit(sys.argv)
glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH | GLUT_MULTISAMPLE)
glutSetOption(GLUT_MULTISAMPLE, 8)
width, height = 400, 400
glutInitWindowSize(width, height)
glutCreateWindow(b"OpenGL Window")
glutReshapeFunc(reshape)
glutDisplayFunc(display)

vs_obj = glCreateShader(GL_VERTEX_SHADER)
glShaderSource(vs_obj, vertex_shader)
glCompileShader(vs_obj)
if not glGetShaderiv(vs_obj, GL_COMPILE_STATUS):
    print(glGetShaderInfoLog(vs_obj))

fs_obj = glCreateShader(GL_FRAGMENT_SHADER)
glShaderSource(fs_obj, fragment_shader)
glCompileShader(fs_obj)
if not glGetShaderiv(fs_obj, GL_COMPILE_STATUS):
    print(glGetShaderInfoLog(fs_obj))

program_obj = glCreateProgram()
glAttachShader(program_obj, vs_obj)
glAttachShader(program_obj, fs_obj)
glLinkProgram(program_obj)
if not glGetProgramiv(program_obj, GL_LINK_STATUS):
    print(glGetProgramInfoLog(program_obj))

glUseProgram(program_obj)

glClearColor(0.0, 0.0, 0.0, 1.0) 
glMatrixMode(GL_MODELVIEW)    
glLoadIdentity()
glTranslate(0, 0, -5)
glEnable(GL_DEPTH_TEST)
glEnable(GL_MULTISAMPLE) # default

glutMainLoop()