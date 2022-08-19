from OpenGL.GL import *
import glm, math
from glfw.GLFW import *
import opengl_utility.context, opengl_utility.mesh, opengl_utility.shader, opengl_utility.texture
from enum import Enum

sh_vert = """
#version 410 core

layout (location = 0) in vec4 a_position;
layout (location = 1) in vec3 a_normal;
layout (location = 2) in vec2 a_uv;
out vec3 v_pos;
out vec3 v_nv;
out vec2 v_uv;
uniform mat4 u_projection;
uniform mat4 u_view;
uniform mat4 u_model;

void main() 
{
    mat3 normal_matrix = transpose(inverse(mat3(u_model)));
    vec4 world_pos = u_model * a_position;
    v_pos = world_pos.xyz;
    v_nv = normal_matrix * a_normal;
    v_uv = a_uv;
    gl_Position = u_projection * u_view * world_pos;
}
"""

sh_frag = """
#version 410 core

in vec3 v_pos;
in vec3 v_nv;
in vec2 v_uv;
out vec4 frag_color;
uniform mat4 u_view;
uniform vec3 u_light_direction = vec3(0.0, 1.0, 0.0); 
uniform vec3 u_light_intensity = vec3(0.5);
uniform sampler2D u_diffuse_texture;

void main()
{
    vec4  color = texture(u_diffuse_texture, v_uv);
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

class ModelOrientation(Enum):
    Z_UP = 0
    Y_UP = 1

class SceneObject:
    def __init__(self, mesh_specification, model_transformation):
        self.mesh = opengl_utility.mesh.SingleMesh(mesh_specification, {'vertex': 0, 'normal': 1, 'uvw': 2})
        self.model_transformation = model_transformation * mesh_specification.model_matrix
        self.texture = opengl_utility.texture.create_texture_from_rgba_color([*mesh_specification.color, 1], 0)

class ModelScene:
    def __init__(self, model_specification, orientation = ModelOrientation.Z_UP):
        self.model_specification = model_specification
        self.orientation = orientation
        self.b_min = self.model_specification.box_min
        self.b_max = self.model_specification.box_max
        self.size = self.b_max - self.b_min    
        if self.orientation == ModelOrientation.Y_UP:
            self.size.y, self.size.z = self.size.z, self.size.y
        self.objects = []

    def init(self):
        opengl_utility.context.print_context_information()
        center = (self.b_min + self.b_max) / 2
        for mesh_specification in self.model_specification.meshs:
            model_matrix = glm.mat4(1)
            if self.orientation == ModelOrientation.Y_UP:
                model_matrix = glm.rotate(model_matrix, math.pi/2, glm.vec3(1, 0, 0))
                model_matrix = glm.translate(model_matrix, glm.vec3(-center.x, -self.b_min.y, -center.z))
            else:
                model_matrix = glm.translate(model_matrix, glm.vec3(-center.x, -center.y, -self.b_min.z))
            self.objects.append(SceneObject(mesh_specification, model_matrix))
        
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
        for object in self.objects:
            model_transformation = model_matrix * object.model_transformation
            glUniformMatrix4fv(self.uniform_locations[2], 1, GL_FALSE, glm.value_ptr(model_transformation))
            glActiveTexture(GL_TEXTURE0)
            glBindTexture(GL_TEXTURE_2D, object.texture)
            object.mesh.draw()