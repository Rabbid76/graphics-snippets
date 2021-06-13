# OpenGL procedural texture shader explanation
# https://stackoverflow.com/questions/67672873/opengl-procedural-texture-shader-explanation

from OpenGL.GLUT import *
from OpenGL.GLU import *
from OpenGL.GL import *
import OpenGL.GL.shaders
import numpy as np
from ctypes import c_void_p
import glm
import math

sh_vert = """
#version 460 core

layout (location = 0) in vec4 a_position;
layout (location = 1) in vec3 a_normal;
layout (location = 2) in vec2 a_uv;

out vec3 v_pos;
out vec3 v_nv;
out vec2 v_uv;

layout (location = 0) uniform mat4 u_projection;
layout (location = 1) uniform mat4 u_view;
layout (location = 2) uniform mat4 u_model;

void main() 
{
    v_pos = vec3(u_model * a_position);
    v_nv = inverse(transpose(mat3(u_model))) * a_normal;
    v_uv = a_uv;
    gl_Position = u_projection * u_view * u_model * a_position;
}
"""

sh_frag = """
#version 460 core

out vec4 frag_color;

in vec3 v_pos;
in vec3 v_nv;
in vec2 v_uv;

layout (location = 10) uniform vec3 color1;
layout (location = 11) uniform vec3 color2;
layout (location = 12) uniform float frequency;
layout (location = 13) uniform float noiseScale;
layout (location = 14) uniform float ringScale;
layout (location = 15) uniform float contrast;

vec3 mod289(vec3 x) {
    return x - floor(x * (1.0 / 289.0)) * 289.0;
}

vec4 mod289(vec4 x) {
    return x - floor(x * (1.0 / 289.0)) * 289.0;
}

vec4 permute(vec4 x) {
    return mod289(((x*34.0)+1.0)*x);
}

vec4 taylorInvSqrt(vec4 r) {
    return 1.79284291400159 - 0.85373472095314 * r;
}

float snoise(vec3 v) {
    const vec2  C = vec2(1.0/6.0, 1.0/3.0) ;
    const vec4  D = vec4(0.0, 0.5, 1.0, 2.0);

    // First corner
    vec3 i  = floor(v + dot(v, C.yyy) );
    vec3 x0 =   v - i + dot(i, C.xxx) ;

    // Other corners
    vec3 g = step(x0.yzx, x0.xyz);
    vec3 l = 1.0 - g;
    vec3 i1 = min( g.xyz, l.zxy );
    vec3 i2 = max( g.xyz, l.zxy );

    //   x0 = x0 - 0.0 + 0.0 * C.xxx;
    //   x1 = x0 - i1  + 1.0 * C.xxx;
    //   x2 = x0 - i2  + 2.0 * C.xxx;
    //   x3 = x0 - 1.0 + 3.0 * C.xxx;
    vec3 x1 = x0 - i1 + C.xxx;
    vec3 x2 = x0 - i2 + C.yyy; // 2.0*C.x = 1/3 = C.y
    vec3 x3 = x0 - D.yyy;      // -1.0+3.0*C.x = -0.5 = -D.y

    // Permutations
    i = mod289(i);
    vec4 p = permute( permute( permute(
                    i.z + vec4(0.0, i1.z, i2.z, 1.0 ))
                + i.y + vec4(0.0, i1.y, i2.y, 1.0 ))
            + i.x + vec4(0.0, i1.x, i2.x, 1.0 ));

    // Gradients: 7x7 points over a square, mapped onto an octahedron.
    // The ring size 17*17 = 289 is close to a multiple of 49 (49*6 = 294)
    float n_ = 0.142857142857; // 1.0/7.0
    vec3  ns = n_ * D.wyz - D.xzx;

    vec4 j = p - 49.0 * floor(p * ns.z * ns.z);  //  mod(p,7*7)

    vec4 x_ = floor(j * ns.z);
    vec4 y_ = floor(j - 7.0 * x_ );    // mod(j,N)

    vec4 x = x_ *ns.x + ns.yyyy;
    vec4 y = y_ *ns.x + ns.yyyy;
    vec4 h = 1.0 - abs(x) - abs(y);

    vec4 b0 = vec4( x.xy, y.xy );
    vec4 b1 = vec4( x.zw, y.zw );

    vec4 s0 = floor(b0)*2.0 + 1.0;
    vec4 s1 = floor(b1)*2.0 + 1.0;
    vec4 sh = -step(h, vec4(0.0));

    vec4 a0 = b0.xzyw + s0.xzyw*sh.xxyy ;
    vec4 a1 = b1.xzyw + s1.xzyw*sh.zzww ;

    vec3 p0 = vec3(a0.xy,h.x);
    vec3 p1 = vec3(a0.zw,h.y);
    vec3 p2 = vec3(a1.xy,h.z);
    vec3 p3 = vec3(a1.zw,h.w);

    // Normalise gradients
    vec4 norm = taylorInvSqrt(vec4(dot(p0,p0), dot(p1,p1), dot(p2, p2), dot(p3,p3)));
    p0 *= norm.x;
    p1 *= norm.y;
    p2 *= norm.z;
    p3 *= norm.w;

    // Mix final noise value
    vec4 m = max(0.6 - vec4(dot(x0,x0), dot(x1,x1), dot(x2,x2), dot(x3,x3)), 0.0);
    m = m * m;
    return 42.0 * dot( m*m, vec4( dot(p0,x0), dot(p1,x1),
                dot(p2,x2), dot(p3,x3) ) );
}

void main() {
    //texture change on movement
    //float n = snoise(v_pos);
   
    //texture fixed on movement
    float n = snoise(vec3(v_uv.x,-0.68, v_uv.y));
    
    float ring = fract(frequency * v_uv.y + noiseScale * n);
    ring *= contrast * (1.0 - ring);
    
    // Adjust ring smoothness and shape, and add some noise
    float lerp = pow(ring, ringScale) + n;
    vec3 base = mix(color1, color2, lerp);
    frag_color = vec4(base, 1.0);
}
"""

