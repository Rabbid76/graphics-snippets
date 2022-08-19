import glm 
from glfw.GLFW import *
from .navigation_controller import NavigationController

class GlfwNavigation:
    def __init__(self, glfw_window, cx, cy, view, fov_y, near, far, get_depth_callback):
        
        self.__glfw_window = glfw_window
        self.__vp_valid = False
        self.__vp_size = (cx, cy)
        self.__fov_y = fov_y
        self.__depth_range = [near, far]
        self.__view = view
        self.__proj = glm.perspective(glm.radians(self.__fov_y), cx/cy, *self.__depth_range)

        self.__navigate_control = NavigationController(
            lambda : self.__view,
            lambda : self.__proj,
            lambda : glm.vec4(0, 0, *self.__vp_size),
            lambda x, y : self.__get_depth(x, y),
            lambda _, __ : glm.vec3(0, 0, 0) )
        self.__get_depth_callback = get_depth_callback

        glfwSetWindowSizeCallback(self.__glfw_window, self.__size_callback)  
        glfwSetMouseButtonCallback(self.__glfw_window, self.__mouse_button_callback)
        glfwSetCursorPosCallback(self.__glfw_window, self.__cursor_position_callback)
        glfwSetScrollCallback(self.__glfw_window, self.__scroll_callback)
        glfwSetCursorEnterCallback(self.__glfw_window, self.__cursor_enter_callback)

    def update(self):
        vp_valid = self.__vp_valid
        if not self.__vp_valid:
            self.__vp_size = glfwGetFramebufferSize(self.__glfw_window)
            self.__vp_valid = True
        return vp_valid, self.__vp_size, self.__view, self.__proj 

    def __get_depth(self, x, y):
        depth_buffer = self.__get_depth_callback(x, self.__vp_size[1]-y)    
        depth = float(depth_buffer[0][0])
        if depth == 1:
            pt_drag  = glm.vec3(0, 0, 0)
            clip_pos = self.__proj * self.__view * glm.vec4(pt_drag, 1)
            ndc_pos  = glm.vec3(clip_pos) / clip_pos.w
            if ndc_pos.z > -1 and ndc_pos.z < 1:
                depth = ndc_pos.z * 0.5 + 0.5
        return depth
    
    def __size_callback(self, window, w, h):
        self.__vp_valid = False
        aspect = w/h
        self.__proj = glm.perspective(glm.radians(self.__fov_y), aspect, *self.__depth_range)

    def __mouse_button_callback(self, window, button, action, mods):
        x, y = glfwGetCursorPos(window)
        wnd_pos = (x, self.__vp_size[1]-y) 
        if button == GLFW_MOUSE_BUTTON_RIGHT and action == GLFW_PRESS:
            self.__navigate_control.StartPan(wnd_pos)
        elif button == GLFW_MOUSE_BUTTON_RIGHT and action == GLFW_RELEASE:
            self.__navigate_control.EndPan(wnd_pos)
        if button == GLFW_MOUSE_BUTTON_LEFT and action == GLFW_PRESS:
            #self.__navigate_control.StartOrbit(wnd_pos, self.__navigate_control.ORBIT)
            self.__navigate_control.StartOrbit(wnd_pos, self.__navigate_control.ROTATE)
        elif button == GLFW_MOUSE_BUTTON_LEFT and action == GLFW_RELEASE:
            self.__navigate_control.EndOrbit(wnd_pos)

    def __cursor_position_callback(self, window, x, y):
        wnd_pos = (x, self.__vp_size[1]-y)
        self.__view, self.__update_view = self.__navigate_control.MoveCursorTo(wnd_pos) 

    def __scroll_callback(self, window, xoffset, yoffset):  
        x, y = glfwGetCursorPos(window)      
        wnd_pos = (x, self.__vp_size[1]-y) 
        self.__view, self.__update_view = self.__navigate_control.MoveOnLineOfSight(wnd_pos, yoffset)

    def __cursor_enter_callback(self, window, entered):
        pass