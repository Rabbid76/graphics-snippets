[![StackOverflow](https://stackexchange.com/users/flair/7322082.png)](https://stackoverflow.com/users/5577765/rabbid76?tab=profile)

---

# Dashed lines with OpenGL core profile

The following is the answer to the StackOverflow question [Dashed line in OpenGL3?](https://stackoverflow.com/questions/52928678/dashed-line-in-opengl3/54543267#54543267)

## Dashed line segments

For separate line segments, this is not very complicated at all. For example drawing the [`GL_LINES`](https://www.khronos.org/opengl/wiki/Primitive#Line_primitives) primitives.  
The trick is to know the start of the line segment in the fragment shader. This easy by using a [`flat`](https://www.khronos.org/opengl/wiki/Type_Qualifier_(GLSL)#Interpolation_qualifiers) interpolation qualifier.  

The vertex shader has to pass the normalized device coordinate to the fragment shader. Once with default interpolation and once with no (`flat`) interpolation. This causes that in the fragment shade, the first input parameter contains the NDC coordinate of the actual position on the line and the later the NDC coordinate of the start of the line.

```glsl
#version 330

layout (location = 0) in vec3 inPos;

flat out vec3 startPos;
out vec3 vertPos;

uniform mat4 u_mvp;

void main()
{
    vec4 pos    = u_mvp * vec4(inPos, 1.0);
    gl_Position = pos;
    vertPos     = pos.xyz / pos.w;
    startPos    = vertPos;
}
```

Additionally the varying inputs, the fragment shader has uniform variables. `u_resolution` contains the width and the height of the viewport. `u_dashSize` contains the length of line and `u_gapSize` the length of a gap in pixel.

So the length of the line from the start to the actual fragment can be calculated:

```glsl
vec2  dir  = (vertPos.xy-startPos.xy) * u_resolution/2.0;
float dist = length(dir);
```

And fragment on the gab can be discarded, by the [`discard`](https://www.khronos.org/opengl/wiki/Fragment_Shader#Special_operations) command.

```glsl
if (fract(dist / (u_dashSize + u_gapSize)) > u_dashSize/(u_dashSize + u_gapSize))
    discard; 
```

Fragment shader:

```glsl
#version 330

flat in vec3 startPos;
in vec3 vertPos;

out vec4 fragColor;

uniform vec2  u_resolution;
uniform float u_dashSize;
uniform float u_gapSize;

void main()
{
    vec2  dir  = (vertPos.xy-startPos.xy) * u_resolution/2.0;
    float dist = length(dir);

    if (fract(dist / (u_dashSize + u_gapSize)) > u_dashSize/(u_dashSize + u_gapSize))
        discard; 
    fragColor = vec4(1.0);
}
```

For the following simple demo program I've used the [GLFW](https://www.glfw.org/) API for creating a window, [GLEW](http://glew.sourceforge.net/) for loading OpenGL and [GLM -OpenGL Mathematics](https://glm.g-truc.net/0.9.9/index.html) for the math. I don't provide the code for the function `CreateProgram`, which just creates a program object, from the vertex shader and fragment shader source code:

![dashed cube](image/dashed_cube.gif)

```cpp
#include <GL/glew.h>
#include <GL/gl.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <GLFW/glfw3.h>

#include <vector>

#define _USE_MATH_DEFINES
#include <math.h>

int main(void)
{
    if (glfwInit() == GLFW_FALSE)
        return 0;
    GLFWwindow *window = glfwCreateWindow(400, 300, "OGL window", nullptr, nullptr);
    if (window == nullptr)
        return 0;
    glfwMakeContextCurrent(window);
    glewExperimental = true;
    if (glewInit() != GLEW_OK)
        return 0;

    GLuint program = CreateProgram(vertShader, fragShader);
    GLint loc_mvp  = glGetUniformLocation(program, "u_mvp");
    GLint loc_res  = glGetUniformLocation(program, "u_resolution");
    GLint loc_dash = glGetUniformLocation(program, "u_dashSize");
    GLint loc_gap  = glGetUniformLocation(program, "u_gapSize");

    glUseProgram(program);
    glUniform1f(loc_dash, 10.0f);
    glUniform1f(loc_gap, 10.0f);

    std::vector<float> varray{
        -1, -1, -1,   1, -1, -1,   1, 1, -1,   -1, 1, -1,
        -1, -1,  1,   1, -1,  1,   1, 1,  1,   -1, 1,  1
    };
    std::vector<unsigned int> iarray{
        0, 1, 1, 2, 2, 3, 3, 0, 
        4, 5, 5, 6, 6, 7, 7, 4,
        0, 4, 1, 5, 2, 6, 3, 7
    };

    GLuint bo[2], vao;
    glGenBuffers(2, bo);
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glEnableVertexAttribArray(0); 
    glBindBuffer(GL_ARRAY_BUFFER, bo[0] );
    glBufferData(GL_ARRAY_BUFFER, varray.size()*sizeof(*varray.data()), varray.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0); 
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bo[1]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, iarray.size()*sizeof(*iarray.data()), iarray.data(), GL_STATIC_DRAW);

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

    glm::mat4 view = glm::lookAt(glm::vec3(0.0f, 0.0f, 5.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 project;
    int vpSize[2]{0, 0};
    while (!glfwWindowShouldClose(window))
    {
        int w, h;
        glfwGetFramebufferSize(window, &w, &h);
        if (w != vpSize[0] ||  h != vpSize[1])
        {
            vpSize[0] = w; vpSize[1] = h;
            glViewport(0, 0, vpSize[0], vpSize[1]);
            project = glm::perspective(glm::radians(90.0f), (float)w/(float)h, 0.1f, 10.0f);
            glUniform2f(loc_res, (float)w, (float)h);
        }

        static float angle = 1.0f;
        glm::mat4 modelview( 1.0f );
        modelview = glm::translate(modelview, glm::vec3(0.0f, 0.0f, -3.0f) );
        modelview = glm::rotate(modelview, glm::radians(angle), glm::vec3(1.0f, 0.0f, 0.0f));
        modelview = glm::rotate(modelview, glm::radians(angle*0.5f), glm::vec3(0.0f, 1.0f, 0.0f));
        angle += 0.5f;
        glm::mat4 mvp = project * modelview;

        glUniformMatrix4fv(loc_mvp, 1, GL_FALSE, glm::value_ptr(mvp));
        glClear(GL_COLOR_BUFFER_BIT);
        glDrawElements(GL_LINES, (GLsizei)iarray.size(), GL_UNSIGNED_INT, nullptr);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glfwTerminate();

    return 0;
}
```

## Dashed line strips

Things get a bit more complicated, if the goal is to draw a dashed line along a polygon. For example drawing a [`GL_LINE_STRIP`](https://www.khronos.org/opengl/wiki/Primitive#Line_primitives) primitive.

The length of the line cannot be calculated in the shader program, without knowing all the primitives of the line. Even if all the primitives would be known (e.g. SSBO), then the calculation would have to be done in a loop.  
I decided to add an additional attribute to the shader program, which contains the "distance" from the start of the line to the vertex coordinate. By "distance" is meant the length of the projected polygon on to the viewport.

This causes that the vertex shader and fragment shader is even simpler:

Vertex shader:

```glsl
#version 330

layout (location = 0) in vec3 inPos;
layout (location = 1) in float inDist;

out float dist;

uniform mat4 u_mvp;

void main()
{
    dist        = inDist;
    gl_Position = u_mvp * vec4(inPos, 1.0);
}
```

Fragment shader:

```glsl
#version 330

in float dist;

out vec4 fragColor;

uniform vec2  u_resolution;
uniform float u_dashSize;
uniform float u_gapSize;

void main()
{
    if (fract(dist / (u_dashSize + u_gapSize)) > u_dashSize/(u_dashSize + u_gapSize))
        discard; 
    fragColor = vec4(1.0);
}
```

In the demo program the `inDist` attribute is calculated on the CPU. Each vertex coordinate is transformed by the model, view, projection matrix. Finally it is transformed from normalized device space to window space. The XY distance between adjacent coordinates of the line strip is calculated and the lengths are summed along the line strip and assigned to the corresponding attribute value:

```cpp
int w = [...], h = [...];               // window widht and height
glm::mat4 mpv = [...];                  // model view projection matrix
std::vector<glm::vec3> varray{ [...] }; // array of vertex 

std::vector<float> darray(varray.size(), 0.0f); // distance attribute - has to be computed

glm::mat4 wndmat = glm::scale(glm::mat4(1.0f), glm::vec3((float)w/2.0f, (float)h/2.0f, 1.0f));
wndmat = glm::translate(wndmat, glm::vec3(1.0f, 1.0f, 0.0f));

glm::vec2 vpPt(0.0f, 0.0f);
float dist = 0.0f;
for (size_t i=0; i < varray.size(); ++i)
{
    darray[i] = dist;
    glm::vec4 clip = mvp * glm::vec4(varray[i], 1.0f);
    glm::vec4 ndc  = clip / clip.w;
    glm::vec4 vpC  = wndmat * ndc;
    float len = i==0 ? 0.0f :  glm::length(vpPt - glm::vec2(vpC));
    vpPt = glm::vec2(vpC);
    dist += len;
}
``` 

Demo program:

![dashed circle](image/dashed_circle.gif)

```cpp
int main(void)
{
    if (glfwInit() == GLFW_FALSE)
        return 0;
    GLFWwindow *window = glfwCreateWindow(800, 600, "OGL window", nullptr, nullptr);
    if (window == nullptr)
        return 0;
    glfwMakeContextCurrent(window);
    glewExperimental = true;
    if (glewInit() != GLEW_OK)
        return 0;

    GLuint program = CreateProgram(vertShader, fragShader);
    GLint loc_mvp  = glGetUniformLocation(program, "u_mvp");
    GLint loc_res  = glGetUniformLocation(program, "u_resolution");
    GLint loc_dash = glGetUniformLocation(program, "u_dashSize");
    GLint loc_gap  = glGetUniformLocation(program, "u_gapSize");

    glUseProgram(program);
    glUniform1f(loc_dash, 10.0f);
    glUniform1f(loc_gap, 10.0f);

    std::vector<glm::vec3> varray;
    for (size_t u=0; u <= 360; ++u)
    {
        double a = u*M_PI/180.0;
        double c = cos(a), s = sin(a);
        varray.emplace_back(glm::vec3((float)c, (float)s, 0.0f));
    }
    std::vector<float> darray(varray.size(), 0.0f);

    GLuint bo[2], vao;
    glGenBuffers(2, bo);
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glEnableVertexAttribArray(0); 
    glEnableVertexAttribArray(1); 
    glBindBuffer(GL_ARRAY_BUFFER, bo[0] );
    glBufferData(GL_ARRAY_BUFFER, varray.size()*sizeof(*varray.data()), varray.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0); 
    glBindBuffer(GL_ARRAY_BUFFER, bo[1] );
    glBufferData(GL_ARRAY_BUFFER, darray.size()*sizeof(*darray.data()), darray.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, 0, 0); 

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

    glm::mat4 view = glm::lookAt(glm::vec3(0.0f, 0.0f, 5.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 project, wndmat;
    int vpSize[2]{0, 0};
    while (!glfwWindowShouldClose(window))
    {
        int w, h;
        glfwGetFramebufferSize(window, &w, &h);
        if (w != vpSize[0] ||  h != vpSize[1])
        {
            vpSize[0] = w; vpSize[1] = h;
            glViewport(0, 0, vpSize[0], vpSize[1]);
            project = glm::perspective(glm::radians(90.0f), (float)w/(float)h, 0.1f, 10.0f);
            glUniform2f(loc_res, (float)w, (float)h);
            wndmat = glm::scale(glm::mat4(1.0f), glm::vec3((float)w/2.0f, (float)h/2.0f, 1.0f));
            wndmat = glm::translate(wndmat, glm::vec3(1.0f, 1.0f, 0.0f));
        }

        static float angle = 1.0f;
        glm::mat4 modelview( 1.0f );
        modelview = glm::translate(modelview, glm::vec3(0.0f, 0.0f, -2.0f) );
        modelview = glm::rotate(modelview, glm::radians(angle), glm::vec3(1.0f, 0.0f, 0.0f));
        modelview = glm::rotate(modelview, glm::radians(angle*0.5f), glm::vec3(0.0f, 1.0f, 0.0f));
        angle += 0.5f;
        glm::mat4 mvp = project * modelview;

        glm::vec2 vpPt(0.0f, 0.0f);
        float dist = 0.0f;
        for (size_t i=0; i < varray.size(); ++i)
        {
            darray[i] = dist;
            glm::vec4 clip = mvp * glm::vec4(varray[i], 1.0f);
            glm::vec4 ndc  = clip / clip.w;
            glm::vec4 vpC  = wndmat * ndc;
            float len = i==0 ? 0.0f :  glm::length(vpPt - glm::vec2(vpC));
            vpPt = glm::vec2(vpC);
            dist += len;
        }
        glBufferSubData(GL_ARRAY_BUFFER, 0, darray.size()*sizeof(*darray.data()), darray.data());

        glUniformMatrix4fv(loc_mvp, 1, GL_FALSE, glm::value_ptr(mvp));
        glClear(GL_COLOR_BUFFER_BIT);
        glDrawArrays(GL_LINE_STRIP, 0, (GLsizei)varray.size());

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glfwTerminate();

    return 0;
}
```
