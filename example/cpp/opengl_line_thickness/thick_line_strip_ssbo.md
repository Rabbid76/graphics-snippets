[OpenGL Line Width](https://stackoverflow.com/questions/3484260/opengl-line-width)  


I recommend to use a [Shader](https://www.khronos.org/opengl/wiki/Shader), which generates [triangle primitives](https://www.khronos.org/opengl/wiki/Primitive#Triangle_primitives) along a line strip (or even a line loop).  
The task is to generate thick line strip, with as less CPU and GPU overhead as possible. That means to avoid computation of polygons on the CPU as well as geometry shaders (or tessellation shaders).

Each segment of the line consist of a quad represented by 2 triangle primitives respectively 6 vertices.

```lang-none
0        2   5
 +-------+  +
 |     /  / |
 |   /  /   |
 | /  /     |
 +  +-------+
1   3        4
```

Between the line segments the miter hast to be found and the quads have to be cut to the miter.

```lang-none
+----------------+
|              / |
| segment 1  /   |
|          /     |
+--------+       |
         | segment 2
         |       |
         |       |
         +-------+
```

Create an array with the corners points of the line strip. The array has to contain the first and the last point twice. Of course it would be easy, to identify the first and last element of the array by comparing the index to 0 and the length of the array, but we don't want to do any extra checks in the shader.  
If a line loop has to be draw, then the last point has to be add to the array head and the first point to its tail.

The array of points is stored to a [Shader Storage Buffer Object](https://www.khronos.org/opengl/wiki/Shader_Storage_Buffer_Object). We use the benefit, that the last variable of the SSBO can be an array of variable size. In older versions of OpenGL (or OpenGL ES) a [Uniform Buffer Object](https://www.khronos.org/opengl/wiki/Uniform_Buffer_Object) or even a [Texture](https://www.khronos.org/opengl/wiki/Texture) can be used.

The shader doesn't need any vertex coordinates or attributes. All we have to know is the index of the line segment. The coordinates are stored in the buffer. To find the index we make use of the the index of the vertex currently being processed ([`gl_VertexID`](https://www.khronos.org/opengl/wiki/Vertex_Shader#Other_inputs)).  
To draw a line strip with `N` segments, `6*(N-1)` vertices have tpo be processed.

We have to create an "empty" [Vertex Array Object](https://www.khronos.org/opengl/wiki/Vertex_Specification#Vertex_Array_Object) (without any vertex attribute specification):

```cpp
glGenVertexArrays(1, &vao);
glBindVertexArray(vao);
```

And to draw `2*(N-1)` triangle (`6*(N-1)` vertices):

```cpp
glDrawArrays(GL_TRIANGLES, 0, 6*(N-1));
```

For the coordinate array in the SSBO, the data type `vec4` is used (Pleas believe me, you don't want to use [`vec3`](https://stackoverflow.com/questions/38172696/should-i-ever-use-a-vec3-inside-of-a-uniform-buffer-or-shader-storage-buffer-o)):

```glsl
layout(std430, binding = 0) buffer TVertex
{
   vec4 vertex[];
};
```

Compute the index of the line segment, where the vertex coordinate belongs too and the index of the point in the 2 triangles:

```glsl
int line_i = gl_VertexID / 6;
int tri_i  = gl_VertexID % 6;
```

Since we are drawing `N-1` line segments, but the number of elements in the array is `N+2`, the elements form `vertex[line_t]` to `vertex[line_t+3]` can be accessed for each vertex which is processed in the vertex shader.    
`vertex[line_t+1]` and `vertex[line_t+2]` are the start respectively end coordinate of the line segment. `vertex[line_t]` and `vertex[line_t+3]` are required to compute the miter.

The thickness of the line should be set in pixel unit (`uniform float u_thickness`). The coordinates have to be transformed from model space to window space. For that the resolution of the viewport has to be known (`uniform vec2 u_resolution`). Don't forget the [perspective divide](https://www.khronos.org/opengl/wiki/Vertex_Post-Processing#Perspective_divide). The drawing of the line will even work at perspective projection.

```glsl
vec4 va[4];
for (int i=0; i<4; ++i)
{
    va[i] = u_mvp * vertex[line_i+i];
    va[i].xyz /= va[i].w;
    va[i].xy = (va[i].xy + 1.0) * 0.5 * u_resolution;
}
```

The miter calculation even works if the predecessor or successor point is equal to the start respectively end point of the line segment. In this case the end of the line is cut normal to its tangent:

```glsl
vec2 v_line   = normalize(va[2].xy - va[1].xy);
vec2 nv_line  = vec2(-v_line.y, v_line.x);
vec2 v_pred   = normalize(va[1].xy - va[0].xy);
vec2 v_succ   = normalize(va[3].xy - va[2].xy);
vec2 v_miter1 = normalize(nv_line + vec2(-v_pred.y, v_pred.x));
vec2 v_miter2 = normalize(nv_line + vec2(-v_succ.y, v_succ.x));
```

In the final vertex shader we just need to calculate either `v_miter1` or `v_miter2` dependent on the `tri_i`. With the miter, the normal vector to the line segment and the line thickness (`u_thickness`), the vertex coordinate can be computed:

```glsl
vec4 pos;
if (tri_i == 0 || tri_i == 1 || tri_i == 3)
{
    vec2 v_pred  = normalize(va[1].xy - va[0].xy);
    vec2 v_miter = normalize(nv_line + vec2(-v_pred.y, v_pred.x));

    pos = va[1];
    pos.xy += v_miter * u_thickness * (tri_i == 1 ? -0.5 : 0.5) / dot(v_miter, nv_line);
}
else
{
    vec2 v_succ  = normalize(va[3].xy - va[2].xy);
    vec2 v_miter = normalize(nv_line + vec2(-v_succ.y, v_succ.x));

    pos = va[2];
    pos.xy += v_miter * u_thickness * (tri_i == 5 ? 0.5 : -0.5) / dot(v_miter, nv_line);
}
```

Finally the window coordinates have to be transformed back to clip space coordinates. Transform from window space to normalized device space. The perspective divide has to be reversed:

```glsl
pos.xy = pos.xy / u_resolution * 2.0 - 1.0;
pos.xyz *= pos.w;
```

The shader can generate the following polygons (rendered with `glPolygonMode(GL_FRONT_AND_BACK, GL_LINE)`)

[![][1]][1]

(with default mode - `glPolygonMode(GL_FRONT_AND_BACK, GL_FILL)`)

[![][2]][2]

For the following simple demo program I've used the [GLFW](https://www.glfw.org/) API for creating a window, [GLEW](http://glew.sourceforge.net/) for loading OpenGL and [GLM -OpenGL Mathematics](https://glm.g-truc.net/0.9.9/index.html) for the math. I don't provide the code for the function `CreateProgram`, which just creates a program object, from the vertex shader and fragment shader source code:

```cpp
#include <vector>
#include <string>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <gl/gl_glew.h>
#include <GLFW/glfw3.h>

std::string vertShader = R"(
#version 460

layout(std430, binding = 0) buffer TVertex
{
   vec4 vertex[]; 
};

uniform mat4  u_mvp;
uniform vec2  u_resolution;
uniform float u_thickness;

void main()
{
    int line_i = gl_VertexID / 6;
    int tri_i  = gl_VertexID % 6;

    vec4 va[4];
    for (int i=0; i<4; ++i)
    {
        va[i] = u_mvp * vertex[line_i+i];
        va[i].xyz /= va[i].w;
        va[i].xy = (va[i].xy + 1.0) * 0.5 * u_resolution;
    }

    vec2 v_line  = normalize(va[2].xy - va[1].xy);
    vec2 nv_line = vec2(-v_line.y, v_line.x);

    vec4 pos;
    if (tri_i == 0 || tri_i == 1 || tri_i == 3)
    {
        vec2 v_pred  = normalize(va[1].xy - va[0].xy);
        vec2 v_miter = normalize(nv_line + vec2(-v_pred.y, v_pred.x));

        pos = va[1];
        pos.xy += v_miter * u_thickness * (tri_i == 1 ? -0.5 : 0.5) / dot(v_miter, nv_line);
    }
    else
    {
        vec2 v_succ  = normalize(va[3].xy - va[2].xy);
        vec2 v_miter = normalize(nv_line + vec2(-v_succ.y, v_succ.x));

        pos = va[2];
        pos.xy += v_miter * u_thickness * (tri_i == 5 ? 0.5 : -0.5) / dot(v_miter, nv_line);
    }

    pos.xy = pos.xy / u_resolution * 2.0 - 1.0;
    pos.xyz *= pos.w;
    gl_Position = pos;
}
)";

std::string fragShader = R"(
#version 460

out vec4 fragColor;

void main()
{
    fragColor = vec4(1.0);
}
)";

GLuint CreateSSBO(std::vector<glm::vec4> &varray)
{
    GLuint ssbo;
    glGenBuffers(1, &ssbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo );
    glBufferData(GL_SHADER_STORAGE_BUFFER, varray.size()*sizeof(*varray.data()), varray.data(), GL_STATIC_DRAW); 
    return ssbo;
}

int main(void)
{
    if ( glfwInit() == 0 )
        return 0;
    GLFWwindow *window = glfwCreateWindow( 800, 600, "GLFW OGL window", nullptr, nullptr );
    if ( window == nullptr )
    {
        glfwTerminate();
        retturn 0;
    }
    glfwMakeContextCurrent(window);
    if ( glewInit() != GLEW_OK )
        return 0;

    GLuint program  = CreateProgram(vertShader, fragShader);
    GLint  loc_mvp  = glGetUniformLocation(program, "u_mvp");
    GLint  loc_res  = glGetUniformLocation(program, "u_resolution");
    GLint  loc_thi  = glGetUniformLocation(program, "u_thickness");

    glUseProgram(program);
    glUniform1f(loc_thi, 20.0);

    GLushort pattern = 0x18ff;
    GLfloat  factor  = 2.0f;

    glm::vec4 p0(-1.0f, -1.0f, 0.0f, 1.0f);
    glm::vec4 p1(1.0f, -1.0f, 0.0f, 1.0f);
    glm::vec4 p2(1.0f, 1.0f, 0.0f, 1.0f);
    glm::vec4 p3(-1.0f, 1.0f, 0.0f, 1.0f);
    std::vector<glm::vec4> varray1{ p3, p0, p1, p2, p3, p0, p1 };
    GLuint ssbo1 = CreateSSBO(varray1);

    std::vector<glm::vec4> varray2;
    for (int u=-8; u <= 368; u += 8)
    {
        double a = u*M_PI/180.0;
        double c = cos(a), s = sin(a);
        varray2.emplace_back(glm::vec4((float)c, (float)s, 0.0f, 1.0f));
    }
    GLuint ssbo2 = CreateSSBO(varray2);

    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    glm::mat4(project);
    int vpSize[2]{0, 0};
    while (!glfwWindowShouldClose(window))
    {
        int w, h;
        glfwGetFramebufferSize(window, &w, &h);
        if (w != vpSize[0] ||  h != vpSize[1])
        {
            vpSize[0] = w; vpSize[1] = h;
            glViewport(0, 0, vpSize[0], vpSize[1]);
            float aspect = (float)w/(float)h;
            project = glm::ortho(-aspect, aspect, -1.0f, 1.0f, -10.0f, 10.0f);
            glUniform2f(loc_res, (float)w, (float)h);
        }

        glClear(GL_COLOR_BUFFER_BIT);

        glm::mat4 modelview1( 1.0f );
        modelview1 = glm::translate(modelview1, glm::vec3(-0.6f, 0.0f, 0.0f) );
        modelview1 = glm::scale(modelview1, glm::vec3(0.5f, 0.5f, 1.0f) );
        glm::mat4 mvp1 = project * modelview1;

        glUniformMatrix4fv(loc_mvp, 1, GL_FALSE, glm::value_ptr(mvp1));
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssbo1);
        GLsizei N1 = (GLsizei)varray1.size()-2;
        glDrawArrays(GL_TRIANGLES, 0, 6*(N1-1));

        glm::mat4 modelview2( 1.0f );
        modelview2 = glm::translate(modelview2, glm::vec3(0.6f, 0.0f, 0.0f) );
        modelview2 = glm::scale(modelview2, glm::vec3(0.5f, 0.5f, 1.0f) );
        glm::mat4 mvp2 = project * modelview2;

        glUniformMatrix4fv(loc_mvp, 1, GL_FALSE, glm::value_ptr(mvp2));
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssbo2);
        GLsizei N2 = (GLsizei)varray2.size()-2;
        glDrawArrays(GL_TRIANGLES, 0, 6*(N2-1));

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glfwTerminate();

    return 0;
}
```


  [1]: https://i.stack.imgur.com/4wgXq.png
  [2]: https://i.stack.imgur.com/fVJqu.png