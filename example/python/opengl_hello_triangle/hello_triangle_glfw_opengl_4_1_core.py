from OpenGL.GL import *
from ctypes import c_void_p, sizeof
import glm
from glfw.GLFW import *

sh_vert = """
#version 410 core

layout (location = 0) in vec4 a_position;
layout (location = 1) in vec4 a_color;
out vec4 v_color;
uniform mat4 u_projection;

void main() 
{
    v_color = a_color;
    gl_Position = u_projection * a_position;
}
"""

sh_frag = """
#version 410 core

out vec4 frag_color;
in vec4 v_color;

void main()
{
    frag_color = v_color;
}
"""

if glfwInit() == GLFW_FALSE:
    raise Exception("error: init glfw")

glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4)
glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1)
glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE)
glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE)
glfwWindowHint(GLFW_SAMPLES, 8)
window = glfwCreateWindow(640, 480, "OpenGL Window", None, None)
glfwMakeContextCurrent(window)

aspect = 640 / 480
def size_callback(window, _w, _h):
    global aspect
    width, height = glfwGetFramebufferSize(window)
    aspect = width / height 
    glViewport(0, 0, width, height)
glfwSetWindowSizeCallback(window, size_callback)   

vendor, renderer = glGetString(GL_VENDOR).decode("utf-8"), glGetString(GL_RENDERER).decode("utf-8")
version, glsl_version = glGetString(GL_VERSION).decode("utf-8"), glGetString(GL_SHADING_LANGUAGE_VERSION).decode("utf-8")
major, minor = glGetInteger(GL_MAJOR_VERSION), glGetInteger(GL_MINOR_VERSION)
extensions = [glGetStringi(GL_EXTENSIONS, i) for i in range(glGetInteger(GL_NUM_EXTENSIONS))]
print(f"\n{vendor} / {renderer}\n  OpenGL: {version}\n  GLSL: {glsl_version}\n  Context {major}.{minor}\n")

attributes = [
#    x       y     z    R  G  B  A
    -0.866, -0.75, 0,   1, 0, 0, 1, 
     0.866, -0.75, 0,   1, 1, 0, 1,
     0,      0.75, 0,   0, 0, 1, 1
]
attributes = (GLfloat * len(attributes))(*attributes)

vao = glGenVertexArrays(1)
vbo = glGenBuffers(1)
glBindVertexArray(vao)
glBindBuffer(GL_ARRAY_BUFFER, vbo)
glBufferData(GL_ARRAY_BUFFER, attributes, GL_STATIC_DRAW)
glVertexAttribPointer(0, 3, GL_FLOAT, False, 7 * sizeof(GLfloat), None)
glVertexAttribPointer(1, 4, GL_FLOAT, False, 7 * sizeof(GLfloat), c_void_p(3 * sizeof(GLfloat)))
glEnableVertexAttribArray(0)
glEnableVertexAttribArray(1)

program_obj = glCreateProgram()
for sh_type, sh_code in [(GL_VERTEX_SHADER, sh_vert), (GL_FRAGMENT_SHADER, sh_frag)]:
    shader_obj = glCreateShader(sh_type)
    glShaderSource(shader_obj, sh_code)
    glCompileShader(shader_obj)
    if not glGetShaderiv(shader_obj, GL_COMPILE_STATUS):
        raise Exception(glGetShaderInfoLog(shader_obj).replace(b'\\n', b'\n'))
    glAttachShader(program_obj, shader_obj)
glLinkProgram(program_obj)
if not glGetProgramiv(program_obj, GL_LINK_STATUS):
    raise Exception(glGetProgramInfoLog(program_obj).replace(b'\\n', b'\n'))
projection_location = glGetUniformLocation(program_obj, "u_projection")    
glUseProgram(program_obj)

glEnable(GL_MULTISAMPLE) # default
glEnable(GL_DEPTH_TEST)
glClearColor(0.0, 0.0, 0.0, 0.0)

while not glfwWindowShouldClose(window):
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)
    if aspect >= 1:
        projection = glm.ortho(-aspect, aspect, -1, 1, -1, 1)
    else:
        projection = glm.ortho(-1, 1, -1/aspect, 1/aspect, -1, 1)  
    glUniformMatrix4fv(projection_location, 1, GL_FALSE, glm.value_ptr(projection))
    glDrawArrays(GL_TRIANGLES, 0, 3)

    glfwSwapBuffers(window)
    glfwPollEvents()

glfwTerminate()
exit()