def display():
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)
    
    elapsed_ms = glutGet(GLUT_ELAPSED_TIME)

    projection = glm.perspective(glm.radians(60), aspect, 0.1, 20.0)
    view = glm.lookAt(glm.vec3(-1, -8, 4), glm.vec3(-1, 0, -1), glm.vec3(0, 0, 1))
    angle = 0#elapsed_ms * math.pi * 2 / 10000.0
    model = glm.rotate(glm.mat4(1), glm.radians(-30), glm.vec3(0, 0, 1))
    model = glm.rotate(model, angle, glm.vec3(0, 1, 0))
    model = glm.scale(model, glm.vec3(1, 5, 0.2))
    
    glUniformMatrix4fv(0, 1, GL_FALSE, glm.value_ptr(projection))
    glUniformMatrix4fv(1, 1, GL_FALSE, glm.value_ptr(view))
    glUniformMatrix4fv(2, 1, GL_FALSE, glm.value_ptr(model))

    glUniform3f(10, 50/255, 40/255, 30/255)
    glUniform3f(11, 200/255, 150/255, 100/255)
    glUniform1f(12, 1.0) # frequency
    glUniform1f(13, 10.0) # noiseScale
    glUniform1f(14, 0.1) # ringScale
    glUniform1f(15, 1.0) # contrast

    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, None)

    glutSwapBuffers()
    glutPostRedisplay()

def reshape(width, height):
    global aspect
    glViewport(0, 0, width, height)
    aspect = width / height    

glutInit(sys.argv)
glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH | GLUT_MULTISAMPLE)
glutSetOption(GLUT_MULTISAMPLE, 8)
glutInitWindowSize(640, 480)
glutCreateWindow(b"OpenGL Window")
glutDisplayFunc(display)
glutReshapeFunc(reshape)

vertices = [(-1,-1,-1), (1,-1,-1), (1, 1,-1), (-1, 1,-1), (-1,-1, 1), (1,-1, 1), (1, 1, 1), (-1, 1, 1)]
uv = [(0,0), (1,0), (1,1), (0,1), (0,0), (1,0), (1,1), (0,1)]
faces = [[0,1,2,3], [1,5,6,2], [5,4,7,6], [4,0,3,7], [3,2,6,7], [1,0,4,5]]
normals = [(0,0,-1), (1,0,0), (0,0,1), (-1,0,0), (0,1,0), (0,-1,0)]

attributes = []
indices = []
for si, f in enumerate(faces):
    for i in f:
        attributes.append(list(vertices[i]) + list(normals[si]) + list(uv[i]))
    indices.append([4*si, 4*si+1, 4*si+2, 4*si, 4*si+2, 4*si+3])
attributes = np.array(attributes, dtype=np.float32) 
indices = np.array(indices, dtype=np.uint32)

vao = glGenVertexArrays(1)
vbo = glGenBuffers(1)
ebo = glGenBuffers(1)
glBindVertexArray(vao)
glBindBuffer(GL_ARRAY_BUFFER, vbo)
glBufferData(GL_ARRAY_BUFFER, attributes, GL_STATIC_DRAW)
glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo)
glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices, GL_STATIC_DRAW)
glVertexAttribPointer(0, 3, GL_FLOAT, False, 8 * attributes.itemsize, None)
glVertexAttribPointer(1, 3, GL_FLOAT, False, 8 * attributes.itemsize, c_void_p(3 * attributes.itemsize))
glVertexAttribPointer(2, 2, GL_FLOAT, False, 8 * attributes.itemsize, c_void_p(6 * attributes.itemsize))
glEnableVertexAttribArray(0)
glEnableVertexAttribArray(1)
glEnableVertexAttribArray(2)

program = OpenGL.GL.shaders.compileProgram(
    OpenGL.GL.shaders.compileShader(sh_vert, GL_VERTEX_SHADER),
    OpenGL.GL.shaders.compileShader(sh_frag, GL_FRAGMENT_SHADER)
)
glUseProgram(program)

glEnable(GL_MULTISAMPLE) # default
glEnable(GL_DEPTH_TEST)

glClearColor(0.0, 0.0, 0.0, 0.0)
glutMainLoop()