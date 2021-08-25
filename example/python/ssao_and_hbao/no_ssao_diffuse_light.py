from OpenGL.GL import *
from glfw.GLFW import *
import OpenGL.GL.shaders
import numpy as np
from ctypes import c_void_p
import glm
import math
from wavefrontloader import *
from test_scene import *
from glfw_navigate import *
import os
os.chdir(os.path.join(os.path.dirname(os.path.abspath(__file__)), '../../../resource'))

if glfwInit() == GLFW_FALSE:
    exit()

glfwWindowHint(GLFW_SAMPLES, 8)
window = glfwCreateWindow(800, 600, "OpenGL Window", None, None)
glfwMakeContextCurrent(window)

scene = TestScene('./model/wavefront', False)
scene.create()
navigate = Navigation(window, glm.vec3(0, -0.5, -3.0), "no_ssao_diffuse_light")

glEnable(GL_MULTISAMPLE)
glClearColor(0.0, 0.0, 0.0, 0.0)

while not glfwWindowShouldClose(window):
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)
    scene.draw(navigate.view_matrix, navigate.projection_matrix)
    glfwSwapBuffers(window)
    glfwPollEvents()
    navigate.handle_post_rehresh_actions()

glfwTerminate()
exit()
   

