import os, sys 
sys.path.append(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..'))

import time
import tkinter
import tkinter.ttk
from pyopengltk import OpenGLFrame
from OpenGL.GL import *
from OpenGL.GL.shaders import *
import ctypes 
import glm 
from utility import triangulated_mesh
from utility.tkinter_utility import TkinterNavigation, TkOpenGLView, TkOpenGLFrame
from utility.opengl_mesh import SingleMesh, MultiMesh

def create_window():
    root = tkinter.Tk()
    status_bar_text = tkinter.StringVar()

    control_frame = tkinter.Frame(root, bg='lightgray', padx=4, pady=4)
    status_bar_frame = tkinter.Frame(root, borderwidth=1)
    open_gl_view = OpenGLView(status_bar_text)
    view_frame = TkOpenGLFrame(open_gl_view, root, width=600, height=400)

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
    open_gl_view.shape_index = 10
    control_shape.grid(column=0, row=0)
    def shape_changed(event):
        open_gl_view.shape_index = control_shape.current()
    control_shape.bind('<<ComboboxSelected>>', shape_changed)  

    control_polygonmode = tkinter.ttk.Combobox(control_frame, state="readonly", font=("Consolas", 10))
    control_polygonmode['values'] = ['point', 'line', 'fill']
    control_polygonmode.current(2)
    open_gl_view.polygon_mode = 2
    control_polygonmode.grid(column=1, row=0)
    def polygon_mode_changed(event):
        open_gl_view.polygon_mode = control_polygonmode.current()
    control_polygonmode.bind('<<ComboboxSelected>>', polygon_mode_changed)  

    control_culling = tkinter.ttk.Combobox(control_frame, state="readonly", font=("Consolas", 10))
    control_culling['values'] = ['no face culling', 'back face culling', 'front face culling']
    control_culling.current(1)
    open_gl_view.cull_mode = 1
    control_culling.grid(column=2, row=0)
    def culling_changed(event):
        open_gl_view.cull_mode = control_culling.current()
    control_culling.bind('<<ComboboxSelected>>', culling_changed)  

    return root

class OpenGLView(TkOpenGLView):
    def __init__(self, status_text):
        super().__init__() 
        self.shape_index = 0
        self.polygon_mode = 2
        self.cull_mode = 1
        self.status_text = status_text

    def init(self, opengl_frame):
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
            opengl_frame,
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

        glEnable(GL_CULL_FACE)
        glFrontFace(GL_CCW)
        glCullFace(GL_BACK)

    def resize(self, opengl_frame):
        self.__tkinter_navigation.reshape()
        glViewport(0, 0, opengl_frame.width, opengl_frame.height)

    def draw(self, opengl_frame):
        self.status_text.set(f"fps {opengl_frame.fps:.03f} frame: {opengl_frame.number_of_frames:05d}")

        vp_valid, vp_size, view, projection = self.__tkinter_navigation.update()
        if not vp_valid:
            glViewport(0, 0, *vp_size)

        if self.cull_mode == 0:
            glDisable(GL_CULL_FACE)
        else:
            glEnable(GL_CULL_FACE)
            glCullFace(GL_FRONT if self.cull_mode == 2 else GL_BACK)
        glPolygonMode(GL_FRONT_AND_BACK, GL_POINT if self.polygon_mode == 0 else GL_LINE if self.polygon_mode == 1 else GL_FILL)

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
                self.__multimesh.draw_range(index, index+1)
            else:
                self.__meshes[index].draw()

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
    vec3  eye   = inverse(u_view)[3].xyz;
    vec3  V     = normalize(eye - v_pos);
    float face  = sign(dot(v_nv, V));
    vec3  N     = normalize(v_nv) * face;
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