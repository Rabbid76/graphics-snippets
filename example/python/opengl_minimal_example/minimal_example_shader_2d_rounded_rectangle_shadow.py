# GLSL, SDF based Rounding Rectangle
# https://stackoverflow.com/questions/68241841/glsl-sdf-based-rounding-rectangle
#
# How to create a proper rounded rectangle in WebGL?
# https://stackoverflow.com/questions/68233304/how-to-create-a-proper-rounded-rectangle-in-webgl/68233743#68233743

from OpenGL.GLUT import *
from OpenGL.GLU import *
from OpenGL.GL import *
import OpenGL.GL.shaders
from ctypes import c_void_p
import glm

sh_vert = """
#version 330 core
layout (location = 0) in vec3 a_pos;
layout (location = 1) in vec2 a_tex_cCoord;

out vec2 tex_coord;
uniform mat4 mvp;

void main()
{
    gl_Position = mvp * vec4(a_pos, 1.0);
    tex_coord = a_tex_cCoord;
}
"""

sh_frag = """
#version 330 core

in vec2 tex_coord;
uniform vec2 rect_size;
uniform vec4 border_color = vec4(vec3(0.0), 1.0);
uniform vec4 fill_color = vec4(vec3(0.7), 1.0);
uniform vec4 shadow_color = vec4(vec3(0.0), 1.0);
uniform float border_thickness = 10.0;
uniform float corner_radius = 30.0;
uniform float shadow_size = 30.0; 

float RectSDF(vec2 p, vec2 size, float radius)
{
    return length(max(abs(p) - size + vec2(radius), 0)) - radius;
}

void main() 
{
    vec2 pos = rect_size * tex_coord;
        
    float distance = RectSDF(pos-rect_size/2.0, rect_size/2.0 - shadow_size/2.0-1.0, corner_radius);
    float blend = smoothstep(-1.0, 1.0, abs(distance) - border_thickness/2.0);
    float shadow_blend = smoothstep(-shadow_size/2.0, shadow_size/2.0, abs(distance));
    
    vec4 back_color = mix(mix(shadow_color, vec4(0.0), shadow_blend), fill_color, fill_color.a*step(distance, 0));
    gl_FragColor = mix(border_color, back_color, blend);
}
"""

def display():
    glClear(GL_COLOR_BUFFER_BIT)
    
    glEnable(GL_BLEND)
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA)
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, None)

    glutSwapBuffers()
    glutPostRedisplay()

def reshape(width, height):
    glViewport(0, 0, width, height)
    

resolution = (400, 300)
rect = (50, 50, 350, 250)
attributes = (GLfloat * 20)(*[rect[0],rect[1],0,0,1, rect[2],rect[1],0,1,1, rect[2],rect[3],0,1,0, rect[0],rect[3],0,0,0])
indices = (GLuint * 6)(*[0,1,2, 0,2,3])

glutInit(sys.argv)
glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB)
glutInitWindowSize(*resolution)
glutCreateWindow(b"OpenGL Window")
glutDisplayFunc(display)
glutReshapeFunc(reshape)

vao = glGenVertexArrays(1)
vbo = glGenBuffers(1)
ebo = glGenBuffers(1)
glBindVertexArray(vao)
glBindBuffer(GL_ARRAY_BUFFER, vbo)
glBufferData(GL_ARRAY_BUFFER, attributes, GL_STATIC_DRAW)
glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo)
glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices, GL_STATIC_DRAW)
glVertexAttribPointer(0, 3, GL_FLOAT, False, 5 * 4, None)
glEnableVertexAttribArray(0)
glVertexAttribPointer(1, 2, GL_FLOAT, False, 5 * 4, c_void_p(3 * 4))
glEnableVertexAttribArray(1)

program = OpenGL.GL.shaders.compileProgram(
    OpenGL.GL.shaders.compileShader(sh_vert, GL_VERTEX_SHADER),
    OpenGL.GL.shaders.compileShader(sh_frag, GL_FRAGMENT_SHADER)
)
glUseProgram(program)
mvp = glm.ortho(0, *resolution, 0, -1, 1)
glUniformMatrix4fv(glGetUniformLocation(program, "mvp"), 1, GL_FALSE, glm.value_ptr(mvp))
glUniform2f(glGetUniformLocation(program, "rect_size"), rect[2]-rect[0], rect[3]-rect[1])
glUniform4f(glGetUniformLocation(program, "border_color"), 0, 0.1, 1, 1)
glUniform4f(glGetUniformLocation(program, "fill_color"), 0, 0, 0, 0)
#glUniform4f(glGetUniformLocation(program, "fill_color"), 0.7, 0.7, 0.8, 1)
#glUniform4f(glGetUniformLocation(program, "shadow_color"), 0.8, 0.8, 0.8, 1)
glUniform1f(glGetUniformLocation(program, "border_thickness"), 8)
#glUniform1f(glGetUniformLocation(program, "corner_radius"), 50)
glUniform1f(glGetUniformLocation(program, "shadow_size"), 50)

glClearColor(1, 1, 1, 0)
glutMainLoop()