#include <stdafx.h>

// OpenGL
#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glu.h>

// glm
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// freeglut
#include <GL/freeglut.h>

// stl
#include <vector>
#include <stdexcept>
#include <chrono>
#include <memory>
#include <cmath>
#include <iostream>

#define _USE_MATH_DEFINES
#include <math.h>


//#define _CREATE_DOCUMENTATION 

std::chrono::high_resolution_clock::time_point g_start_time;

bool valid_viewport = false;

void OnIdle( void );
void Resize( int, int );
void Display( void );

#if defined(_CREATE_DOCUMENTATION)
int vp_lrbt[4]{ -2, 4, -2, 2 };
int wnd_width = 300;
#else
int vp_lrbt[4]{ -5, 5, -5, 5 };
int wnd_width = 500;
#endif



int main(int argc, char** argv)
{
    int vp_cx = abs(vp_lrbt[1] - vp_lrbt[0]) + 1;
    int vp_cy = abs(vp_lrbt[3] - vp_lrbt[2]) + 1;
    int wnd_cy = wnd_width;
    int wnd_cx = wnd_cy * vp_cx / vp_cy;

    // Initialise FreeGLUT
    glutInit(&argc, argv);

    // Setup OpenGL window properties
    glutInitContextProfile(GLUT_COMPATIBILITY_PROFILE);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);

    // Create window
    glutInitWindowSize(wnd_cx, wnd_cy);
    int wnd = glutCreateWindow("test");
    if ( wnd == 0 )
        throw std::runtime_error( "error initializing window" ); 

    // Register display callback function
    glutDisplayFunc( Display ); 
    glutReshapeFunc( Resize );
    glutIdleFunc( OnIdle );

    glewExperimental = true;
    if ( glewInit() != GLEW_OK )
        throw std::runtime_error( "error initializing glew" );     

    std::cout << glGetString( GL_VENDOR ) << std::endl;
    std::cout << glGetString( GL_RENDERER ) << std::endl;
    std::cout << glGetString( GL_VERSION ) << std::endl;
    std::cout << glGetString( GL_SHADING_LANGUAGE_VERSION ) << std::endl;

    GLint major = 0, minor = 0, contex_mask = 0;
    glGetIntegerv(GL_MAJOR_VERSION, &major);
    glGetIntegerv(GL_MINOR_VERSION, &minor);
    glGetIntegerv(GL_CONTEXT_PROFILE_MASK, &contex_mask);
    std::cout << "context: " << major << "." << minor << " ";
    if ( contex_mask & GL_CONTEXT_CORE_PROFILE_BIT  )
      std::cout << "core";
    else if ( contex_mask & GL_CONTEXT_COMPATIBILITY_PROFILE_BIT )
      std::cout << "compatibility";
    if ( contex_mask & GL_CONTEXT_FLAG_FORWARD_COMPATIBLE_BIT  )
      std::cout << ", forward compatibility";
    if ( contex_mask & GL_CONTEXT_FLAG_ROBUST_ACCESS_BIT  )
      std::cout << ", robust access";
    if ( contex_mask & GL_CONTEXT_FLAG_DEBUG_BIT  )
      std::cout << ", debug";
    std::cout << std::endl;
    
    // extensions
    //std::cout << glGetStringi( GL_EXTENSIONS, ... ) << std::endl;

    std::cout << std::endl;

    // setup orthographic projection
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
    glOrtho( vp_lrbt[0]-0.5f, vp_lrbt[1]+0.5f, vp_lrbt[2]-0.5f, vp_lrbt[3]+0.5f, -1.0f, 1.0f );
    glMatrixMode( GL_MODELVIEW );

    g_start_time = std::chrono::high_resolution_clock::now();
    glutMainLoop();

    glutDestroyWindow(wnd);
    return 0;
}

void OnIdle( void )
{
  glutPostRedisplay();
  // ....
}

void Resize( int cx, int cy )
{
  valid_viewport = false;
  // ....
}


void Display( void )
{
    std::chrono::high_resolution_clock::time_point current_time = std::chrono::high_resolution_clock::now();
    auto   delta_time = current_time - g_start_time;
    double time_ms    = (double)std::chrono::duration_cast<std::chrono::milliseconds>(delta_time).count();

    static double interval_ms    = 5000.0; // 5 seconds
    static double shoulder_speed = 1.03;
    static double elbow_speed    = 2.17;

    double time_val = time_ms / interval_ms;

#if defined(_CREATE_DOCUMENTATION)
    time_val = 0.0;
#endif

    float shoulder_ang_deg = (float)(-15.0 + time_val * 360.0 * shoulder_speed);
    float elbow_ang_deg    = (float)( 45.0 + time_val * 360.0 * elbow_speed);

    if ( valid_viewport == false )
    {
      int vpSize[2]
      {
          glutGet(GLUT_WINDOW_WIDTH),
          glutGet(GLUT_WINDOW_HEIGHT)
      };
      glViewport( 0, 0, vpSize[0], vpSize[1] );
      valid_viewport = true;
    }

    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();

    glClearColor(0.95f, 0.95f, 0.92f, 1.0f);  
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
     
    glLineWidth( 1.0f );
    glColor4f( 0.1f, 0.8f, 0.8f, 1.0f );

    glBegin( GL_LINES );
    for ( int i = vp_lrbt[0]; i <= vp_lrbt[1]; ++ i)
    {
      glVertex2f( (float)i, (float)vp_lrbt[2]-1.0f );
      glVertex2f( (float)i, (float)vp_lrbt[3]+1.0f );
    }
    for ( int i = vp_lrbt[2]; i <= vp_lrbt[3]; ++ i)
    {
      glVertex2f( (float)vp_lrbt[0]-1.0f, (float)i );
      glVertex2f( (float)vp_lrbt[1]+1.0f, (float)i );
    }
    glEnd();


    glLineWidth( 5.0f );
    glColor4f( 0.1f, 0.3f, 0.8f, 1.0f );
   
    glPushMatrix();
   
    glTranslatef(-1.0f, 0.0f, 0.0f);
    glRotatef(shoulder_ang_deg, 0.0f, 0.0f, 1.0f);
    glTranslatef(1.0f, 0.0f, 0.0f);
    glPushMatrix();
    glScalef(2.0f, 0.4f, 1.0f);
    glutWireCube(1.0f);
    glPopMatrix();    

    glTranslatef(1.0f, 0.0f, 0.0f);
    glRotatef(elbow_ang_deg, 0.0f, 0.0f, 1.0f);
    glTranslatef(1.0f, 0.0f, 0.0f);
    glPushMatrix();
    glScalef(2.0f, 0.4f, 1.0f);
    glutWireCube(1.0f);
    glPopMatrix();

    glPopMatrix();

    glLineWidth( 3.0f );
    glColor4f( 0.9f, 0.4f, 0.3f, 1.0f );
    glBegin( GL_LINES );
    glVertex2f( 0.0f, -0.25f );
    glVertex2f( 0.0f, 0.25f );
    glVertex2f( -0.25f, 0.0f );
    glVertex2f( 0.25f, 0.0f );
    glEnd();

    glutSwapBuffers();
}