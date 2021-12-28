from OpenGL.GL import *
from glfw.GLFW import *
import OpenGL.GL.shaders
import numpy as np
from ctypes import c_void_p
import glm
import math
import random
from wavefrontloader import *
from test_scene import *
from glfw_navigate import *
from buffers import *
import os
os.chdir(os.path.join(os.path.dirname(os.path.abspath(__file__)), '../../../resource'))

sh_ssao_vert = """
#version 460 core

layout (location = 0) in vec4 a_position;

void main() 
{
    gl_Position = a_position;
}
"""

sh_ssao_frag = """
#version 460 core

layout (binding = 1) uniform sampler2D u_depthSampler;
layout (binding = 2) uniform sampler2D u_ssaoNoiseSampler;
layout (location = 1) uniform vec2 u_viewportsize; 
layout (location = 2) uniform float u_radius = 0.01;
out vec4 frag_color;

float Depth(in sampler2D depthSampler, in vec2 texC)
{
    return texture(depthSampler, texC).x;
}

vec3 GetNormalFromDepth( in float depth, in vec2 vUV )
{    
    vec2 offsetX = vec2(1.0/u_viewportsize.x, 0.0);
    vec2 offsetY = vec2(0.0, 1.0/u_viewportsize.y);

    float depthOffsetX = Depth(u_depthSampler, vUV + offsetX); // Horizontal neighbour
    float depthOffsetY = Depth(u_depthSampler, vUV + offsetY); // Vertical neighbour
    vec3 normal = vec3(0.0);

    vec3 pX = vec3(offsetX, depthOffsetX - depth);
    vec3 pY = vec3(offsetY, depthOffsetY - depth);
    normal = cross(pY, pX);
    normal.z = abs(normal.z); // We want normal.z positive

    return normalize(normal); // [-1,1]
}

void main() 
{
    float ssaoRadius = u_radius;
    float ssaoFalloff = 0.0001;
    float ssaoArea = 1.0;
    float ssaoBase = 0.0;
    float ssaoStrength = 1.0;
    vec2  uvVarying = gl_FragCoord.xy / u_viewportsize;
    float depth = Depth(u_depthSampler, uvVarying);

    float ambientOcclusion = 1.0;
    float alpha = 0.0;
    if (depth > 0.0)
    {
        vec3 sampleSphere[16];
        vec3 random = texture(u_ssaoNoiseSampler, uvVarying.st * u_viewportsize.xy / 4.0).xyz;
        
        vec3 position = vec3(uvVarying, depth);
        vec3 normal = GetNormalFromDepth(depth, uvVarying);

        sampleSphere[0] = vec3( 0.5381, 0.1856,-0.4319);
        sampleSphere[1] = vec3( 0.1379, 0.2486, 0.4430);
        sampleSphere[2] = vec3( 0.3371, 0.5679,-0.0057);
        sampleSphere[3] = vec3(-0.6999,-0.0451,-0.0019);
        sampleSphere[3] = vec3( 0.0689,-0.1598,-0.8547);
        sampleSphere[5] = vec3( 0.0560, 0.0069,-0.1843);
        sampleSphere[6] = vec3(-0.0146, 0.1402, 0.0762);
        sampleSphere[7] = vec3( 0.0100,-0.1924,-0.0344);
        sampleSphere[8] = vec3(-0.3577,-0.5301,-0.4358);
        sampleSphere[9] = vec3(-0.3169, 0.1063, 0.0158);
        sampleSphere[10] = vec3( 0.0103,-0.5869, 0.0046);
        sampleSphere[11] = vec3(-0.0897,-0.4940, 0.3287);
        sampleSphere[12] = vec3( 0.7119,-0.0154,-0.0918);
        sampleSphere[13] = vec3(-0.0533, 0.0596,-0.5411);
        sampleSphere[14] = vec3( 0.0352,-0.0631, 0.5460);
        sampleSphere[15] = vec3(-0.4776, 0.2847,-0.0271);

        float radiusDepth = ssaoRadius/depth;
        float occlusion = 0.0;
        for(int i=0; i < 16; i++)
        {
            vec3 ray = radiusDepth * reflect(sampleSphere[i], random);
            vec3 hemiRay = position + sign(dot(ray, normal)) * ray;
    
            float occDepth = Depth(u_depthSampler, clamp(hemiRay.xy, 0.0, 1.0) );
            float difference = depth - occDepth;
    
            occlusion += step(ssaoFalloff, difference) * (1.0 - smoothstep(ssaoFalloff, ssaoArea, difference));
    
            // float rangeCheck = abs(difference) < radiusDepth ? 1.0 : 0.0;
            // occlusion += (occDepth <= position.z ? 1.0 : 0.0) * rangeCheck;
        }
    
        float ao = 1.0 - ssaoStrength * occlusion * (1.0 / float(16));

        ambientOcclusion = clamp(ao + ssaoBase, 0.0, 1.0);
        alpha = 1.0;

    }
    frag_color = vec4(vec3(ambientOcclusion), alpha);
}
"""

