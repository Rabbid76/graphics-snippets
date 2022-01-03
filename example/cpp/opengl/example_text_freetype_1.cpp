// STL
#include <iostream>
#include <string>
#include <stdexcept>
#include <chrono>


// GLM [https://glm.g-truc.net/0.9.9/api/index.html]
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


// OpenGL
#include <gl/gl_glew.h>


// FreeGLUT [http://freeglut.sourceforge.net/]
#include <gl/freeglut.h>


// project includes
#include <gl/gl_debug.h>
#include <gl/gl_shader.h>

#include <ft2build.h>
#include FT_FREETYPE_H

// preprocessor definitions

#ifndef M_PI
#define M_PI 3.14159265358979323846 
#endif

#define MAIN_LOOP_EVENT 1

// prototypes

void display(void);

// shader

std::string sh_vert = R"(
#version 460 core

layout (location = 0) in vec4 in_attr;

out vec2 uv;

uniform mat4 project;
uniform mat4 view;
uniform mat4 model;

void main()
{  
    vec4 view_pos = view * model * vec4(in_attr.xy, 0.0, 1.0);

    uv = in_attr.zw;
    gl_Position = project * vec4(view_pos.xyz, 1.0);
}
)";

std::string sh_frag = R"(
#version 460 core

out vec4 fragColor;

in vec2 uv;

uniform sampler2D myTextureSampler;
uniform vec4 texcol;

void main()
{
    fragColor = texcol * texture(myTextureSampler, uv).r;
}
)";


std::chrono::high_resolution_clock::time_point start_time;
OpenGL::ShaderProgramSimple* g_prog;
FT_Library library;
FT_Face face;
GLuint vbo00;
const char* fontfilename = "../../../../../../resource/font/FreeSans.ttf";

void init()
{
    g_prog = new OpenGL::ShaderProgramSimple(
    {
        { sh_vert, GL_VERTEX_SHADER },
        { sh_frag, GL_FRAGMENT_SHADER }
    });

    if (FT_Init_FreeType(&library)) 
    {
        fprintf(stderr, "Could not init freetype library\n");
    }

    if (FT_New_Face(library, fontfilename, 0, &face)) 
    {
        fprintf(stderr, "Could not open font %s\n", fontfilename);
    }

    glGenBuffers(1, &vbo00);
}

void render_text(const char* text, float x, float y, float sx, float sy) {
    const char* p;
    FT_GlyphSlot g = face->glyph;

    glActiveTexture(GL_TEXTURE0);

    GLuint tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    g_prog->SetUniformI1("myTextureSampler", 0);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, vbo00);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);

    for (p = text; *p; p++) 
    {
        if (FT_Load_Char(face, *p, FT_LOAD_RENDER))
            continue;

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED,
            g->bitmap.width, g->bitmap.rows, 0,
            GL_RED, GL_UNSIGNED_BYTE, g->bitmap.buffer);

        float x2 = x + g->bitmap_left * sx;
        float y2 = -y - g->bitmap_top * sy;
        float w = g->bitmap.width * sx;
        float h = g->bitmap.rows * sy;

        GLfloat box[4][4] = {
            {x2, -y2, 0, 0},
            {x2 + w, -y2, 1, 0},
            {x2, -y2 - h, 0, 1},
            {x2 + w, -y2 - h, 1, 1},
        };

        auto bs = sizeof box;
        glBufferData(GL_ARRAY_BUFFER, bs, box, GL_DYNAMIC_DRAW);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        x += (g->advance.x >> 6) * sx;
        y += (g->advance.y >> 6) * sy;
    }

    glDisableVertexAttribArray(0);
    glDeleteTextures(1, &tex);

}

void display(void)
{
    static float angle = 0.0f;

    int vpSize[2]
    {
        glutGet(GLUT_WINDOW_WIDTH),
        glutGet(GLUT_WINDOW_HEIGHT)
    };
    float ascpect = (float)vpSize[0] / (float)vpSize[1];
    float orthoX = ascpect > 1.0f ? ascpect : 1.0f;
    float orthoY = ascpect > 1.0f ? 1.0f : ascpect;

    //glm::mat4 project( 1.0f );
    glm::mat4 project = glm::ortho(-orthoX, orthoX, -orthoY, orthoY, -1.0f, 1.0f);
    //glm::mat4 project = glm::perspective(glm::radians(90.0f), ascpect, 0.01f, 4.0f);
    //glm::mat4 project = glm::ortho(0.0f, (float)vpSize[0], (float)vpSize[1], 0.0f, -1.0f, 1.0f );

    glm::mat4 view(1.0f);
    //view = glm::lookAt(glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

    glm::mat4 model(1.0f);
    model = glm::translate(model, glm::vec3(-orthoX * 0.9f, 0.0f, 0.0f));
    //model = glm::rotate( model, glm::radians(angle), glm::vec3(0.0f, 0.0f, 1.0f) );
    model = glm::scale(model, glm::vec3(10.0f, 10.0f, 1.0f));
    angle += 1.0f;

    g_prog->Use();
    g_prog->SetUniformM44("project", glm::value_ptr(project));
    g_prog->SetUniformM44("view", glm::value_ptr(view));
    g_prog->SetUniformM44("model", glm::value_ptr(model));

    glViewport(0, 0, vpSize[0], vpSize[1]);

    glClearColor(1, 1, 1, 1);
    glClear(GL_COLOR_BUFFER_BIT);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    float sx = 2.0f / 2304.0f / 4.0f;
    float sy = 2.0f / 1440.0f / 4.0f;
    FT_Set_Pixel_Sizes(face, 0, 48);

    g_prog->SetUniformF4("texcol", TVec4{ 1, 0, 0, 1 });
    render_text("The Quick Brown Fox Jumps Over The Lazy Dog", 0, 0, sx, sy);

    glutSwapBuffers();
    glutPostRedisplay();
}


// main

int main(int argc, char** argv)
{
    std::cout << "start" << std::endl;

    glutInit(&argc, argv);

    glutInitContextProfile(GLUT_COMPATIBILITY_PROFILE);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);

    glutInitWindowSize(800, 600);
    int wnd = glutCreateWindow("FreeGLUT OpenGL window");

    if (glewInit() != GLEW_OK)
        throw std::runtime_error("error initializing glew");

    OpenGL::CContext::TDebugLevel debug_level = OpenGL::CContext::TDebugLevel::all;
    OpenGL::CContext context;
    context.Init(debug_level);

    glutDisplayFunc(display);

    init();
    start_time = std::chrono::high_resolution_clock::now();
    std::cout << "main loop" << std::endl;

#if MAIN_LOOP_EVENT == 1
    int i = 0;
    while (i++ >= 0)
    {
        glutMainLoopEvent();
        glutPostRedisplay();
    }
#else
    glutMainLoop();
#endif

    std::cout << "terminate" << std::endl;
    glutDestroyWindow(wnd);
    return 0;
}
