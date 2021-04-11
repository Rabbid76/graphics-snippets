import glm
import tkinter
import tkinter.ttk
from utility.navigation_controller import NavigationController

class TkinterNavigation:
    def __init__(self, opengl_frame, view, fov_y, near, far, get_depth_callback):
        
        self.opengl_frame = opengl_frame
        cx, cy = self.opengl_frame.width, self.opengl_frame.height
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

        self.opengl_frame.bind('<Button-1>', self.__mouse_button_left_down)
        self.opengl_frame.bind('<ButtonRelease-1>', self.__mouse_button_left_up)
        self.opengl_frame.bind('<Button-3>', self.__mouse_button_right_down)
        self.opengl_frame.bind('<ButtonRelease-3>', self.__mouse_button_right_up)
        self.opengl_frame.bind('<Motion>', self.__mouse_motion)
        self.opengl_frame.bind('<MouseWheel>', self.__mouse_wheel)

    def update(self):
        vp_valid = self.__vp_valid
        if not self.__vp_valid:
            self.__vp_size = [self.opengl_frame.width, self.opengl_frame.height]
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
    
    def reshape(self):
        self.__vp_valid = False
        aspect = self.opengl_frame.width / self.opengl_frame.height
        self.__proj = glm.perspective(glm.radians(self.__fov_y), aspect, *self.__depth_range)

    def __mouse_button_left_down(self, event):
        x, y = event.x, event.y
        wnd_pos = (x, self.__vp_size[1]-y) 
        self.__navigate_control.StartOrbit(wnd_pos, self.__navigate_control.ORBIT)
        #self.__navigate_control.StartOrbit(wnd_pos, self.__navigate_control.ROTATE)
    
    def __mouse_button_left_up(self, event):
        x, y = event.x, event.y
        wnd_pos = (x, self.__vp_size[1]-y) 
        self.__navigate_control.EndOrbit(wnd_pos)

    def __mouse_button_right_down(self, event):
        x, y = event.x, event.y
        wnd_pos = (x, self.__vp_size[1]-y) 
        self.__navigate_control.StartPan(wnd_pos)
    
    def __mouse_button_right_up(self, event):
        x, y = event.x, event.y
        wnd_pos = (x, self.__vp_size[1]-y) 
        self.__navigate_control.EndPan(wnd_pos)

    def __mouse_motion(self, event):
        x, y = event.x, event.y
        wnd_pos = (x, self.__vp_size[1]-y)
        self.__view, self.__update_view = self.__navigate_control.MoveCursorTo(wnd_pos) 

    def __mouse_passive_motion(self, x, y ): 
        pass

    def __mouse_entry(self, state):
        pass

    def __mouse_wheel(self, event):
        x, y = event.x, event.y 
        direction = -1 if event.delta < 0 else 1        
        wnd_pos = (x, self.__vp_size[1]-y) 
        self.__view, self.__update_view = self.__navigate_control.MoveOnLineOfSight(wnd_pos, direction)