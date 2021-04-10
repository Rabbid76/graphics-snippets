import os, sys 
sys.path.append(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..'))

import time
import tkinter
import tkinter.ttk
from pyopengltk import OpenGLFrame
from OpenGL.GL import *
from OpenGL.GL.shaders import *
import math 
import ctypes 
import glm 
from utility import triangulated_mesh
from utility.navigation_controller import NavigationController

def create_window():
    root = tkinter.Tk()
    status_bar_text = tkinter.StringVar()

    control_frame = tkinter.Frame(root, bg='lightgray', padx=4, pady=4)
    status_bar_frame = tkinter.Frame(root, borderwidth=1)
    view_frame = OpenGLView(status_bar_text, root, width=600, height=400)

    root.grid_columnconfigure(0, weight=1)
    root.grid_rowconfigure(0, weight=0)
    root.grid_rowconfigure(1, weight=1)
    root.grid_rowconfigure(2, weight=0)

    control_frame.grid(row=0, column=0, sticky="we")
    view_frame.grid(row=1, column=0, sticky="nwse")
    status_bar_frame.grid(row=2, column=0, sticky="we")

    status_label = tkinter.Label(status_bar_frame, textvariable=status_bar_text, font=("Consolas", 10))
    status_label.grid(column=0, row=0)
    
    control_shape = tkinter.ttk.Combobox(control_frame, state="readonly", font=("Consolas", 10))
    control_shape['values'] = [
        "Tetrahedron", "Hexahedron", "Octahedron", "Dodecahedron", "Icosahedron",
        "Tube", "Cone", "SphereSlice", "SphereTessellated", "Torus", "TrefoilKnot", "TorusKnot", "Arrow"]
    control_shape.current(10)
    view_frame.shape_index = 10
    control_shape.grid(column=0, row=0)
    def shape_changed(event):
        view_frame.shape_index = control_shape.current()
    control_shape.bind('<<ComboboxSelected>>', shape_changed)  

    return root

class OpenGLView(OpenGLFrame):
    def __init__(self, status_text, *args, **kwds):
        super().__init__(*args, kwds) 
        self.shape_index = 0
        self.status_text = status_text
        self.opengl_initialized = False

    def initgl(self): 
        self.times = [time.time()] 
        self.nframes = 0
        self.animate = 10 # milliseconds
        if not self.opengl_initialized:
            self.init_opengl_states_and_objects()
        self.opengl_initialized = True
        self.resize_opengl_viewport()

    def redraw(self):
        self.times += [time.time()]
        if len(self.times) > 100:
            self.times.pop(0)
        fps = (len(self.times)-1) / (self.times[-1] - self.times[0])
        self.nframes += 1
        self.status_text.set(f"fps {fps:.03f} frame: {self.nframes:05d}")
        self.draw_opengl_scene()

    def elapsed_ms(self):
      return (self.times[-1] - self.start_time) * 1000

    def init_opengl_states_and_objects(self):
        self.start_time = time.time()
        self.__glsl_vert = phong_vert
        self.__glsl_frag = phong_frag

        mesh_defs = [
            triangulated_mesh.Tetrahedron(),
            triangulated_mesh.Cube(),
            triangulated_mesh.Octahedron(),
            triangulated_mesh.Dodecahedron(),
            triangulated_mesh.Icosahedron(),
            triangulated_mesh.Tube(),
            triangulated_mesh.Cone(),
            triangulated_mesh.SphereSlice(),
            triangulated_mesh.SphereTessellated(),
            triangulated_mesh.Torus(),
            triangulated_mesh.TrefoilKnot(),
            triangulated_mesh.TorusKnot(),
            triangulated_mesh.Arrow()]

        self.__tkinter_navigation = TkinterNavigation(
            self,
            glm.lookAt(glm.vec3(0,0,2), glm.vec3(0, 0, 0), glm.vec3(0,1,0)),
            90, 0.1, 100,
            lambda x, y : glReadPixels(x, y, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT))

        self.__program = compileProgram( 
            compileShader( self.__glsl_vert, GL_VERTEX_SHADER ),
            compileShader( self.__glsl_frag, GL_FRAGMENT_SHADER ),
        )
        self.___attrib = { a : glGetAttribLocation (self.__program, a) for a in ['a_pos', 'a_nv', 'a_col'] }
        print(self.___attrib)
        self.___uniform = { u : glGetUniformLocation (self.__program, u) for u in ['u_model', 'u_view', 'u_proj'] }
        print(self.___uniform)

        self.__ssbo = glGenBuffers(1)
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, self.__ssbo)
        glBufferData(GL_SHADER_STORAGE_BUFFER, 4*16*len(mesh_defs), None, GL_STATIC_DRAW)
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, self.__ssbo)

        self.__meshes = [SingleMesh(definition) for definition in mesh_defs] 
        self.__multimesh = MultiMesh(mesh_defs, *mesh_defs[0].format)

        glEnable(GL_DEPTH_TEST)
        glUseProgram(self.__program)
        #glPolygonMode(GL_FRONT_AND_BACK, GL_LINE)

    def resize_opengl_viewport(self):
        self.__tkinter_navigation.reshape()
        glViewport(0, 0, self.width, self.height)

    def draw_opengl_scene(self):

        vp_valid, vp_size, view, projection = self.__tkinter_navigation.update()
        if not vp_valid:
            glViewport(0, 0, *vp_size)

        glUniformMatrix4fv(self.___uniform['u_proj'], 1, GL_FALSE, glm.value_ptr(projection))
        glUniformMatrix4fv(self.___uniform['u_view'], 1, GL_FALSE, glm.value_ptr(view))
        
        glClearColor(0.2, 0.3, 0.3, 1.0)
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)

        angle1 = 0 # self.elapsed_ms() * math.pi * 2 / 5000.0
        angle2 = 0 # self.elapsed_ms() * math.pi * 2 / 7333.0
        model = glm.mat4(1)
        model = glm.rotate(model, angle1, glm.vec3(0, 0, 1))
        model = glm.rotate(model, angle2, glm.vec3(0, 1, 0))
        
        multi_mesh = True
        index = self.shape_index
        if index < len(self.__meshes):
            glBindBuffer(GL_SHADER_STORAGE_BUFFER, self.__ssbo)
            glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, glm.sizeof(glm.mat4), glm.value_ptr(model)) 
            if multi_mesh:
                self.__multimesh.draw_single(index)
            else:
                self.__meshes[index].draw()

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

