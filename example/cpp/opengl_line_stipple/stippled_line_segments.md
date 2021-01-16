# `glLineStipple` in OpenGL core profile 

- [glLineStipple deprecated in OpenGL 3.1](https://stackoverflow.com/questions/6017176/gllinestipple-deprecated-in-opengl-3-1/55088683#55088683)  
- [Dashed line in OpenGL3?](https://stackoverflow.com/questions/52928678/dashed-line-in-opengl3/54543267#54543267)  
- [Three.js uniform dashed line relative to camera](https://stackoverflow.com/questions/54516794/three-js-uniform-dashed-line-relative-to-camera/54524171#54524171)  

Answer to question [glLineStipple deprecated in OpenGL 3.1](https://stackoverflow.com/questions/6017176/gllinestipple-deprecated-in-opengl-3-1/55088683#55088683)  

To answer this question, we've to investigate first, what [`glLineStipple`](https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/xhtml/glLineStipple.xml) actually does.

See the image, where the quad at the left is drawn by 4 separated line segments using the primitive type [`GL_LINES`](https://www.khronos.org/opengl/wiki/Primitive#Line_primitives).  
The circle at the right is drawn by a consecutive polygon line, using the primitive type [`GL_LINE_STRIP`](https://www.khronos.org/opengl/wiki/Primitive#Line_primitives).

[![][1]][1]

When using line segments, the stipple pattern is restarted at each segment. The pattern is *restated at each primitive.  
When using a line strip, then the stipple pattern is applied seamless to the entire polygon. A pattern seamlessly continuous beyond vertex coordinates.
Be aware that the length of the pattern is stretched at the diagonals. This is possibly the key to the implementation.  

For separate line segments, this is not very complicated at all, but for line strips things get a bit more complicated. The length of the line cannot be calculated in the shader program, without knowing all the primitives of the line. Even if all the primitives would be known (e.g. SSBO), then the calculation would have to be done in a loop.  
See also [Dashed lines with OpenGL core profile](https://stackoverflow.com/questions/52928678/dashed-line-in-opengl3/54543267#54543267).

Anyway, it is not necessary to implement a geometry shader. The trick is to know the start of the line segment in the fragment shader. This easy by using a [`flat`](https://www.khronos.org/opengl/wiki/Type_Qualifier_(GLSL)#Interpolation_qualifiers) interpolation qualifier.

The vertex shader has to pass the normalized device coordinate to the fragment shader. Once with default interpolation and once with no (flat) interpolation. This causes that in the fragment shade, the first input parameter contains the NDC coordinate of the actual position on the line and the later the NDC coordinate of the start of the line.

<!-- language: glsl -->

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

Additionally the varying inputs, the fragment shader has uniform variables. `u_resolution` contains the width and the height of the viewport. `u_factor` and `u_pattern` are the multiplier and the 16 bit pattern according to the parameters of [`glLineStipple`](https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/xhtml/glLineStipple.xml).

So the length of the line from the start to the actual fragment can be calculated:

<!-- language: glsl -->

    vec2  dir  = (vertPos.xy-startPos.xy) * u_resolution/2.0;
    float dist = length(dir);

And fragment on the gap can be discarded, by the [`discard`](https://www.khronos.org/opengl/wiki/Fragment_Shader#Special_operations) command.

<!-- language: glsl -->

    uint bit = uint(round(dist / u_factor)) & 15U;
    if ((u_pattern & (1U<<bit)) == 0U)
        discard; 

Fragment shader:

<!-- language: glsl -->

    #version 330

    flat in vec3 startPos;
    in vec3 vertPos;

    out vec4 fragColor;

    uniform vec2  u_resolution;
    uniform uint  u_pattern;
    uniform float u_factor;

    void main()
    {
        vec2  dir  = (vertPos.xy-startPos.xy) * u_resolution/2.0;
        float dist = length(dir);

        uint bit = uint(round(dist / u_factor)) & 15U;
        if ((u_pattern & (1U<<bit)) == 0U)
            discard; 
        fragColor = vec4(1.0);
    }

This implementation is much easier and shorter, then using geometry shaders. The `flat` interpolation qualifier is supported since [GLSL 1.30](https://www.khronos.org/registry/OpenGL/specs/gl/GLSLangSpec.1.30.pdf) and [GLSL ES 3.00](https://www.khronos.org/registry/OpenGL/specs/es/3.0/GLSL_ES_Specification_3.00.pdf). In this version geometry shaders are not supported.    
See the line rendering which was generated with the above shader.

[![][2]][2]

The shader gives a proper result line segments, but fails for line strips, since the stipple pattern is restarted at each vertex coordinate.  
The issue can't even be solved by a geometry shader. This part of the question remains still unresolved.

For the following simple demo program I've used the [GLFW](https://www.glfw.org/) API for creating a window, [GLEW](http://glew.sourceforge.net/) for loading OpenGL and [GLM -OpenGL Mathematics](https://glm.g-truc.net/0.9.9/index.html) for the math. I don't provide the code for the function `CreateProgram`, which just creates a program object, from the vertex shader and fragment shader source code:

<!-- language: cpp -->

    #include <vector>
    #include <string>
    #include <glm/glm.hpp>
    #include <glm/gtc/matrix_transform.hpp>
    #include <glm/gtc/type_ptr.hpp>
    #include <gl/gl_glew.h>
    #include <GLFW/glfw3.h>

    std::string vertShader = R"(
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
    )";

    std::string fragShader = R"(
    #version 330

    flat in vec3 startPos;
    in vec3 vertPos;

    out vec4 fragColor;

    uniform vec2  u_resolution;
    uniform uint  u_pattern;
    uniform float u_factor;

    void main()
    {
        vec2  dir  = (vertPos.xy-startPos.xy) * u_resolution/2.0;
        float dist = length(dir);

        uint bit = uint(round(dist / u_factor)) & 15U;
        if ((u_pattern & (1U<<bit)) == 0U)
            discard; 
        fragColor = vec4(1.0);
    }
    )";

    GLuint CreateVAO(std::vector<glm::vec3> &varray)
    {
        GLuint bo[2], vao;
        glGenBuffers(2, bo);
        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);
        glEnableVertexAttribArray(0); 
        glBindBuffer(GL_ARRAY_BUFFER, bo[0] );
        glBufferData(GL_ARRAY_BUFFER, varray.size()*sizeof(*varray.data()), varray.data(), GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0); 

        return vao;
    }

    int main(void)
    {
        if ( glfwInit() == 0 )
            return 0;
        GLFWwindow *window = glfwCreateWindow( 800, 600, "GLFW OGL window", nullptr, nullptr );
        if ( window == nullptr )
            return 0;
        glfwMakeContextCurrent(window);

        if ( glewInit() != GLEW_OK )
            return 0;

        GLuint program    = CreateProgram(vertShader, fragShader);
        GLint loc_mvp     = glGetUniformLocation(program, "u_mvp");
        GLint loc_res     = glGetUniformLocation(program, "u_resolution");
        GLint loc_pattern = glGetUniformLocation(program, "u_pattern");
        GLint loc_factor  = glGetUniformLocation(program, "u_factor");

        glUseProgram(program);

        GLushort pattern = 0x18ff;
        GLfloat  factor  = 2.0f;
        glUniform1ui(loc_pattern, pattern);
        glUniform1f(loc_factor, factor);
        //glLineStipple(2.0, pattern);
        //glEnable(GL_LINE_STIPPLE);

        glm::vec3 p0(-1.0f, -1.0f, 0.0f);
        glm::vec3 p1(1.0f, -1.0f, 0.0f);
        glm::vec3 p2(1.0f, 1.0f, 0.0f);
        glm::vec3 p3(-1.0f, 1.0f, 0.0f);
        std::vector<glm::vec3> varray1{ p0, p1, p1, p2, p2, p3, p3, p0 };
        GLuint vao1 = CreateVAO(varray1);

        std::vector<glm::vec3> varray2;
        for (size_t u=0; u <= 360; u += 8)
        {
            double a = u*M_PI/180.0;
            double c = cos(a), s = sin(a);
            varray2.emplace_back(glm::vec3((float)c, (float)s, 0.0f));
        }
        GLuint vao2 = CreateVAO(varray2);

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
            glBindVertexArray(vao1);
            glDrawArrays(GL_LINES, 0, (GLsizei)varray1.size());

            glm::mat4 modelview2( 1.0f );
            modelview2 = glm::translate(modelview2, glm::vec3(0.6f, 0.0f, 0.0f) );
            modelview2 = glm::scale(modelview2, glm::vec3(0.5f, 0.5f, 1.0f) );
            glm::mat4 mvp2 = project * modelview2;

            glUniformMatrix4fv(loc_mvp, 1, GL_FALSE, glm::value_ptr(mvp2));
            glBindVertexArray(vao2);
            glDrawArrays(GL_LINE_STRIP, 0, (GLsizei)varray2.size());

            glfwSwapBuffers(window);
            glfwPollEvents();
        }
        glfwTerminate();

        return 0;
    }


  [1]: https://i.stack.imgur.com/TQoZA.png
  [2]: https://i.stack.imgur.com/1DWYd.png