# How to implement zoom towards mouse like in 3dsMax?
# https://stackoverflow.com/questions/54057549/how-to-implement-zoom-towards-mouse-like-in-3dsmax

import math
from ctypes import c_void_p

import numpy as np
from OpenGL.GL import *
from OpenGL.GLU import *
from OpenGL.GLUT import *
from glm import *
class Camera():

    def __init__(
        self,
        eye=None, target=None, up=None,
        fov=None, near=0.1, far=100000,
        **kwargs
    ):
        self.eye = vec3(eye) or vec3(0, 0, 1)
        self.target = vec3(target) or vec3(0, 0, 0)
        self.up = vec3(up) or vec3(0, 1, 0)
        self.original_up = vec3(self.up)
        self.fov = fov or radians(45)
        self.near = near
        self.far = far

    def update(self, aspect):
        self.view = lookAt(self.eye, self.target, self.up)
        self.projection = perspective(self.fov, aspect, self.near, self.far)

    def zoom(self, *args):
        delta = -args[1] * 0.1
        distance = length(self.target - self.eye)
        self.eye = self.target + (self.eye - self.target) * (delta + 1)

    def zoom_towards_cursor(self, *args):
        x = args[2]
        y = args[3]
        v = glGetIntegerv(GL_VIEWPORT)
        viewport = vec4(float(v[0]), float(v[1]), float(v[2]), float(v[3]))
        height = viewport.z

        p0 = vec3(x, height - y, 0.0)
        p1 = vec3(x, height - y, 1.0)
        v1 = unProject(p0, self.view, self.projection, viewport)
        v2 = unProject(p1, self.view, self.projection, viewport)

        world_from = vec3(
            (-v1.z * (v2.x - v1.x)) / (v2.z - v1.z) + v1.x,
            (-v1.z * (v2.y - v1.y)) / (v2.z - v1.z) + v1.y,
            0.0
        )

        self.eye.z = self.eye.z * (1.0 + 0.1 * args[1])

        view = lookAt(self.eye, self.target, self.up)
        v1 = unProject(p0, view, self.projection, viewport)
        v2 = unProject(p1, view, self.projection, viewport)

        world_to = vec3(
            (v1.z * (v2.x - v1.x)) / (v2.z - v1.z) + v1.x,
            (-v1.z * (v2.y - v1.y)) / (v2.z - v1.z) + v1.y,
            0.0
        )

        offset = world_to - world_from
        print(self.eye.z, world_from, world_to, offset)

        self.eye += offset
        self.target += offset

    def zoom_shift_eye(self, *args):
        x = args[2]
        y = args[3]
        v = glGetIntegerv(GL_VIEWPORT)
        viewport = vec4(float(v[0]), float(v[1]), float(v[2]), float(v[3]))
        width  = viewport.z
        height = viewport.w

        pt_wnd     = vec3(x, height - y, 1.0)
        pt_world   = unProject(pt_wnd, self.view, self.projection, viewport)
        ray_cursor = normalize(pt_world - self.eye)
 
        delta = -args[1]
        self.eye    = self.eye    + ray_cursor * delta
        self.target = self.target + ray_cursor * delta 

    def zoom_frustum(self, *args):
        delta = -args[1] * 0.1
        distance = length(self.target - self.eye)
        self.eye = self.target + (self.eye - self.target) * (delta + 1)


class GlutController():

    def __init__(self, camera):
        self.camera = camera
        self.zoom = self.camera.zoom_shift_eye

    def glut_mouse_wheel(self, *args):
        self.zoom(*args)


class MyWindow:

    def __init__(self, w, h):
        self.width = w
        self.height = h

        glutInit()
        glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH)
        glutInitWindowSize(w, h)
        glutCreateWindow('OpenGL Window')

        self.startup()

        glutReshapeFunc(self.reshape)
        glutDisplayFunc(self.display)
        glutMouseWheelFunc(self.controller.glut_mouse_wheel)
        glutKeyboardFunc(self.keyboard_func)
        glutIdleFunc(self.idle_func)

    def keyboard_func(self, *args):
        try:
            key = args[0].decode("utf8")

            if key == "\x1b":
                glutLeaveMainLoop()

            if key in ['1']:
                self.controller.zoom = self.camera.zoom
                print("Using normal zoom")
            elif key in ['2']:
                self.controller.zoom = self.camera.zoom_towards_cursor
                print("Using zoom towards mouse")
            elif key in ['3']:
                self.controller.zoom = self.camera.zoom_shift
                print("Using shift view position along view ray")
            elif key in ['4']:
                self.controller.zoom = self.camera.zoom_frustum
                print("Using zoom frustum")

        except Exception as e:
            import traceback
            traceback.print_exc()

    def startup(self):
        glEnable(GL_DEPTH_TEST)

        aspect = self.width / self.height
        params = {
            "eye": vec3(10, 10, 10),
            "target": vec3(0, 0, 0),
            "up": vec3(0, 1, 0)
        }
        self.cameras = [
            Camera(**params)
        ]
        self.camera = self.cameras[0]
        self.model = mat4(1)
        self.controller = GlutController(self.camera)

    def run(self):
        glutMainLoop()

    def idle_func(self):
        glutPostRedisplay()

    def reshape(self, w, h):
        glViewport(0, 0, w, h)
        self.width = w
        self.height = h

    def display(self):
        self.camera.update(self.width / self.height)

        glClearColor(0.2, 0.3, 0.3, 1.0)
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)

        glMatrixMode(GL_PROJECTION)
        glLoadIdentity()
        gluPerspective(degrees(self.camera.fov), self.width / self.height, self.camera.near, self.camera.far)
        glMatrixMode(GL_MODELVIEW)
        glLoadIdentity()
        e = self.camera.eye
        t = self.camera.target
        u = self.camera.up
        gluLookAt(e.x, e.y, e.z, t.x, t.y, t.z, u.x, u.y, u.z)
        glColor3f(1, 1, 1)
        glBegin(GL_LINES)
        for i in range(-5, 6):
            if i == 0:
                continue
            glVertex3f(-5, 0, i)
            glVertex3f(5, 0, i)
            glVertex3f(i, 0, -5)
            glVertex3f(i, 0, 5)
        glEnd()

        glBegin(GL_LINES)
        glColor3f(1, 1, 1)
        glVertex3f(-5, 0, 0)
        glVertex3f(0, 0, 0)
        glVertex3f(0, 0, -5)
        glVertex3f(0, 0, 0)

        glColor3f(1, 0, 0)
        glVertex3f(0, 0, 0)
        glVertex3f(5, 0, 0)
        glColor3f(0, 1, 0)
        glVertex3f(0, 0, 0)
        glVertex3f(0, 5, 0)
        glColor3f(0, 0, 1)
        glVertex3f(0, 0, 0)
        glVertex3f(0, 0, 5)
        glEnd()

        glutSwapBuffers()


if __name__ == '__main__':
    window = MyWindow(600, 400)
    #window = MyWindow(800, 600)
    window.run()