class SingleMesh:
    def __init__(self, mesh_specification):
        attr_array = mesh_specification.attributes
        index_array = mesh_specification.indices
        stride, format = mesh_specification.format

        self.__no_indices = len(index_array)
        vertex_attributes = (ctypes.c_float * len(attr_array))(*attr_array)
        indices = (ctypes.c_uint32 * self.__no_indices)(*index_array)
        
        self.__vao = glGenVertexArrays(1)
        self.__vbo, self.__ibo = glGenBuffers(2)
        
        glBindVertexArray(self.__vao)

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, self.__ibo)
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices, GL_STATIC_DRAW)

        glBindBuffer(GL_ARRAY_BUFFER, self.__vbo)
        glBufferData(GL_ARRAY_BUFFER, vertex_attributes, GL_STATIC_DRAW)

        offset = 0
        float_size = ctypes.sizeof(ctypes.c_float)
        for i, attribute_format in enumerate(format):   
            tuple_size = attribute_format[1]
            glVertexAttribPointer(i, tuple_size, GL_FLOAT, False, stride*float_size, ctypes.c_void_p(offset))
            offset += tuple_size * float_size
            glEnableVertexAttribArray(i)

    def draw(self):
        glBindVertexArray(self.__vao)
        glDrawElements(GL_TRIANGLES, self.__no_indices, GL_UNSIGNED_INT, None)

