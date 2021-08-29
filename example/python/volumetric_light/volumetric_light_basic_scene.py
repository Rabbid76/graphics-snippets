import os, sys 
sys.path.append(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..')) 

import math 
import ctypes 
import glm 
from OpenGL.GLUT import *
from OpenGL.GL import *
from OpenGL.GL.shaders import *
from OpenGL.arrays import *
from utility.glut_utility import GlutNavigation
from utility import triangulated_mesh
from utility.opengl_mesh import MultiMesh

sh_shadow_vert = """
#version 460 core

layout (location = 0) in vec3 a_pos;

layout (location = 0) uniform mat4 u_proj; 
layout (location = 1) uniform mat4 u_view; 

layout(std430, binding = 1) buffer Draw
{
    mat4 model[];
} draw;

void main()
{
   gl_Position = u_proj * u_view * draw.model[gl_DrawID] * vec4(a_pos, 1.0);
}
"""

sh_shadow_frag = """
#version 460 core

void main()
{
}
"""


sh_phong_vert = """
#version 460 core

layout (location = 0) in vec3 a_pos;
layout (location = 1) in vec3 a_nv;
layout (location = 2) in vec3 a_uvw;

out vec3 v_pos;
out vec3 v_nv;
out vec3 v_uvw;
out vec4 v_shadow_pos;

layout (location = 0) uniform mat4 u_proj; 
layout (location = 1) uniform mat4 u_view; 
layout (location = 2) uniform mat4 u_shadow_proj; 
layout (location = 3) uniform mat4 u_shadow_view; 

layout(std430, binding = 1) buffer Draw
{
    mat4 model[];
} draw;

void main()
{
    mat4 model      = draw.model[gl_DrawID];
    mat3 normal     = transpose(inverse(mat3(model)));
    vec4 world_pos  = model * vec4(a_pos.xyz, 1.0);

    v_pos        = world_pos.xyz;
    v_nv         = normal * a_nv; 
    v_uvw        = a_uvw; 
    v_shadow_pos = u_shadow_proj * u_shadow_view * world_pos;
    gl_Position  = u_proj * u_view * world_pos;
}
"""

sh_phong_frag = """
#version 460 core

out vec4 frag_color;
in  vec3 v_pos;
in  vec3 v_nv;
in  vec3 v_uvw;
in  vec4 v_shadow_pos;

layout (location = 1) uniform mat4 u_view;
layout (location = 4) uniform vec4 u_light_pos; 
layout (location = 5) uniform vec4 u_light_dir; 
//layout(binding = 1)  uniform sampler2D u_shadow_depth;
layout(binding = 1)  uniform sampler2DShadow u_shadow_depth; 

vec3 HUEtoRGB(in float H)
{
    float R = abs(H * 6.0 - 3.0) - 1.0;
    float G = 2.0 - abs(H * 6.0 - 2.0);
    float B = 2.0 - abs(H * 6.0 - 4.0);
    return clamp( vec3(R,G,B), 0.0, 1.0 );
}

void main()
{
    vec3 camera_pos = inverse(u_view)[3].xyz;

    float shadow = 0.0;
    if (-v_shadow_pos.w < v_shadow_pos.z && v_shadow_pos.z < v_shadow_pos.w)
    {
        vec3 shadow_pos = v_shadow_pos.xyz / v_shadow_pos.w * 0.5 + 0.5;
        //float shadow_depth = texture(u_shadow_depth, shadow_pos.st).x;
        //if (shadow_depth >= shadow_pos.z)
        //    shadow = 1.0;
        shadow = texture(u_shadow_depth, shadow_pos.xyz);
    }

    vec4  color = vec4(HUEtoRGB(v_uvw.z), 1.0);
    vec3  L     = normalize(u_light_pos.xyz - v_pos);
    vec3  N     = normalize(v_nv);
    vec3  V     = normalize(camera_pos - v_pos);
    vec3  H     = normalize(V + L);
    float ka    = 0.1;
    float cone  = dot(normalize(u_light_dir.xyz), -L);
    float cull  = step(u_light_dir.w, cone) * step(0, dot(N, V)) * shadow;
    float kd    = max(0.0, dot(N, L)) * 0.9 * cull;
    float NdotH = max(0.0, dot(N, H));
    float sh    = 100.0;
    float ks    = pow(NdotH, sh) * 0.1 * cull;
    frag_color  = vec4(color.rgb * (ka + kd + ks), color.a);
}
"""        

def mainloop():
    vp_valid, vp_size, view, projection = glut_navigation.update()
    
    angle1 = elapsed_ms() * math.pi * 2 / 5000.0
    angle2 = elapsed_ms() * math.pi * 2 / 7333.0
    model_matrices = []
    for i in range(no_of_meshes):
        if i == 0:
            model = glm.mat4(1)
            model = glm.translate(model, glm.vec3(0, 0, -1))
        else:
            angleY = angle1 + math.pi*2 * (i-1) / (no_of_meshes-1)
            model = glm.mat4(1)
            model = glm.rotate(model, angleY, glm.vec3(0, 0, 0.5))
            model = glm.translate(model, glm.vec3(diameter/2, 0, 0.5))
            model = glm.rotate(model, angle2, glm.vec3(0, 1, 0))
        model_matrices.append(model)

    light_pos = glm.vec3(0, 0, 3)
    light_dir = glm.vec3(0, 0, -1)
    light_cone_angle_degree = 60
    light_cone_cos = math.cos(math.radians(light_cone_angle_degree) / 2)
    shadow_proj = glm.perspective(glm.radians(light_cone_angle_degree + 1), 1, 0.1, 100.0)
    shadow_view = glm.lookAt(light_pos, light_pos+light_dir, glm.vec3(0,1,0))

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, model_ssbo)
    for i, model in enumerate(model_matrices):
        glBufferSubData(GL_SHADER_STORAGE_BUFFER, i*4*16, glm.sizeof(glm.mat4), glm.value_ptr(model)) 

    glBindFramebuffer(GL_FRAMEBUFFER, shadow_fbo)
    glViewport(0, 0, *shadow_buffer_size)
    glClear(GL_DEPTH_BUFFER_BIT)

    glUseProgram(shadow_program)
    glUniformMatrix4fv(0, 1, GL_FALSE, glm.value_ptr(shadow_proj))
    glUniformMatrix4fv(1, 1, GL_FALSE, glm.value_ptr(shadow_view))

    glEnable(GL_POLYGON_OFFSET_FILL)
    glPolygonOffset(1.0, 1.0)        
    model_multimesh.draw()
    glDisable(GL_POLYGON_OFFSET_FILL)

    glBindFramebuffer(GL_FRAMEBUFFER, 0)
    glViewport(0, 0, *vp_size)
    glClearColor(0.2, 0.3, 0.3, 1.0)
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)
    
    glUseProgram(phong_program)
    glUniformMatrix4fv(0, 1, GL_FALSE, glm.value_ptr(projection))
    glUniformMatrix4fv(1, 1, GL_FALSE, glm.value_ptr(view))
    glUniformMatrix4fv(2, 1, GL_FALSE, glm.value_ptr(shadow_proj))
    glUniformMatrix4fv(3, 1, GL_FALSE, glm.value_ptr(shadow_view))
    glUniform4fv(4, 1, glm.value_ptr(glm.vec4(light_pos, 1)))
    glUniform4fv(5, 1, glm.value_ptr(glm.vec4(light_dir, light_cone_cos)))
    
    glBindTextureUnit(1, shadow_depth_to)
    model_multimesh.draw()
    
    glutSwapBuffers()
    glutPostRedisplay()

