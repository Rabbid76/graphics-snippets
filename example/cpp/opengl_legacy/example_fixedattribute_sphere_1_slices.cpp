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

// stl
#include <vector>
#include <iostream>
#include <stdexcept>
#include <string>
#include <algorithm>

#define _USE_MATH_DEFINES
#include <cmath>
#include <math.h>

// project includes
#include <gl/gl_debug.h>

// preprocessor definitions

#ifndef M_PI
#define M_PI 3.14159265358979323846 
#endif


void CreateSphereMeshLayer( int layers, int circumferenceTiles, float layer_scale, float layer_mix, std::vector<float> &va, std::vector<int> &ia )
{
    const float pi = 3.1414927f;

    va.reserve( (layers)*2*(circumferenceTiles+1)*10 );  // 5 floats: x, y, z, u, v 
    for ( int il = 0; il < layers; ++ il )
    {
        float layer_rel0 = (float)il / (float)layers;
        float layer_ang0 = (1.0f - 2.0f * layer_rel0) * pi/2.0f ;
        float layer_sin0 = std::sin( layer_ang0 );
        float layer_cos0 = std::cos( layer_ang0 );
        float layer_rel1 = (float)(il+1) / (float)layers;
        float layer_ang1 = (1.0f - 2.0f * layer_rel1) * pi/2.0f ;
        float layer_sin1 = std::sin( layer_ang1 );
        float layer_cos1 = std::cos( layer_ang1 );
        for ( int ic = 0; ic <= circumferenceTiles; ic ++ )
        {
            float circum_rel = (float)ic / (float)circumferenceTiles;
            float cricum_ang = circum_rel * 2.0f*pi - pi;
            float circum_sin = std::sin( cricum_ang );
            float circum_cos = std::cos( cricum_ang );

            float h0 = layer_sin0;
            float h1 = layer_sin1;
            float delta = h0 - h1;
            
            h0 = h0 * layer_scale;
            h1 = h0 - delta; 

            va.push_back( layer_cos0 * circum_cos ); // x
            va.push_back( layer_cos0 * circum_sin ); // y
            va.push_back( h0 );                      // z
            va.push_back( circum_rel );              // u
            va.push_back( 1.0f - layer_rel0 );       // v

            va.push_back( layer_cos1 * circum_cos ); // x
            va.push_back( layer_cos1 * circum_sin ); // y
            va.push_back( h1 );                      // z
            va.push_back( circum_rel );              // u
            va.push_back( 1.0f - layer_rel1 );       // v
        }
    }

    ia.reserve( layers*circumferenceTiles*6 );
    for ( int il = 0; il < layers; ++ il )
    {
        for ( int ic = 0; ic < circumferenceTiles; ic ++ )
        {
          int i0 = il * 2*(circumferenceTiles+1) + 2*ic;
          int i1 = i0 + 2;
          int i2 = i0 + 3;
          int i3 = i0 + 1;

          int faces[]{ i0, i1, i2, i0, i2, i3 };
          ia.insert(ia.end(), faces+(il==0?3:0), faces+(il==layers-1?3:6));
        }
    }
}


    void CreateSphereMesh( int layers, int circumferenceTiles, std::vector<float> &va, std::vector<int> &ia )
    {
        const float pi = 3.1414927f;

        // create the vertex attributes
        va.reserve( (layers+1)*(circumferenceTiles+1)*5 );  // 5 floats: x, y, z, u, v 
        for ( int il = 0; il <= layers; ++ il )
        {
            float layer_rel = (float)il / (float)layers;
            float layer_ang = (1.0f - 2.0f * layer_rel) * pi/2.0f ;
            float layer_sin = std::sin( layer_ang );
            float layer_cos = std::cos( layer_ang );
            for ( int ic = 0; ic <= circumferenceTiles; ic ++ )
            {
                float circum_rel = (float)ic / (float)circumferenceTiles;
                float cricum_ang = circum_rel * 2.0f*pi - pi;
                float circum_sin = std::sin( cricum_ang );
                float circum_cos = std::cos( cricum_ang );

                va.push_back( layer_cos * circum_cos ); // x
                va.push_back( layer_cos * circum_sin ); // y
                va.push_back( layer_sin );              // z
                va.push_back( circum_rel );             // u
                va.push_back( 1.0f - layer_rel );       // v
            }
        }

        // create the face indices 
        ia.reserve( layers*circumferenceTiles*6 );
        for ( int il = 0; il < layers; ++ il )
        {
            for ( int ic = 0; ic < circumferenceTiles; ic ++ )
            {
              int i0 = il * (circumferenceTiles+1) + ic;
              int i1 = i0 + 1;
              int i3 = i0 + circumferenceTiles+1;
              int i2 = i3 + 1;

              int faces[]{ i0, i1, i2, i0, i2, i3 };
              ia.insert(ia.end(), faces+(il==0?3:0), faces+(il==layers-1?3:6));
            }
        }
    }


