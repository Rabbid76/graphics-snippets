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

// function prototypes

void display( void );

// main


// Using VBO to draw the cube by loading wavefront object
// https://stackoverflow.com/questions/50376506/using-vbo-to-draw-the-cube-by-loading-wavefront-object/50376679#50376679


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

    // 8  vertex coordiantes: 8 * 3 float 
    std::vector<float> v{
      -1.0f, -1.0f,  1.0f,
      -1.0f,  1.0f,  1.0f,
      -1.0f, -1.0f, -1.0f,
      -1.0f,  1.0f, -1.0f,
       1.0f, -1.0f,  1.0f,
       1.0f,  1.0f,  1.0f,
       1.0f, -1.0f, -1.0f,
       1.0f,  1.0f, -1.0f };

    // 6  normal vectors: 6 * 3 float
    std::vector<float> nv{
       -1.0f,  0.0f,  0.0f,
        0.0f,  0.0f, -1.0f,
        1.0f,  0.0f,  0.0f,
        0.0f,  0.0f,  1.0f,
        0.0f, -1.0f,  0.0f,
        0.0f,  1.0f,  0.0f };

    // 12 colors coordiantes: 12 * 3 float 
    std::vector<float> c{
        0.583f,  0.771f,  0.014f,
        0.609f,  0.115f,  0.436f,
        0.327f,  0.483f,  0.844f,
        0.822f,  0.569f,  0.201f,
        0.435f,  0.602f,  0.223f,
        0.310f,  0.747f,  0.185f,
        0.597f,  0.770f,  0.761f,
        0.559f,  0.436f,  0.730f,
        0.359f,  0.583f,  0.152f,
        0.483f,  0.596f,  0.789f,
        0.559f,  0.861f,  0.639f,
        0.195f,  0.548f,  0.859f };

    // 12 faces 3*2 indices/face: 12 * 3 * 2 unsigned int 
    std::vector<unsigned int> indices{
        2, 1,    3, 1,    1, 1,
        4, 2,    7, 2,    3, 2,
        8, 3,    5, 3,    7, 3,
        6, 4,    1, 4,    5, 4,
        7, 5,    1, 5,    3, 5,
        4, 6,    6, 6,    8, 6,
        2, 1,    4, 1,    3, 1,
        4, 2,    8, 2,    7, 2,
        8, 3,    6, 3,    5, 3,
        6, 4,    2, 4,    1, 4,
        7, 5,    5, 5,    1, 5,
        4, 6,    2, 6,    6, 6 };

    // final vertex attributes 12 * 3 *(3 + 3 + 3) floats
    // x0 y0 z0    nvx0 nvy0 nvz0    cr0 cg0 cb0
    // x1 y1 z1    nvx1 nvy1 nvz1    cr1 cg1 cb1
    std::vector<float> va; 

    const unsigned int no_of_faces = 12;
    for (unsigned int f=0; f<no_of_faces; ++f )
    {
        for (unsigned int t=0; t<3; ++t )
        {
            unsigned int vi = indices[(f*3+t)*2]-1;   // vertex index
            unsigned int ni = indices[(f*3+t)*2+1]-1; // normal vector index
            unsigned int ci = f;                      // color index

            va.insert(va.end(), v.begin()  + vi*3, v.begin()  + vi*3+3); // insert vertex coordinate
            va.insert(va.end(), nv.begin() + ni*3, nv.begin() + ni*3+3); // insert normal vector
            va.insert(va.end(), c.begin()  + ci*3, c.begin()  + ci*3+3); // insert color
        }
    }

    GLuint vbo;
    glGenBuffers( 1, &vbo );
    glBindBuffer( GL_ARRAY_BUFFER, vbo );
    glBufferData( GL_ARRAY_BUFFER, va.size()*sizeof(*va.data()), va.data(), GL_STATIC_DRAW );

    glVertexPointer( 3, GL_FLOAT, 9*sizeof(*va.data()), 0 );
    glEnableClientState( GL_VERTEX_ARRAY );
    glNormalPointer( GL_FLOAT, 9*sizeof(*va.data()), (void*)(3*sizeof(*va.data())) ); 
    glEnableClientState( GL_NORMAL_ARRAY );
    glColorPointer( 3, GL_FLOAT, 9*sizeof(*va.data()), (void*)(6*sizeof(*va.data())) ); 
    glEnableClientState( GL_COLOR_ARRAY );
    glBindBuffer( GL_ARRAY_BUFFER, 0 );

    glEnable( GL_DEPTH_TEST );

    glutMainLoop();

    std::cout << "terminate" << std::endl;
    glutDestroyWindow(wnd);
    return 0;
}

float angle = 0.0f;

void display( void )
{
    static float angle1 = 0.0f;
    static float angle2 = 0.0f;
    static float angle3 = 0.0f;

    int vpSize[2]
    {
        glutGet(GLUT_WINDOW_WIDTH),
        glutGet(GLUT_WINDOW_HEIGHT)
    };
    glViewport( 0, 0, vpSize[0], vpSize[1] );
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);  
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

    float aspect = (float)vpSize[0]/(float)vpSize[1];

    glm::mat4 project( 1.0f );
    project = glm::perspective( (float)M_PI / 3.0f, aspect, 0.1f, 10.0f );

    glm::mat4 view( 1.0f );
    view = glm::lookAt( glm::vec3( 0.0f, -5.0f, 0.6f ), glm::vec3( 0.0f, 0.0f, 0.6f ), glm::vec3( 0.0f, 0.0f, 1.0f ) );

    glm::mat4 model( 1.0f );
    model = glm::translate( model, glm::vec3(0.1f, 0.0f, 1.0f) );
    model = glm::rotate( model, glm::radians(angle1), glm::vec3(1.0f, 0.0f, 0.0f) );
    angle1 += 0.13f*5.0f;
    model = glm::rotate( model, glm::radians(angle2), glm::vec3(0.0f, 1.0f, 0.0f) );
    angle2 += 0.03f*5.0f;
    model = glm::rotate( model, glm::radians(angle2), glm::vec3(0.0f, 0.0f, 1.0f) );
    angle2 += 0.23f*5.0f;
    glm::mat4 model_view = view * model;

    glMatrixMode( GL_PROJECTION );
    glLoadMatrixf( glm::value_ptr(project) );

    glMatrixMode( GL_MODELVIEW );
    glLoadMatrixf( glm::value_ptr(model_view) );
      
    glDrawArrays( GL_TRIANGLES, 0, 36 );

    glutSwapBuffers();
    glutPostRedisplay();
}