glutInit()
glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH)
glutInitWindowSize(800, 600)
glut_window = glutCreateWindow('OpenGL Window')

mesh_defs = [
    #triangulated_mesh.Tetrahedron(),
    #triangulated_mesh.Cube(),
    #triangulated_mesh.Icosahedron(),
    #triangulated_mesh.Tube(),
    #triangulated_mesh.Cone(),
    #triangulated_mesh.SphereSlice(),
    #triangulated_mesh.SphereTessellated(),
    #triangulated_mesh.Torus(),
    triangulated_mesh.TrefoilKnot(),
    #triangulated_mesh.TorusKnot(),
    #triangulated_mesh.Arrow()
]

distance = 2.5
diameter = 0 if len(mesh_defs) <= 1 else distance * len(mesh_defs) / math.pi
camera_distance = 4 if len(mesh_defs) <= 1 else diameter

mesh_defs = [triangulated_mesh.Quad(diameter+6)] + mesh_defs
no_of_meshes = len(mesh_defs)

glut_navigation = GlutNavigation(
    glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT),
    glm.lookAt(glm.vec3(0,-camera_distance*0.75,camera_distance*0.75), glm.vec3(0, 0, 0), glm.vec3(0,0,1)),
    90, 0.1, 20,
    lambda x, y : glReadPixels(x, y, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT))

shadow_program = compileProgram( 
    compileShader(sh_shadow_vert, GL_VERTEX_SHADER),
    compileShader(sh_shadow_frag, GL_FRAGMENT_SHADER),
)

phong_program = compileProgram( 
    compileShader(sh_phong_vert, GL_VERTEX_SHADER),
    compileShader(sh_phong_frag, GL_FRAGMENT_SHADER),
)

model_ssbo = glGenBuffers(1)
glBindBuffer(GL_SHADER_STORAGE_BUFFER, model_ssbo)
glBufferData(GL_SHADER_STORAGE_BUFFER, 4*16*len(mesh_defs), None, GL_STATIC_DRAW)
glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, model_ssbo)

model_multimesh = MultiMesh(mesh_defs, *mesh_defs[0].format)

shadow_buffer_size = (1024, 1024)
shadow_depth_to = glGenTextures(1)
glBindTexture(GL_TEXTURE_2D, shadow_depth_to)
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR)
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR)
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP)
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP)
glTexParameteri(GL_TEXTURE_2D, GL_DEPTH_STENCIL_TEXTURE_MODE, GL_DEPTH_COMPONENT)
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL)
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE)
glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, *shadow_buffer_size, 0, GL_DEPTH_COMPONENT, GL_FLOAT, None)

shadow_fbo = glGenFramebuffers(1)
glBindFramebuffer(GL_FRAMEBUFFER, shadow_fbo)
glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadow_depth_to, 0)
frameBufferStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER)
if frameBufferStatus != GL_FRAMEBUFFER_COMPLETE:
    print('frame buffer incomplete: {}'.format(frameBufferStatus) )
    sys.exit()
glBindFramebuffer(GL_FRAMEBUFFER, 0)

glEnable(GL_DEPTH_TEST)

def elapsed_ms():
    return glutGet(GLUT_ELAPSED_TIME) - starttime
starttime = glutGet(GLUT_ELAPSED_TIME)

glutDisplayFunc(mainloop)
glutMainLoop()
