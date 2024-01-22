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

// GLFW [https://www.glfw.org/]
#include <GLFW/glfw3.h>

// project includes
#include <gl/gl_debug.h>
#include <gl/gl_shader.h>

// FreeType
#include <ft2build.h>
#include FT_FREETYPE_H

// preprocessor definitions

#ifndef M_PI
#define M_PI 3.14159265358979323846 
#endif

const char* fontfilename = "./resource/font/FreeSans.ttf";

// shader

std::string sh_vert = R"(
#version 410 core

layout (location = 0) in vec4 in_attr;

out vec2 vUV;

uniform mat4 projection;
uniform mat4 model;

void main()
{
    vUV         = in_attr.zw;
	gl_Position = projection * model * vec4(in_attr.xy, 0.0, 1.0);
}
)";

std::string sh_frag = R"(
#version 410 core

in vec2 vUV;

uniform sampler2D u_texture;

uniform vec3 textColor;

out vec4 fragColor;

void main()
{
    vec2 uv = vUV.xy;
    float text = texture(u_texture, uv).r;
    fragColor = vec4(textColor.rgb*text, text);
}
)";

/// Holds all state information relevant to a character as loaded using FreeType
struct Character {
    GLuint TextureID;   // ID handle of the glyph texture
    glm::ivec2 Size;    // Size of glyph
    glm::ivec2 Bearing;  // Offset from baseline to left/top of glyph
    GLuint Advance;    // Horizontal offset to advance to next glyph
};

std::map<GLchar, Character> Characters;
GLuint VAO, VBO;

void RenderText1(OpenGL::ShaderProgramSimple& shader, std::string text, GLfloat x, GLfloat y, GLfloat scale, glm::vec3 color);
void RenderText2(OpenGL::ShaderProgramSimple& shader, std::string text, GLfloat x, GLfloat y, GLfloat scale, glm::vec3 color, glm::vec2 dir);

// The MAIN function, from here we start our application and run the Game loop
int main()
{
    // Init GLFW
    glfwInit();
#ifdef __APPLE__
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
    glewExperimental = true;
#endif
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

    GLFWwindow* window = glfwCreateWindow(800, 600, "LearnOpenGL", nullptr, nullptr); // Windowed
    glfwMakeContextCurrent(window);

    // Initialize GLEW to setup the OpenGL Function pointers
    glewExperimental = GL_TRUE;
    glewInit();

    int vpSize[2];
    glfwGetFramebufferSize(window, &vpSize[0], &vpSize[1]);
    float dpiScale = static_cast<GLfloat>(vpSize[0]) / 800.0f;

    // Define the viewport dimensions
    glViewport(0, 0, vpSize[0], vpSize[1]);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Compile and setup the shader
    OpenGL::ShaderProgramSimple shader(
    {
        { sh_vert, GL_VERTEX_SHADER },
        { sh_frag, GL_FRAGMENT_SHADER }
    });

    glm::mat4 projection = glm::ortho(0.0f, static_cast<GLfloat>(vpSize[0]), static_cast<GLfloat>(vpSize[1]), 0.0f);

    shader.Use();
    GLint project_loc = glGetUniformLocation(shader.Prog(), "projection");
    glUniformMatrix4fv(project_loc, 1, GL_FALSE, glm::value_ptr(projection));

    // FreeType
    FT_Library ft;
    // All functions return a value different than 0 whenever an error occurred
    if (FT_Init_FreeType(&ft))
        std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;

    // Load font as face
    FT_Face face;
    if (FT_New_Face(ft, fontfilename, 0, &face))
        std::cout << "ERROR::FREETYPE: Failed to load font" << std::endl;

    // Set size to load glyphs as
    FT_Set_Pixel_Sizes(face, 0, 48);

    // Disable byte-alignment restriction
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    // Load first 128 characters of ASCII set
    for (GLubyte c = 0; c < 128; c++)
    {
        // Load character glyph 
        if (FT_Load_Char(face, c, FT_LOAD_RENDER))
        {
            std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
            continue;
        }
        // Generate texture
        GLuint texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_RED,
            face->glyph->bitmap.width,
            face->glyph->bitmap.rows,
            0,
            GL_RED,
            GL_UNSIGNED_BYTE,
            face->glyph->bitmap.buffer
        );
        // Set texture options
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        // Now store character for later use
        Character character = {
            texture,
            glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
            glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
            static_cast<GLuint>(face->glyph->advance.x)
        };
        Characters.insert(std::pair<GLchar, Character>(c, character));
    }
    glBindTexture(GL_TEXTURE_2D, 0);
    // Destroy FreeType once we're finished
    FT_Done_Face(face);
    FT_Done_FreeType(ft);


    // Configure VAO/VBO for texture quads
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // Game loop
    while (!glfwWindowShouldClose(window))
    {
        // Check and call events
        glfwPollEvents();

        // Clear the colorbuffer
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        RenderText1(shader, "This is sample text", dpiScale * 25.0f, dpiScale * 50.0f, dpiScale, glm::vec3(0.5, 0.8f, 0.2f));
        RenderText2(shader, "(C) LearnOpenGL.com", dpiScale * 35.0f, dpiScale * 200.0f, dpiScale * 0.8f, glm::vec3(0.3, 0.7f, 0.9f), glm::vec2(1.0f, -0.25f));

        // Swap the buffers
        glfwSwapBuffers(window);
    }

    glfwTerminate();
    return 0;
}