class MultiMesh:
    def __init__(self, mesh_specifications, stride, format):
        
        attributes = [mesh.attributes for mesh in mesh_specifications]
        indices = [mesh.indices for mesh in mesh_specifications]
        attributes_len = sum(len(a) for a in attributes)
        indices_len = sum(len(i) for i in indices)

        self.__no_of_meshes = len(mesh_specifications)
        draw_indirect_list = []
        first_index = 0
        base_vertex = 0
        for attr_list, index_list in zip(attributes, indices):
            no_of_indices = len(index_list)
            no_of_attributes = len(attr_list) // stride
            draw_indirect_list += [no_of_indices, 1, first_index, base_vertex, 0]
            first_index += no_of_indices
            base_vertex += no_of_attributes

        self.__vao = glGenVertexArrays(1)
        self.__vbo, self.__ibo, self.__dbo = glGenBuffers(3)

        draw_indirect_array = (ctypes.c_uint32 * len(draw_indirect_list))(*draw_indirect_list)
        glBindBuffer(GL_DRAW_INDIRECT_BUFFER, self.__dbo)
        glBufferData(GL_DRAW_INDIRECT_BUFFER, draw_indirect_array, GL_STATIC_DRAW)

        glBindBuffer(GL_ARRAY_BUFFER, self.__vbo)
        glBufferData(GL_ARRAY_BUFFER, attributes_len*4, None, GL_STATIC_DRAW)
        offset = 0
        for attr_list in attributes:
            no_of_values = len(attr_list)
            value_array = (ctypes.c_float * no_of_values)(*attr_list)
            glBufferSubData(GL_ARRAY_BUFFER, offset, no_of_values*4, value_array)
            offset += no_of_values*4

        glBindVertexArray(self.__vao)

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, self.__ibo)
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices_len*4, None, GL_STATIC_DRAW)
        offset = 0
        for index_list in indices:
            no_of_indices = len(index_list)
            index_array = (ctypes.c_uint32 * no_of_indices)(*index_list)
            glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, offset, no_of_indices*4, index_array)
            offset += no_of_indices*4

        offset = 0
        float_size = ctypes.sizeof(ctypes.c_float)
        for i, attribute_format in enumerate(format):   
            tuple_size = attribute_format[1]
            glVertexAttribPointer(i, tuple_size, GL_FLOAT, False, stride*float_size, ctypes.c_void_p(offset))
            offset += tuple_size * float_size
            glEnableVertexAttribArray(i)

    def draw(self):
        # GLAPI/glDrawElementsIndirect
        # https://www.khronos.org/opengl/wiki/GLAPI/glDrawElementsIndirect
        glBindVertexArray(self.__vao)
        glBindBuffer(GL_DRAW_INDIRECT_BUFFER, self.__dbo)
        glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, None, self.__no_of_meshes, 4*5)
    
    def draw_single(self, index):
        if index < self.__no_of_meshes:
            glBindVertexArray(self.__vao)
            glBindBuffer(GL_DRAW_INDIRECT_BUFFER, self.__dbo)
            glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, ctypes.c_void_p(index*4*5), 1, 4*5)

phong_vert = """
#version 460 core

layout (location = 0) in vec3 a_pos;
layout (location = 1) in vec3 a_nv;
layout (location = 2) in vec3 a_uvw;

out vec3 v_pos;
out vec3 v_nv;
out vec3 v_uvw;

uniform mat4 u_proj; 
uniform mat4 u_view; 

layout(std430, binding = 1) buffer Draw
{
    mat4 model[];
} draw;

void main()
{
    mat4 model  = draw.model[gl_DrawID];
    mat3 normal = transpose(inverse(mat3(model)));
    
    vec4 world_pos = model * vec4(a_pos.xyz, 1.0);

    v_pos       = world_pos.xyz;
    v_nv        = normal * a_nv; 
    v_uvw       = a_uvw; 
    gl_Position = u_proj * u_view * world_pos;
}
"""

phong_frag = """
#version 460 core

out vec4 frag_color;
in  vec3 v_pos;
in  vec3 v_nv;
in  vec3 v_uvw;
uniform mat4 u_view; 

vec3 HUEtoRGB(in float H)
{
    float R = abs(H * 6.0 - 3.0) - 1.0;
    float G = 2.0 - abs(H * 6.0 - 2.0);
    float B = 2.0 - abs(H * 6.0 - 4.0);
    return clamp( vec3(R,G,B), 0.0, 1.0 );
}

void main()
{
    vec4  color = vec4(HUEtoRGB(v_uvw.z), 1.0);
    vec3  L     = normalize(vec3(1.0, -1.0, 1.0));
    vec3  N     = normalize(v_nv);
    vec3  eye   = inverse(u_view)[3].xyz;
    vec3  V     = normalize(eye - v_pos);
    vec3  H     = normalize(V + L);
    float ka    = 0.5;
    float kd    = max(0.0, dot(N, L)) * 0.5;
    float NdotH = max(0.0, dot(N, H));
    float sh    = 100.0;
    float ks    = pow(NdotH, sh) * 0.1;
    frag_color  = vec4(color.rgb * (ka + kd + ks), color.a);
}
"""     

root = create_window()
root.mainloop()