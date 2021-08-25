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
layout (binding = 2) uniform sampler2D u_normalSampler;
layout (binding = 3) uniform sampler2D u_ssaoKernelSampler;
layout (binding = 4) uniform sampler2D u_ssaoNoiseSampler;
layout (location = 1) uniform vec2 u_viewportsize; 
layout (location = 2) uniform float u_radius = 0.01;
out vec4 frag_color;

void main() 
{
    float ssaoRadius = u_radius;
    float ssaoFalloff = 0.0001;
    float ssaoArea = 1.0;
    float ssaoBase = 0.0;
    float ssaoStrength = 1.0;
    vec2  uvVarying = gl_FragCoord.xy / u_viewportsize;
    float depth = texture(u_depthSampler, uvVarying).x;
    float ambientOcclusion = 1.0;
    float alpha = 0.0;
    if (depth > 0.0)
    {
        vec3 random = texture(u_ssaoNoiseSampler, uvVarying.st * u_viewportsize.xy / 4.0).xyz;
        vec3 position = vec3(uvVarying, depth);
        vec3 normal = texture(u_normalSampler, uvVarying).xyz;
        float radiusDepth = ssaoRadius/depth;
        float occlusion = 0.0;
        int kernelSize = textureSize(u_ssaoKernelSampler, 0).x;
        for(int i=0; i < kernelSize; i++)
        {
            vec3 sample_vec = texelFetch(u_ssaoKernelSampler, ivec2(i, 0), 0).xyz;
            vec3 ray = radiusDepth * reflect(sample_vec, random);
            vec3 hemiRay = position + sign(dot(ray, normal)) * ray;
    
            float occDepth = texture(u_depthSampler, clamp(hemiRay.xy, 0.0, 1.0)).x;
            float difference = depth - occDepth;
    
            occlusion += step(ssaoFalloff, difference) * (1.0 - smoothstep(ssaoFalloff, ssaoArea, difference));
    
            // float rangeCheck = abs(difference) < radiusDepth ? 1.0 : 0.0;
            // occlusion += (occDepth <= position.z ? 1.0 : 0.0) * rangeCheck;
        }
    
        float ao = 1.0 - ssaoStrength * occlusion * (1.0 / float(kernelSize));

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

scene_fbo = FrameBuffer([(GL_RGBA, GL_RGB8), (GL_RGB, GL_RGB16_SNORM)], True, GL_LINEAR)
ssao_fbo = FrameBuffer([(GL_RED, GL_R16_SNORM)], False, GL_LINEAR)
def create_frambuffers(vp_size):
    scene_fbo.create(*vp_size)
    ssao_fbo.create(*vp_size)

def create_noise(noise_size):
    noise = numpy.empty((noise_size * noise_size, 3), dtype = numpy.float32)
    for i1 in range(2):
        for i2 in range(noise_size * noise_size // 2):
            iang = 2*i2+i1
            angle = 2.0 * math.pi * iang / noise_size
            i = i1*noise_size // 2 + i2
            v = glm.normalize(glm.vec3(math.cos(angle), math.sin(angle), 0))
            noise[i,:] = [*v]
    noise_texture = glGenTextures(1)
    glBindTexture(GL_TEXTURE_2D, noise_texture)
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1)
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16_SNORM, noise_size, noise_size, 0, GL_RGB, GL_FLOAT, noise)
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4)          
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST)
    return noise_texture

def create_kernel(kernel_size):
    kernel = numpy.empty((kernel_size * 3), dtype = numpy.float32)
    for i in range(kernel_size):
        v = glm.normalize(glm.vec3(random.uniform(-1, 1), random.uniform(-1, 1), random.uniform(-1, 1)))
        len_xy = glm.length(glm.vec2(v))
        v.z = v.z * (1 + len_xy) - len_xy
        kernel[i,:] = [*v]
    kernel_texture = glGenTextures(1)
    glBindTexture(GL_TEXTURE_2D, kernel_texture)
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16_SNORM, kernel_size, 0, 0, GL_RGB, GL_FLOAT, kernel)          
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST)
    return kernel_texture

if glfwInit() == GLFW_FALSE:
    exit()

glfwWindowHint(GLFW_SAMPLES, 8)
window = glfwCreateWindow(400, 300, "OpenGL Window", None, None)
glfwMakeContextCurrent(window)

ssao_program = OpenGL.GL.shaders.compileProgram(
    OpenGL.GL.shaders.compileShader(sh_ssao_vert, GL_VERTEX_SHADER),
    OpenGL.GL.shaders.compileShader(sh_ssao_frag, GL_FRAGMENT_SHADER)
)

blend_program = OpenGL.GL.shaders.compileProgram(
    OpenGL.GL.shaders.compileShader(sh_blend_vert, GL_VERTEX_SHADER),
    OpenGL.GL.shaders.compileShader(sh_blend_frag, GL_FRAGMENT_SHADER)
)

scene = TestScene('./model/wavefront', True)
scene.create()
navigate = Navigation(window, glm.vec3(0, -0.5, -3.0), "ssao_fixed")
navigate.change_vp_size_callback = create_frambuffers

screensapce_vao = create_screenspace_vao()
noise_texture = create_noise(4)
kernel_texture = create_noise(64)
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
    glBindTexture(GL_TEXTURE_2D, scene_fbo.color_textures[1])
    glActiveTexture(GL_TEXTURE3)
    glBindTexture(GL_TEXTURE_2D, kernel_texture)
    glActiveTexture(GL_TEXTURE4)
    glBindTexture(GL_TEXTURE_2D, noise_texture)
    glUniform2fv(1, 1, navigate.viewport_size)
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
   