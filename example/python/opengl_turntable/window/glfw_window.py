from OpenGL.GL import *
from glfw.GLFW import *
from .glfw_navigation import GlfwNavigation
import glm
from PIL import Image
import time

class GlfwWindow:
    def __init__(self, w, h, caption, callbacks, distance_to_target, look_z, screenshot_prefix):

        if glfwInit() == GLFW_FALSE:
            raise Exception("error: init glfw")

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4)
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1)
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE)
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE)
        glfwWindowHint(GLFW_SAMPLES, 8)
        self.__caption = caption
        self.__glfw_wnd = glfwCreateWindow(w, h, caption, None, None)
        glfwMakeContextCurrent(self.__glfw_wnd)

        glfwSetKeyCallback(self.__glfw_wnd, self.__key_pressed)

        self.__callbacks = callbacks
        self.__distance_to_target = distance_to_target
        self.__glfw_navigation = GlfwNavigation(
            self.__glfw_wnd,
            w, h,
            glm.lookAt(glm.vec3(0, -self.__distance_to_target, look_z), glm.vec3(0, 0, look_z), glm.vec3(0, 0, 1)),
            90, min(0.1, self.__distance_to_target/100), self.__distance_to_target * 2,
            lambda x, y : glReadPixels(x, y, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT))
        self.__auto_rotate = True
        self.__sceen_shot = False
        self.screenshot_prefix = screenshot_prefix
        self.vp_size = [0, 0]

        if callable(getattr(self.__callbacks, 'init', None)):
            self.__callbacks.init()

    def elapsed_ms(self):
        return glfwGetTime() * 1000 - self.__starttime

    def run(self):
        self.__starttime = 0
        self.__starttime = self.elapsed_ms()
        last_time = glfwGetTime()
        frames = 0
        while not glfwWindowShouldClose(self.__glfw_wnd):
            self.__mainloop()
            frames += 1
            if frames % 100 == 0:
                crrent_time = glfwGetTime()
                fps = 100 / (crrent_time - last_time)
                last_time = crrent_time
                glfwSetWindowTitle(self.__glfw_wnd, f"{self.__caption} FPS: {fps:03.03f}")
            glfwSwapBuffers(self.__glfw_wnd)
            glfwPollEvents()
        glfwTerminate()

    def __key_pressed(self, window, key, scancode, action, mods):
        if key == GLFW_KEY_R and action == GLFW_PRESS:
            self.__auto_rotate = not self.__auto_rotate
        elif key == GLFW_KEY_S and action == GLFW_PRESS:
            self.__sceen_shot = True    

    def __mainloop(self):
        vp_valid, self.vp_size, view_matrix, projection_matrix = self.__glfw_navigation.update()
        if not vp_valid:
            glViewport(0, 0, *self.vp_size)
        elapsed_time = self.elapsed_ms() / 1000
        model_matrix = glm.mat4(1)
        if self.__auto_rotate:
            model_matrix = glm.rotate(glm.mat4(1), glm.radians(elapsed_time * 90), glm.vec3(0, 0, 1))
        if callable(getattr(self.__callbacks, 'draw', None)):
            self.__callbacks.draw(elapsed_time, view_matrix, projection_matrix, model_matrix)

        if self.__sceen_shot:
            self.__sceen_shot = False
            image_data = glReadPixels(0, 0, *self.vp_size, GL_RGBA, GL_UNSIGNED_BYTE)
            image = Image.frombytes('RGBA', self.vp_size, image_data).transpose(method=Image.FLIP_TOP_BOTTOM)
            image.save(self.screenshot_prefix + time.strftime("_%Y%m%d_%H%M%S") + '.png', 'PNG')