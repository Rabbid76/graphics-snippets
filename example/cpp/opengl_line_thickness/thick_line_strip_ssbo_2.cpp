// OpenGL Line Width
// https://stackoverflow.com/questions/3484260/opengl-line-width/59688394#59688394
// Drawing a line in modern OpenGL
// https://stackoverflow.com/questions/60440682/drawing-a-line-in-modern-opengl/60440937#60440937


// STL
//#include <iostream>
#include <vector>
#include <string>
#include <stdexcept>
//#include <chrono>


// GLM [https://glm.g-truc.net/0.9.9/api/index.html]
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


// OpenGL
#include <gl/gl_glew.h>


// GLFW [https://www.glfw.org/]
#include <GLFW/glfw3.h>


// project includes
#include <gl/gl_debug.h>
#include <gl/gl_shader.h>


// shader

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


// main

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
        throw std::runtime_error( "error initializing glfw" );

    //glfwWindowHint(GLFW_REFRESH_RATE, 10);

    GLFWwindow *window = glfwCreateWindow( 800, 600, "GLFW OGL window", nullptr, nullptr );
    if ( window == nullptr )
    {
        glfwTerminate();
        throw std::runtime_error( "error initializing window" ); 
    }

    glfwMakeContextCurrent(window);

    if ( glewInit() != GLEW_OK )
        throw std::runtime_error( "error initializing glew" );

    OpenGL::CContext::TDebugLevel debug_level = OpenGL::CContext::TDebugLevel::all;
    OpenGL::CContext context;
    context.Init( debug_level );

    GLuint program  = OpenGL::CreateProgram(vertShader, fragShader);
    GLint  loc_mvp  = glGetUniformLocation(program, "u_mvp");
    GLint  loc_res  = glGetUniformLocation(program, "u_resolution");
    GLint  loc_thi  = glGetUniformLocation(program, "u_thickness");

    glUseProgram(program);
    glUniform1f(loc_thi, 20.0);

    GLushort pattern = 0x18ff;
    GLfloat  factor  = 2.0f;

    std::vector<glm::vec4> varray;
    varray.emplace_back(glm::vec4(0.0f, -1.0f, 0.0f, 1.0f));
    varray.emplace_back(glm::vec4(1.0f, -1.0f, 0.0f, 1.0f));
    for (int u=0; u <= 90; u += 10)
    {
        double a = u*M_PI/180.0;
        double c = cos(a), s = sin(a);
        varray.emplace_back(glm::vec4((float)c, (float)s, 0.0f, 1.0f));
    }
    varray.emplace_back(glm::vec4(-1.0f, 1.0f, 0.0f, 1.0f));
    for (int u = 90; u >= 0; u -= 10)
    {
        double a = u * M_PI / 180.0;
        double c = cos(a), s = sin(a);
        varray.emplace_back(glm::vec4((float)c-1.0f, (float)s-1.0f, 0.0f, 1.0f));
    }
    varray.emplace_back(glm::vec4(1.0f, -1.0f, 0.0f, 1.0f));
    varray.emplace_back(glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
    GLuint ssbo = CreateSSBO(varray);
    
    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssbo);
    GLsizei N = (GLsizei)varray.size() - 2;

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

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
        
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glUniformMatrix4fv(loc_mvp, 1, GL_FALSE, glm::value_ptr(mvp1));
        glDrawArrays(GL_TRIANGLES, 0, 6*(N-1));

        glm::mat4 modelview2( 1.0f );
        modelview2 = glm::translate(modelview2, glm::vec3(0.6f, 0.0f, 0.0f) );
        modelview2 = glm::scale(modelview2, glm::vec3(0.5f, 0.5f, 1.0f) );
        glm::mat4 mvp2 = project * modelview2;
        
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glUniformMatrix4fv(loc_mvp, 1, GL_FALSE, glm::value_ptr(mvp2));
        glDrawArrays(GL_TRIANGLES, 0, 6*(N-1));
        
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glfwTerminate();

    return 0;
}
