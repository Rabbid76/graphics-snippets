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

// STB 
#define STB_IMAGE_IMPLEMENTATION
//#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image.h>
//#include <stb_image_write.h>

// project includes
#include <gl/gl_debug.h>

// preprocessor definitions

#ifndef M_PI
#define M_PI 3.14159265358979323846 
#endif

// forward declarations

void display( void );


// globale variables

std::chrono::high_resolution_clock::time_point start_time;

GLuint tobj;


// main

int main(int argc, char** argv)
{
    std::cout << "start" << std::endl;

    glutInit(&argc, argv);

    glutInitContextProfile(GLUT_COMPATIBILITY_PROFILE);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);

    glutInitWindowSize(800, 600);
    int wnd = glutCreateWindow("FreeGLUT OpenGL window");

    if ( glewInit() != GLEW_OK )
        throw std::runtime_error( "error initializing glew" );

    OpenGL::CContext::TDebugLevel debug_level = OpenGL::CContext::TDebugLevel::all;
    OpenGL::CContext context;
    context.Init( debug_level );

    glutDisplayFunc(display);  

    std::string path = "C:/source/rabbid76workbench/test/texture/boomerang.png";
    //std::string path = "C:/source/graphics-snippets/resource/texture/supermario.jpg";
    int cx, cy, ch;
    stbi_uc *img = stbi_load( path.c_str(), &cx, &cy, &ch, 4 );
    if ( img == nullptr )
        std::cout << "error loading texture" << std::endl;

    float values[4]{ 0.f, 1.f, 0.f, 1.f };

    glGenTextures(1, &tobj);
    glActiveTexture( GL_TEXTURE0 );
    glBindTexture(GL_TEXTURE_2D, tobj);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);  //for BMP
    
    //glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 2, 2, 0, GL_RED, GL_FLOAT, values);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, cx, cy, 0, GL_RGBA, GL_UNSIGNED_BYTE, img);
    stbi_image_free( img );

    //glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE  );
    //glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE  );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    //glGenerateMipmap( GL_TEXTURE_2D );
    
    //glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_G, GL_RED );
    //glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_B, GL_RED );
          
    glBindTexture( GL_TEXTURE_2D, 0 );                                         

    start_time = std::chrono::high_resolution_clock::now();
 
    std::cout << "main loop" << std::endl;
    glutMainLoop(); 

    std::cout << "terminate" << std::endl;
    glutDestroyWindow(wnd);
    return 0;    
} 

void display( void )
{
    static float angle = 1.0f;

    int vpSize[2]
    {
        glutGet(GLUT_WINDOW_WIDTH),
        glutGet(GLUT_WINDOW_HEIGHT)
    };
    float ascpect = (float)vpSize[0] / (float)vpSize[1];
    float orthoX = ascpect > 1.0f ? ascpect : 1.0f;
    float orthoY = ascpect > 1.0f ? 1.0f : ascpect;

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    //glOrtho(-orthoX, orthoX, orthoY, -orthoY, -1.0, 1.0);
    glOrtho( 0.0f, vpSize[0], vpSize[1], 0.0f, -1.0, 1.0);

    glViewport( 0, 0, vpSize[0], vpSize[1] );
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
      
    //GLfloat edgeLength = 1.0f;
    //GLfloat PosX = 0.0f;
    //GLfloat PosY = 0.0f;
    //GLfloat PosZ = 0.0f;
    GLfloat edgeLength = 200.0f;
    GLfloat PosX = vpSize[0] / 2.0f;
    GLfloat PosY = vpSize[1] / 2.0f;
    GLfloat PosZ = 0.0f;
    GLfloat halfSideLength = edgeLength * 0.5f;
    GLfloat vertices[] =
    {
        PosX - halfSideLength, PosY - halfSideLength, PosZ, // bottom left
        PosX + halfSideLength, PosY - halfSideLength, PosZ, // bottom right
        PosX + halfSideLength, PosY + halfSideLength, PosZ, // top right
        PosX - halfSideLength, PosY + halfSideLength, PosZ // top left
    };
    GLfloat colour[] =
    {
        1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f
    };
    GLfloat map2D[] = {
        0.0f, 0.0f,
        1.0f, 0.0f,
        1.0f, 1.0f,
        0.0f, 1.0f
    };

    glColor4f( 1.0f, 1.0f, 1.0f, 1.0f );
    glEnable(GL_TEXTURE_2D);
    //glBindTexture(GL_TEXTURE_2D, textures[1]);
    glBindTexture(GL_TEXTURE_2D, tobj);
    //glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
    glEnableClientState( GL_VERTEX_ARRAY );
    glEnableClientState( GL_COLOR_ARRAY );
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glVertexPointer( 3, GL_FLOAT, 0, vertices );
    glTexCoordPointer( 2, GL_FLOAT, 0, map2D );
    glColorPointer( 3, GL_FLOAT, 0, colour );
    glDrawArrays( GL_QUADS, 0, 4 );
    glDisableClientState( GL_COLOR_ARRAY );
    glDisableClientState(GL_TEXTURE_2D_ARRAY);
    glDisableClientState( GL_VERTEX_ARRAY );
    glDisable(GL_TEXTURE_2D);

    glutSwapBuffers();
    glutPostRedisplay();
} 
