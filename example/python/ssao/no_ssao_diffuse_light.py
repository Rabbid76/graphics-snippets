from OpenGL.GL import *
from glfw.GLFW import *
import OpenGL.GL.shaders
import numpy as np
from ctypes import c_void_p
import glm
import math
from wavefrontloader import *
from test_scene import *
import os
os.chdir(os.path.join(os.path.dirname(os.path.abspath(__file__)), '../../../resource'))

drag_start = glm.vec2(0)
drag_vec = glm.vec2(0)
rotate_vec = glm.vec2(0)
drag = False

def mouse_button_callback(window, button, action, mods):
    global drag, drag_start, drag_vec, rotate_vec

    if button != GLFW_MOUSE_BUTTON_LEFT:
        return
    if action == GLFW_PRESS:
        drag = True
        xpos, ypos = glfwGetCursorPos(window)
        drag_start = glm.vec2(xpos, ypos)
    elif action == GLFW_RELEASE:
        drag = False
        rotate_vec += drag_vec
        drag_vec = glm.vec2(0)
        rotate_vec.y = glm.clamp(rotate_vec.y, -1, 1)
        rotate_vec.x, _ = math.modf(rotate_vec.x)

def cursor_position_callback(window, xpos, ypos):
    if drag:
        global drag_vec
        drag_vec = glm.vec2(xpos - drag_start.x, drag_start.y - ypos) / glm.vec2(vp_size[0], vp_size[1]/2)

if glfwInit() == GLFW_FALSE:
    exit()

glfwWindowHint(GLFW_SAMPLES, 8)
vp_size = 800, 600
window = glfwCreateWindow(*vp_size, "OpenGL Window", None, None)
glfwMakeContextCurrent(window)
glfwSetMouseButtonCallback(window, mouse_button_callback)
glfwSetCursorPosCallback(window, cursor_position_callback)

scene = TestScene('./model/wavefront')
scene.create()

glEnable(GL_MULTISAMPLE)
glClearColor(0.0, 0.0, 0.0, 0.0)

start_time_s = glfwGetTime()
while not glfwWindowShouldClose(window):
    current_time_s = glfwGetTime()
    delta_time_s = current_time_s - start_time_s

    vp_size = glfwGetFramebufferSize(window)
    glViewport(0, 0, *vp_size)
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)
    
    projection = glm.perspective(glm.radians(45), vp_size[0]/vp_size[1], 0.01, 10.0)
    view = glm.translate(glm.mat4(1), glm.vec3(0, -0.5, -3.0))
    angle = delta_time_s * math.pi * 2 / 5
    view_rot_vec = drag_vec + rotate_vec
    view_rot_vec.y = glm.clamp(view_rot_vec.y, -1, 1)
    view_rot_vec.x, _ = math.modf(view_rot_vec.x)
    view = view * glm.rotate(glm.rotate(glm.mat4(1), view_rot_vec.y * -math.pi/2, glm.vec3(1, 0, 0)), view_rot_vec.x * math.pi*2, glm.vec3(0, 1, 0))
    
    scene.draw(view, projection)

    glfwSwapBuffers(window)
    glfwPollEvents()

glfwTerminate()
exit()
   