void RenderText2(OpenGL::ShaderProgramSimple& shader, std::string text, GLfloat x, GLfloat y, GLfloat scale, glm::vec3 color, glm::vec2 dir)
{
    // Activate corresponding render state	
    shader.Use();
    glUniform3f(glGetUniformLocation(shader.Prog(), "textColor"), color.x, color.y, color.z);
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(VAO);

    GLfloat vertices[6][4] = {
        { 0.0, -1.0,   0.0, 0.0 },
        { 0.0,  0.0,   0.0, 1.0 },
        { 1.0,  0.0,   1.0, 1.0 },

        { 0.0, -1.0,   0.0, 0.0 },
        { 1.0,  0.0,   1.0, 1.0 },
        { 1.0, -1.0,   1.0, 0.0 }
    };

    // Update content of VBO memory
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices); // Be sure to use glBufferSubData and not glBufferData
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    GLfloat   angle_rad = atan2(dir.y, dir.x);
    glm::mat4 rotateM = glm::rotate(glm::mat4(1.0f), angle_rad, glm::vec3(0.0f, 0.0f, 1.0f));
    glm::mat4 transOriginM = glm::translate(glm::mat4(1.0f), glm::vec3(x, y, 0.0f));

    // Iterate through all characters
    std::string::const_iterator c;
    GLfloat char_x = 0.0f;
    for (c = text.begin(); c != text.end(); c++)
    {
        Character ch = Characters[*c];

        GLfloat w = ch.Size.x * scale;
        GLfloat h = ch.Size.y * scale;
        GLfloat xrel = char_x + ch.Bearing.x * scale;
        GLfloat yrel = (ch.Size.y - ch.Bearing.y) * scale;

        // Now advance cursors for next glyph (note that advance is number of 1/64 pixels)
        char_x += (ch.Advance >> 6) * scale; // Bitshift by 6 to get value in pixels (2^6 = 64 (divide amount of 1/64th pixels by 64 to get amount of pixels))

        glm::mat4 scaleM = glm::scale(glm::mat4(1.0f), glm::vec3(w, h, 1.0f));
        glm::mat4 transRelM = glm::translate(glm::mat4(1.0f), glm::vec3(xrel, yrel, 0.0f));

        glm::mat4 modelM = transOriginM * rotateM * transRelM * scaleM;
        GLint model_loc = glGetUniformLocation(shader.Prog(), "model");
        glUniformMatrix4fv(model_loc, 1, GL_FALSE, glm::value_ptr(modelM));

        // Render glyph texture over quad
        glBindTexture(GL_TEXTURE_2D, ch.TextureID);

        // Render quad
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}


void RenderText1(OpenGL::ShaderProgramSimple& shader, std::string text, GLfloat x, GLfloat y, GLfloat scale, glm::vec3 color)
{
    static GLfloat test_scale = 1.0f;
    glm::mat4 modelM = glm::scale(glm::mat4(1.0f), glm::vec3(test_scale, test_scale, 1.0f));

    // Activate corresponding render state	
    shader.Use();
    GLint model_loc = glGetUniformLocation(shader.Prog(), "model");
    glUniformMatrix4fv(model_loc, 1, GL_FALSE, glm::value_ptr(modelM));
    glUniform3f(glGetUniformLocation(shader.Prog(), "textColor"), color.x, color.y, color.z);
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(VAO);

    // Iterate through all characters
    std::string::const_iterator c;
    for (c = text.begin(); c != text.end(); c++)
    {
        Character ch = Characters[*c];

        GLfloat xpos = x + ch.Bearing.x * scale;
        GLfloat ypos = y + (ch.Size.y - ch.Bearing.y) * scale;

        GLfloat w = ch.Size.x * scale;
        GLfloat h = ch.Size.y * scale;

        GLfloat vertices[6][4] = {
            { xpos,     ypos - h, 0.0, 0.0 },
            { xpos,     ypos,     0.0, 1.0 },
            { xpos + w, ypos,     1.0, 1.0 },

            { xpos,     ypos - h,  0.0, 0.0 },
            { xpos + w, ypos,      1.0, 1.0 },
            { xpos + w, ypos - h,  1.0, 0.0 }
        };
        // Render glyph texture over quad
        glBindTexture(GL_TEXTURE_2D, ch.TextureID);
        // Update content of VBO memory
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices); // Be sure to use glBufferSubData and not glBufferData

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        // Render quad
        glDrawArrays(GL_TRIANGLES, 0, 6);
        // Now advance cursors for next glyph (note that advance is number of 1/64 pixels)
        x += (ch.Advance >> 6) * scale; // Bitshift by 6 to get value in pixels (2^6 = 64 (divide amount of 1/64th pixels by 64 to get amount of pixels))
    }
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}