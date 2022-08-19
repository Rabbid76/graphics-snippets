from OpenGL.GL import *
import glm, math
from glfw.GLFW import *
import opengl_utility.context, opengl_utility.mesh, opengl_utility.shader

sh_vert = """
#version 410 core

layout (location = 0) in vec4 a_position;
layout (location = 1) in vec3 a_normal;
layout (location = 2) in vec3 a_uvw;
out vec3 v_pos;
out vec3 v_nv;
out vec3 v_uvw;
uniform mat4 u_projection;
uniform mat4 u_view;
uniform mat4 u_model;

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
#version 410 core

in vec3 v_pos;
in vec3 v_nv;
in vec3 v_uvw;
out vec4 frag_color;
uniform mat4 u_view;
uniform vec3 u_light_direction = vec3(0.0, 1.0, 0.0); 
uniform vec3 u_light_intensity = vec3(0.5);

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

class CubeSpecification:
    def __init__(self):
        v = [[-1,-1,1], [1,-1,1], [1,1,1], [-1,1,1], [-1,-1,-1], [1,-1,-1], [1,1,-1], [-1,1,-1]]
        t = [[0, 1], [1, 1], [1, 0], [0, 0]]
        n = [[0,0,1], [1,0,0], [0,0,-1], [-1,0,0], [0,1,0], [0,-1,0]]
        e = [[0,1,2,3], [1,5,6,2], [5,4,7,6], [4,0,3,7], [3,2,6,7], [1,0,4,5]]
        l = 1/math.sqrt(2)
        self.indices = [si*4+[0, 1, 2, 0, 2, 3][vi] for si in range(6) for vi in range(6)]
        self.attributes = []
        for si in range(len(e)):
            for qi, vi in enumerate(e[si]):
                self.attributes += [v[vi][0]*l, v[vi][1]*l,v[vi][2]*l, *n[si], *t[qi], si/6]
        format = [['vertex', 3], ['normal', 3], ['uvw', 3]]
        self.format = sum([info[1] for info in format]), format

class TestScene:
    def __init__(self):
        pass

    def init(self):
        opengl_utility.context.print_context_information()

        self.mesh = opengl_utility.mesh.SingleMesh(CubeSpecification(), {'vertex': 0, 'normal': 1, 'uvw': 2})

        program_obj = opengl_utility.shader.compile_shader(sh_vert, sh_frag)
        self.uniform_locations = [glGetUniformLocation(program_obj, name) for name in ['u_projection', 'u_view', 'u_model', 'u_light_direction', 'u_light_intensity']]   
        glUseProgram(program_obj)
        glUniform3fv(self.uniform_locations[3], 1, [-0.5, 1.0, -0.5])
        glUniform3f(self.uniform_locations[4], 0.2, 0.8, 0.4)

        glEnable(GL_MULTISAMPLE) # default
        glEnable(GL_DEPTH_TEST)
        glClearColor(0.0, 0.0, 0.0, 0.0)

    def draw(self, elapsed_time, view_matrix, projection_matrix, model_matrix):
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)
        glUniformMatrix4fv(self.uniform_locations[0], 1, GL_FALSE, glm.value_ptr(projection_matrix))
        glUniformMatrix4fv(self.uniform_locations[1], 1, GL_FALSE, glm.value_ptr(view_matrix))
        glUniformMatrix4fv(self.uniform_locations[2], 1, GL_FALSE, glm.value_ptr(model_matrix))
        self.mesh.draw()