int main(void)
{
    if ( glfwInit() == 0 )
        throw std::runtime_error( "error initializing glfw" );

    glfwWindowHint( GLFW_SAMPLES, 8 );
    GLFWwindow *wnd = glfwCreateWindow( 800, 600, "OGL window", nullptr, nullptr );
    if ( wnd == nullptr )
    {
        glfwTerminate();
        throw std::runtime_error( "error initializing window" ); 
    }

    glfwMakeContextCurrent(wnd);

    if ( glewInit() != GLEW_OK )
        throw std::runtime_error( "error initializing glew" );

    OpenGL::CContext::TDebugLevel debug_level = OpenGL::CContext::TDebugLevel::all;
    OpenGL::CContext context;
    context.Init( debug_level );

    std::string path = "C:/source/rabbid76workbench/test/texture/worldmap1.bmp";
    int cx, cy, ch;
    stbi_uc *img = stbi_load( path.c_str(), &cx, &cy, &ch, 4 );
    if ( img == nullptr )
        std::cout << "error loading texture" << std::endl;

    GLuint texture;
    glGenTextures(1, &texture);
    glActiveTexture( GL_TEXTURE0 );
    glBindTexture(GL_TEXTURE_2D, texture);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);  //for BMP
    
    //glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 2, 2, 0, GL_RED, GL_FLOAT, values);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, cx, cy, 0, GL_RGBA, GL_UNSIGNED_BYTE, img);
    stbi_image_free( img );

    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );


    std::vector<float> va; // vertex attribute array
    std::vector<int>   ia; // primitve index array
    CreateSphereMesh( 16, 32, va, ia );
    
    GLuint vao;
    glGenVertexArrays( 1, &vao );
    glBindVertexArray( vao );

    GLuint vbo;
    glGenBuffers( 1, &vbo );
    glBindBuffer( GL_ARRAY_BUFFER, vbo );
    glBufferData( GL_ARRAY_BUFFER, va.size()*sizeof(*va.data()), va.data(), GL_STATIC_DRAW );

    GLuint ibo;
    glGenBuffers( 1, &ibo );
    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, ibo );
    glBufferData( GL_ELEMENT_ARRAY_BUFFER, ia.size()*sizeof(*ia.data()), ia.data(), GL_STATIC_DRAW );

    glVertexPointer( 3, GL_FLOAT, 5*sizeof(*va.data()), 0 );
    glEnableClientState( GL_VERTEX_ARRAY );
    glTexCoordPointer( 2, GL_FLOAT, 5*sizeof(*va.data()), (void*)(3*sizeof(*va.data())) );
    glEnableClientState( GL_TEXTURE_COORD_ARRAY );
    
    glBindVertexArray( 0 );
    glBindBuffer( GL_ARRAY_BUFFER, 0 );
    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );

    /*
    GLint avert_loc = ....;
    GLint atexc_loc = ....;
    glVertexAttribPointer( avert_loc, 3, GL_FLOAT, GL_FALSE, 5*sizeof(*va.data()), 0 );
    glEnableVertexAttribArray( avert_loc );
    glVertexAttribPointer( atexc_loc, 2, GL_FLOAT, GL_FALSE, 5*sizeof(*va.data()), (void*)(3*sizeof(*va.data()))  );
    glEnableVertexAttribArray( atexc_loc );
    */

    static bool drawSphere = false;
    static bool drawLayers = true;

    gluPerspective( 90.0f, (float)800/(float)600, 0.5f, 10.0f );

    glMatrixMode( GL_MODELVIEW );
    if ( drawSphere)
      gluLookAt( 0.0, -3.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0 );
    else 
      gluLookAt( 0.0, -3.0, 2.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0 );

    static float scale = 1.0f;
    glScalef( scale, scale, scale );

    float r_ang = 0.0f;
    float l_ang = 0.0f;
    while (!glfwWindowShouldClose(wnd))
    {
        glPushMatrix();

        int vpSize[2];
        glfwGetFramebufferSize( wnd, &vpSize[0], &vpSize[1] );
        glViewport( 0, 0, vpSize[0], vpSize[1] );
      
        glEnable( GL_DEPTH_TEST );

        if ( drawSphere )
        {
            glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
            glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

            glRotatef( 10.0, 0.0f, 1.0f, 0.0f );
            glRotatef( r_ang, 0.0f, 0.0f, 1.0f );
            r_ang -= 2.0f;

            glEnable( GL_TEXTURE_2D );
            glBindVertexArray( vao );
            glDrawElements( GL_TRIANGLES, (GLsizei)ia.size(), GL_UNSIGNED_INT, 0 );
            glBindVertexArray( 0 );
        }

        if ( drawLayers )
        {
            glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
            glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

            glRotatef( r_ang, 0.0f, 0.0f, 1.0f );
            r_ang += 0.1f;

            float layer_scale = sin( l_ang ) * 0.5f + 1.5f;
            std::vector<float> va_l; // vertex attribute array
            std::vector<int>   ia_l; // primitve index array
            CreateSphereMeshLayer( 8, 12, layer_scale, 1.0f, va_l, ia_l );
            l_ang += 0.01f;

            glVertexPointer( 3, GL_FLOAT, 5*sizeof(*va_l.data()), va_l.data() );
            glEnableClientState( GL_VERTEX_ARRAY );

            glDepthFunc( GL_LESS );
            glColor4f( 1.0f, 1.0f, 1.0f, 1.0f );
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            glLineWidth( 3.0f );
            glDrawElements( GL_TRIANGLES, (GLsizei)ia_l.size(), GL_UNSIGNED_INT, ia_l.data() );

            glDepthFunc( GL_LEQUAL );
            glColor4f( 0.0f, 0.0f, 0.0f, 1.0f );
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            glDrawElements( GL_TRIANGLES, (GLsizei)ia_l.size(), GL_UNSIGNED_INT, ia_l.data() );
        }

        glfwSwapBuffers(wnd);
        glfwPollEvents();

        glPopMatrix();
    }

    glfwDestroyWindow( wnd );
    wnd = nullptr;
    glfwTerminate();

    return 0;
}