sh_blend_vert = """
#version 460 core

layout (location = 0) in vec4 a_position;

void main() 
{
    gl_Position = a_position;
}
"""

sh_blend_frag = """
#version 460 core

layout (binding = 1) uniform sampler2D u_colorSampler;
layout (binding = 2) uniform sampler2D u_ssaoSampler;
layout (location = 1) uniform vec2 u_viewportsize; 
out vec4 frag_color;

float SSAO44(in sampler2D ssaoSampler, in vec2 texC)
{
    vec2 texOffs = 1.0 / u_viewportsize;
    float ssao = 0.0;  
    for (int inxX = -1; inxX < 3; ++ inxX)
    {
        for (int inxY = -1; inxY < 3; ++ inxY)
            ssao += texture(ssaoSampler, texC.st + texOffs * vec2(inxX, inxY)).x;
    }
    return ssao / 16.0;
}

void main() 
{
    vec2 uv = gl_FragCoord.xy / u_viewportsize;
    vec4 texture_color = texture(u_colorSampler, uv);
    float ssao = SSAO44(u_ssaoSampler, uv);
    frag_color = vec4(texture_color.rgb * ssao, texture_color.a);
}
"""

scene_fbo = FrameBuffer([(GL_RGBA, GL_RGB8)], True, GL_LINEAR)
ssao_fbo = FrameBuffer([(GL_RED, GL_R16_SNORM)], False, GL_LINEAR)
def create_frambuffers(vp_size):
    scene_fbo.create(*vp_size)
    ssao_fbo.create(*vp_size)

def create_noise(noise_size):
    noise = numpy.empty((noise_size * noise_size, 3), dtype = numpy.float32)
    for i in range(noise_size * noise_size):
        v = glm.normalize(glm.vec3(random.uniform(-1, 1), random.uniform(-1, 1), random.uniform(-1, 1)))
        noise[i,:] = [*v]
    noise_texture = glGenTextures(1)
    glBindTexture(GL_TEXTURE_2D, noise_texture)
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1)
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16_SNORM, noise_size, noise_size, 0, GL_RGB, GL_FLOAT, noise)   
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4)      
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST)
    return noise_texture

if glfwInit() == GLFW_FALSE:
    exit()

glfwWindowHint(GLFW_SAMPLES, 8)
window = glfwCreateWindow(800, 600, "OpenGL Window", None, None)
glfwMakeContextCurrent(window)

ssao_program = OpenGL.GL.shaders.compileProgram(
    OpenGL.GL.shaders.compileShader(sh_ssao_vert, GL_VERTEX_SHADER),
    OpenGL.GL.shaders.compileShader(sh_ssao_frag, GL_FRAGMENT_SHADER)
)

blend_program = OpenGL.GL.shaders.compileProgram(
    OpenGL.GL.shaders.compileShader(sh_blend_vert, GL_VERTEX_SHADER),
    OpenGL.GL.shaders.compileShader(sh_blend_frag, GL_FRAGMENT_SHADER)
)

scene = TestScene('./model/wavefront', False)
scene.create()
navigate = Navigation(window, glm.vec3(0, -0.5, -3.0), "ssao_fixed")
navigate.change_vp_size_callback = create_frambuffers

screensapce_vao = create_screenspace_vao()
noise_texture = create_noise(4)
create_frambuffers(navigate.viewport_size)

glEnable(GL_MULTISAMPLE)
glClearColor(0.0, 0.0, 0.0, 0.0)

while not glfwWindowShouldClose(window):
      
    glBindFramebuffer(GL_FRAMEBUFFER, scene_fbo.fbo)
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)

    scene.draw(navigate.view_matrix, navigate.projection_matrix)
    
    glBindFramebuffer(GL_FRAMEBUFFER, ssao_fbo.fbo)
    glClear(GL_COLOR_BUFFER_BIT)

    glUseProgram(ssao_program)
    glActiveTexture(GL_TEXTURE1)
    glBindTexture(GL_TEXTURE_2D, scene_fbo.depth_texture)
    glActiveTexture(GL_TEXTURE2)
    glBindTexture(GL_TEXTURE_2D, noise_texture)
    glUniform2fv(1, 1, navigate.viewport_size)
    glUniform1f(2, 0.01)
    glBindVertexArray(screensapce_vao)
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4)

    glBindFramebuffer(GL_FRAMEBUFFER, 0)
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)

    glUseProgram(blend_program)
    glActiveTexture(GL_TEXTURE1)
    glBindTexture(GL_TEXTURE_2D, scene_fbo.color_textures[0])
    glActiveTexture(GL_TEXTURE2)
    glBindTexture(GL_TEXTURE_2D, ssao_fbo.color_textures[0])
    glUniform2fv(1, 1, navigate.viewport_size)
    glBindVertexArray(screensapce_vao)
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4)
    
    glfwSwapBuffers(window)
    glfwPollEvents()
    navigate.handle_post_rehresh_actions()

glfwTerminate()
exit()
   