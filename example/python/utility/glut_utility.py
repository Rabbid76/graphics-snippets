import glm 
from OpenGL.GLUT import *
from utility.navigation_controller import NavigationController

class GlutNavigation:
    def __init__(self, cx, cy, view, fov_y, near, far, get_depth_callback):
        
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

        glutReshapeFunc(self.__reshape)
        glutMouseFunc(self.__mouse_button)
        glutMotionFunc(self.__mouse_motion)
        glutPassiveMotionFunc(self.__mouse_passive_motion)
        glutEntryFunc(self.__mouse_entry)
        glutMouseWheelFunc(self.__mouse_wheel)

    def update(self):
        vp_valid = self.__vp_valid
        if not self.__vp_valid:
            self.__vp_size = [glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT)]
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
    
    def __reshape(self, w, h):
        self.__vp_valid = False
        aspect = w/h
        self.__proj = glm.perspective(glm.radians(self.__fov_y), aspect, *self.__depth_range)

    def __mouse_button(self, button, state, x, y ):
        wnd_pos = (x, self.__vp_size[1]-y) 
        if button == GLUT_RIGHT_BUTTON and state == GLUT_DOWN:
            self.__navigate_control.StartPan(wnd_pos)
        elif button == GLUT_RIGHT_BUTTON and state == GLUT_UP:
            self.__navigate_control.EndPan(wnd_pos)
        if button == GLUT_LEFT_BUTTON and state == GLUT_DOWN:
            #self.__navigate_control.StartOrbit(wnd_pos, self.__navigate_control.ORBIT)
            self.__navigate_control.StartOrbit(wnd_pos, self.__navigate_control.ROTATE)
        elif button == GLUT_LEFT_BUTTON and state == GLUT_UP:
            self.__navigate_control.EndOrbit(wnd_pos)

    def __mouse_motion(self, x, y ):
        wnd_pos = (x, self.__vp_size[1]-y)
        self.__view, self.__update_view = self.__navigate_control.MoveCursorTo(wnd_pos) 

    def __mouse_passive_motion(self, x, y ): 
        pass

    def __mouse_entry(self, state):
        pass

    def __mouse_wheel(self, wheel, direction, x, y):        
        wnd_pos = (x, self.__vp_size[1]-y) 
        self.__view, self.__update_view = self.__navigate_control.MoveOnLineOfSight(wnd_pos, direction)