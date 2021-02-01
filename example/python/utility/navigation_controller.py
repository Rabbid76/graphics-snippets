import math
import glm 

#
# Navigation controller
#
class NavigationController:

    OFF    = 0
    ORBIT  = 1
    ROTATE = 2

    def __init__(self, view_mat, proj_mat, view_rect, depth_val, pivot_pt):

        self.__id = id
        self.__get_view_rect = view_rect
        self.__get_view_mat  = view_mat
        self.__get_proj_mat  = proj_mat
        self.__get_depth_val = depth_val
        self.__get_pivot     = pivot_pt

        self.__pan = False
        self.__pan_start = glm.vec3(0, 0, 1)
        self.__orbit = self.OFF
        self.__orbit_start = glm.vec3(0, 0, 1)
        self.__pivot_world = glm.vec3(0, 0, 0)

    def ProjectionMat(self):
        proj = self.__get_proj_mat()
        return proj, glm.inverse(proj)

    def ViewMat(self):
        view = self.__get_view_mat()
        return view, glm.inverse(view)

    def WindowMat(self):
        vp_rect = self.VpRect()
        inv_wnd = glm.translate(glm.mat4(1), glm.vec3(-1, -1, -1))
        inv_wnd = glm.scale(inv_wnd, glm.vec3(2/vp_rect[2], 2/vp_rect[3], 2))
        inv_wnd = glm.translate(inv_wnd, glm.vec3(vp_rect[0], vp_rect[1], 0))
        return glm.inverse(inv_wnd), inv_wnd

    def VpRect(self):
        return self.__get_view_rect()

    def Depth(self, cursor_pos):
        return self.__get_depth_val(*cursor_pos)

    def PivotWorld(self, cursor_pos):
        return self.__get_pivot(*cursor_pos)

    def StartPan(self, cursor_pos):
        self.__pan = True
        self.__pan_start = glm.vec3(*cursor_pos, self.Depth(cursor_pos))
        
    def EndPan(self, cursor_pos):
        self.__pan = False

    def StartOrbit(self, cursor_pos, mode = ORBIT):
        self.__orbit = mode if mode >= self.ORBIT and mode <= self.ROTATE else self.ORBIT
        self.__orbit_start = glm.vec3(*cursor_pos, self.Depth(cursor_pos))
        self.__pivot_world = self.PivotWorld(cursor_pos)
        
    def EndOrbit(self, cursor_pos):
        self.__orbit = self.OFF

    def MoveOnLineOfSight(self, cursor_pos, delta):

        # get viewport rectangle
        #vp_rect = self.VpRect()
       
        # get view, projection and window matrix
        proj, inv_proj = self.ProjectionMat()
        view, inv_view = self.ViewMat()
        wnd,  inv_wnd  = self.WindowMat() 

        # get world space position on view ray
        pt_wnd     = glm.vec3(*cursor_pos, 1.0)
        #pt_world  = glm.unProject(pt_wnd, view, proj, vp_rect)
        pt_h_world = inv_view * inv_proj * inv_wnd * glm.vec4(*pt_wnd, 1)
        pt_world   = glm.vec3(pt_h_world) / pt_h_world.w
         
        # get view position
        eye  = glm.vec3(inv_view[3])
        
        # get "zoom" direction and amount
        ray_cursor = glm.normalize(pt_world - eye)
        
        # translate view position and update view matrix
        inv_view = glm.translate(glm.mat4(1), ray_cursor * delta) * inv_view
        
        # return new view matrix
        return glm.inverse(inv_view), True

        
    def MoveCursorTo(self, cursor_pos):
        
        view_changed = False 

        # get view matrix and  viewport rectangle
        view, inv_view = self.ViewMat()
        view_rect      = self.VpRect()

        if self.__pan:
 
            # get drag start and end
            wnd_from = self.__pan_start
            wnd_to   = glm.vec3(*cursor_pos, self.__pan_start[2])
            self.__pan_start = wnd_to

            # get projection and window matrix
            proj, inv_proj = self.ProjectionMat()
            wnd,  inv_wnd  = self.WindowMat() 

            # calculate drag start and world coordinates
            pt_h_world = [inv_view * inv_proj * inv_wnd * glm.vec4(*pt, 1) for pt in [wnd_from, wnd_to]]
            pt_world   = [glm.vec3(pt_h) / pt_h.w for pt_h in pt_h_world]
           
            # calculate drag world translation
            world_vec = pt_world[1] - pt_world[0]

            # translate view position and update view matrix
            inv_view     = glm.translate(glm.mat4(1), world_vec * -1) * inv_view
            view         = glm.inverse(inv_view)
            view_changed = True

        elif self.__orbit == self.ORBIT:

            # get the drag start and end
            wnd_from = self.__orbit_start
            wnd_to   = glm.vec3(*cursor_pos, self.__orbit_start[2])
            self.__orbit_start = wnd_to

            # calculate the pivot, rotation axis and angle
            pivot     = glm.vec3(view * glm.vec4(*self.__pivot_world, 1))
            orbit_dir = wnd_to - wnd_from 
            axis  = glm.vec3(-orbit_dir.y, orbit_dir.x, 0)
            angle = glm.length(glm.vec2(orbit_dir.x/(view_rect[2]-view_rect[0]), orbit_dir.y/(view_rect[3]-view_rect[1]))) * math.pi

            # calculate the rotation matrix and the rotation around the pivot 
            rot_mat   = glm.rotate(glm.mat4(1), angle, axis)
            rot_pivot = glm.translate(glm.mat4(1), pivot) * rot_mat * glm.translate(glm.mat4(1), -pivot)
            
            #transform and update view matrix
            view         = rot_pivot * view
            view_changed = True 

        elif self.__orbit == self.ROTATE:

            # get the drag start and end
            wnd_from = self.__orbit_start
            wnd_to   = glm.vec3(*cursor_pos, self.__orbit_start[2])
            self.__orbit_start = wnd_to

            # calculate the pivot, rotation axis and angle
            pivot_view   = glm.vec3(view * glm.vec4(*self.__pivot_world, 1))
            orbit_dir    = wnd_to - wnd_from 

            # get the projection of the up vector to the view port 
            # TODO

            # calculate the rotation components for the rotation around the view space x axis and the world up vector 
            orbit_dir_x  = glm.vec2(0, 1)
            orbit_vec_x  = glm.vec2(0, orbit_dir.y)
            orbit_dir_up = glm.vec2(1, 0)
            orbit_vec_up = glm.vec2(orbit_dir.x, 0)

            # calculate the rotation matrix around the view space x axis through the pivot
            rot_pivot_x = glm.mat4(1)
            if glm.length(orbit_vec_x) > 0.5: 
                axis_x      = glm.vec3(-1, 0, 0)
                angle_x     = glm.dot(orbit_dir_x, glm.vec2(orbit_vec_x.x/(view_rect[2]-view_rect[0]), orbit_vec_x.y/(view_rect[3]-view_rect[1]))) * math.pi
                rot_mat_x   = glm.rotate(glm.mat4(1), angle_x, axis_x)
                rot_pivot_x = glm.translate(glm.mat4(1), pivot_view) * rot_mat_x * glm.translate(glm.mat4(1), -pivot_view)
            
            # calculate the rotation matrix around the world space up vector through the pivot
            rot_pivot_up = glm.mat4(1)
            if glm.length(orbit_vec_up) > 0.5: 
                axis_up      = glm.vec3(0, 0, 1)
                angle_up     = glm.dot(orbit_dir_up, glm.vec2(orbit_vec_up.x/(view_rect[2]-view_rect[0]), orbit_vec_up.y/(view_rect[3]-view_rect[1]))) * math.pi
                rot_mat_up   = glm.rotate(glm.mat4(1), angle_up, axis_up)
                rot_pivot_up = glm.translate(glm.mat4(1), self.__pivot_world) * rot_mat_up * glm.translate(glm.mat4(1), -self.__pivot_world)
            
            #transform and update view matrix
            view         = rot_pivot_x * view * rot_pivot_up
            view_changed = True 

        # return the view matrix
        return view, view_changed