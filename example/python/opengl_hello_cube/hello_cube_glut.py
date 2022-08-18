from OpenGL.GL import *
from OpenGL.GLUT import *
from ctypes import c_void_p, sizeof
import glm, math

sh_vert = """
#version 460 core

layout (location = 0) in vec4 a_position;
layout (location = 1) in vec3 a_normal;
layout (location = 2) in vec3 a_uvw;
out vec3 v_pos;
out vec3 v_nv;
out vec3 v_uvw;
layout (location = 0) uniform mat4 u_projection;
layout (location = 1) uniform mat4 u_view;
layout (location = 2) uniform mat4 u_model;

void main() 
{
    mat3 normal_matrix = transpose(inverse(mat3(u_model)));
    vec4 world_pos = u_model * a_position;
    v_pos = world_pos.xyz;
    v_nv = normal_matrix * a_normal;
    v_uvw = a_uvw;
    gl_Position = u_projection * u_view * world_pos;
}
"""

sh_frag = """
#version 460 core

in vec3 v_pos;
in vec3 v_nv;
in vec3 v_uvw;
out vec4 frag_color;
layout (location = 1) uniform mat4 u_view;
layout (location = 3) uniform vec3 u_light_direction = vec3(0.0, 1.0, 0.0); 
layout (location = 4) uniform vec3 u_light_intensity = vec3(0.5);

vec3 HUEtoRGB(in float H)
{
    float R = abs(H * 6.0 - 3.0) - 1.0;
    float G = 2.0 - abs(H * 6.0 - 2.0);
    float B = 2.0 - abs(H * 6.0 - 4.0);
    return clamp( vec3(R,G,B), 0.0, 1.0 );
}

void main()
{
    vec4  color = vec4(HUEtoRGB(v_uvw.z), 1.0);
    vec3  L     = -normalize(u_light_direction);
    vec3  eye   = inverse(u_view)[3].xyz;
    vec3  V     = normalize(eye - v_pos);
    float face  = sign(dot(v_nv, V));
    vec3  N     = normalize(v_nv) * face;
    vec3  H     = normalize(V + L);
    float ka    = u_light_intensity[0];
    float kd    = max(0.0, dot(N, L)) * u_light_intensity[1];
    float NdotH = max(0.0, dot(N, H));
    float sh    = 100.0;
    float ks    = pow(NdotH, sh) * u_light_intensity[2];
    frag_color  = vec4(color.rgb * (ka + kd + ks), color.a);
}
"""

def display():
    elapsed_time = (glutGet(GLUT_ELAPSED_TIME) - start_time) / 1000
    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)
    
    projection_matrix = glm.perspective(glm.radians(90), aspect, 0.1, 10)
    view_matrix = glm.lookAt(glm.vec3(0, -3, 0), glm.vec3(0, 0, 0), glm.vec3(0, 0, 1))
    model_matrix = glm.rotate(glm.mat4(1), glm.radians(elapsed_time * 90), glm.vec3(0.5, 0, 1))

    glUniformMatrix4fv(0, 1, GL_FALSE, glm.value_ptr(projection_matrix))
    glUniformMatrix4fv(1, 1, GL_FALSE, glm.value_ptr(view_matrix))
    glUniformMatrix4fv(2, 1, GL_FALSE, glm.value_ptr(model_matrix))
    glDrawElements(GL_TRIANGLES, no_of_indices, GL_UNSIGNED_INT, None)

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
t = [[0, 1], [1, 1], [1, 0], [0, 0]]
n = [[0,0,1], [1,0,0], [0,0,-1], [-1,0,0], [0,1,0], [0,-1,0]]
e = [[0,1,2,3], [1,5,6,2], [5,4,7,6], [4,0,3,7], [3,2,6,7], [1,0,4,5]]
l = 1/math.sqrt(2)
indices = [si*4+[0, 1, 2, 0, 2, 3][vi] for si in range(6) for vi in range(6)]
attributes = []
for si in range(len(e)):
    for qi, vi in enumerate(e[si]):
        attributes += [v[vi][0]*l, v[vi][1]*l,v[vi][2]*l, *n[si], *t[qi], si/6]
attributes = (GLfloat * len(attributes))(*attributes)
indices = (GLuint * len(indices))(*indices)
no_of_indices = len(indices)

vao = glGenVertexArrays(1)
vbo = glGenBuffers(1)
glBindVertexArray(vao)
glBindBuffer(GL_ARRAY_BUFFER, vbo)
glBufferData(GL_ARRAY_BUFFER, attributes, GL_STATIC_DRAW)
glVertexAttribPointer(0, 3, GL_FLOAT, False, 9 * sizeof(GLfloat), None)
glVertexAttribPointer(1, 3, GL_FLOAT, False, 9 * sizeof(GLfloat), c_void_p(3 * sizeof(GLfloat)))
glVertexAttribPointer(2, 3, GL_FLOAT, False, 9 * sizeof(GLfloat), c_void_p(6 * sizeof(GLfloat)))
glEnableVertexAttribArray(0)
glEnableVertexAttribArray(1)
glEnableVertexAttribArray(2)
ebo = glGenBuffers(1)
glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo)
glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices, GL_STATIC_DRAW)

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
glUseProgram(program_obj)
glUniform3fv(3, 1, [-0.5, 1.0, -0.5])
glUniform3f(4, 0.2, 0.8, 0.4)

glEnable(GL_MULTISAMPLE) # default
glEnable(GL_DEPTH_TEST)
glClearColor(0.0, 0.0, 0.0, 0.0)

start_time = glutGet(GLUT_ELAPSED_TIME)
glutMainLoop()
exit()
