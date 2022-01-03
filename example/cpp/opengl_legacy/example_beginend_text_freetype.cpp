#include <stdafx.h>

// OpenGL
#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glu.h>

// glad
//#include <glad/glad.h>

// glm
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// glfw
#include <GLFW/glfw3.h>

// stb 
#define STB_IMAGE_IMPLEMENTATION
//#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image.h>
//#include <stb_image_write.h>

// FreeType
#include <ft2build.h>
#include FT_FREETYPE_H

// stl
#include <vector>
#include <stdexcept>
#include <iostream>
#include <chrono>
#include <map>

#define _USE_MATH_DEFINES
#include <cmath>
#include <math.h>

// project includes
#include <gl/gl_debug.h>

// preprocessor definitions

#ifndef M_PI
#define M_PI 3.14159265358979323846 
#endif

const char* fontfilename = "../../../../../../resource/font/FreeSans.ttf";

// Properties
const GLuint WIDTH = 800, HEIGHT = 600;

/// Holds all state information relevant to a character as loaded using FreeType
struct Character {
    GLuint TextureID;   // ID handle of the glyph texture
    glm::ivec2 Size;    // Size of glyph
    glm::ivec2 Bearing;  // Offset from baseline to left/top of glyph
    GLuint Advance;    // Horizontal offset to advance to next glyph
};

std::map<GLchar, Character> Characters;
GLuint VAO, VBO;

void RenderText(std::string text, GLfloat x, GLfloat y, GLfloat scale, glm::vec3 color);

// The MAIN function, from here we start our application and run the Game loop
int main()
{
    // Init GLFW
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "LearnOpenGL", nullptr, nullptr); // Windowed
    glfwMakeContextCurrent(window);

    // Initialize GLEW to setup the OpenGL Function pointers
    glewExperimental = GL_TRUE;
    glewInit();

    // Define the viewport dimensions
    glViewport(0, 0, WIDTH, HEIGHT);

    // Set OpenGL options
    //glCullFace( GL_BACK );
    //glFrontFace( GL_CW );
    glEnable(GL_CULL_FACE);


    //glEnable(GL_BLEND);
    //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    //glm::mat4 projection = glm::ortho(0.0f, static_cast<GLfloat>(WIDTH), 0.0f, static_cast<GLfloat>(HEIGHT));

    glm::mat4 projection = glm::ortho(0.0f, static_cast<GLfloat>(WIDTH), static_cast<GLfloat>(HEIGHT), 0.0f);

    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf(glm::value_ptr(projection));
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // FreeType
    FT_Library ft;
    // All functions return a value different than 0 whenever an error occurred
    if (FT_Init_FreeType(&ft))
        std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;

    // Load font as face
    FT_Face face;
    //if (FT_New_Face(ft, "fonts/arial.ttf", 0, &face))
    if (FT_New_Face(ft, "font/FreeSans.ttf", 0, &face))
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
            GL_LUMINANCE,
            face->glyph->bitmap.width,
            face->glyph->bitmap.rows,
            0,
            GL_LUMINANCE,
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
            face->glyph->advance.x
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

    glVertexPointer(2, GL_FLOAT, 4 * sizeof(GLfloat), nullptr);
    glEnableClientState(GL_VERTEX_ARRAY);
    glTexCoordPointer(2, GL_FLOAT, 4 * sizeof(GLfloat), (void*)(2 * sizeof(GLfloat)));
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // Game loop
    while (!glfwWindowShouldClose(window))
    {
        // Check and call events
        glfwPollEvents();

        // Clear the color buffer
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        RenderText("This is sample text", 25.0f, 50.0f, 1.0f, glm::vec3(0.5, 0.8f, 0.2f));
        RenderText("(C) LearnOpenGL.com", 540.0f, 570.0f, 0.5f, glm::vec3(0.3, 0.7f, 0.9f));

        // Swap the buffers
        glfwSwapBuffers(window);
    }

    glfwTerminate();
    return 0;
}

void RenderText(std::string text, GLfloat x, GLfloat y, GLfloat scale, glm::vec3 color)
{
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_COLOR, GL_ONE_MINUS_SRC_COLOR);

    glEnable(GL_TEXTURE_2D);

    // Activate corresponding render state	
    glColor4f(color.x, color.y, color.z, 1.0f);
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

    glDisable(GL_TEXTURE_2D);
    glDisable(GL_BLEND);
}