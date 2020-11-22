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
#include <stdexcept>
#include <array>
#include <map>
#include <tuple>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <chrono>

#define _USE_MATH_DEFINES
#include <cmath>
#include <math.h>

// utility

#include <mesh/meshdef_template.h>

// project includes
#include <gl/gl_debug.h>
#include <gl/gl_shader.h>

// preprocessor definitions

#ifndef M_PI
#define M_PI 3.14159265358979323846 
#endif

using TShaderInfo = std::tuple< std::string, int >;
using TVec3       = std::array< float, 3 >;
using TVec4       = std::array< float, 4 >;
using TMat44      = std::array< TVec4, 4 >;
using TSize       = std::array< int, 2 >;


class TMesh
    : public MeshDef::TMeshData<float, unsigned int>
{
public:

    using TBase = MeshDef::TMeshData<float, unsigned int>;

    using TArrayBuffer        = std::vector<float>;
    using TElementArrayBuffer = std::vector<unsigned int>;

    TMesh( void ) {}

    virtual Render::TMeshFaceType       FaceType( void )            const override { return Render::TMeshFaceType::triangles; }
    virtual Render::TMeshFaceSizeKind   FaceSizeKind( void )        const override { return Render::TMeshFaceSizeKind::constant; } 
    virtual Render::TMeshIndexKind      IndexKind( void )           const override { return Render::TMeshIndexKind::common; }
    virtual Render::TMeshAttributePack  Pack( void )                const override { return Render::TMeshAttributePack::separated_tightly; }
    virtual const TAttributeContainer & Vertices( void )            const override { return _dummy; } 
    virtual const TIndexContainer     * Indices( void )             const override { return nullptr; }
    virtual unsigned int                FaceSize( void )            const override { return 3; }
    virtual const TIndexContainer     * FaceSizes( void )           const override { return nullptr; }
    virtual unsigned int                FaceRestart( void )         const override { return 0; }
    virtual Render::TMeshNormalKind     NormalKind( void )          const override { return Render::TMeshNormalKind::vertex; }
    virtual const TAttributeContainer * Normals( void )             const override { return nullptr;}
    virtual const TIndexContainer     * NormalIndices( void )       const override { return nullptr; }
    virtual const TAttributeContainer * FaceNormals( void )         const override { return nullptr; }
    virtual const TIndexContainer     * FaceNormalIndices( void )   const override { return nullptr; }
    virtual const TAttributeContainer * TextureCoordinates( void )  const override { return nullptr; }
    virtual const TIndexContainer     * TextureCoordIndices( void ) const override { return nullptr; }
    virtual const TAttributeContainer * Colors( void )              const override { return nullptr; }
    virtual const TIndexContainer     * ColorIndices( void )        const override { return nullptr; }  

    virtual size_t NoOfVertices( void ) const override { return _attributes.size() / 8; }

    const TArrayBuffer        & AttributeArray( void ) const { return _attributes; }
    const TElementArrayBuffer & ElementArray( void )   const { return _indices; }

    
    virtual void Reserve( unsigned int count ) override
    {
        _attributes.reserve( count * 8 );
    }
    
    virtual void ReserveFaces( unsigned int count ) override
    {
        _indices.reserve( count * 3 );
    }

    virtual void Add( float ptX, float ptY, float ptZ, float nvX, float nvY, float nvZ, bool normalized, float tU, float tV )
    { 
        glm::vec3 nv = normalized ? glm::vec3( nvX, nvY, nvZ ) : glm::normalize( glm::vec3( nvX, nvY, nvZ ) );
        _attributes.insert( _attributes.begin(), {  ptX, ptY, ptZ, nv.x, nv.y, nv.z, tU, tV } );
    }

    virtual void AddFace( unsigned int i0,  unsigned int i1,  unsigned int i2 )
    {
        _indices.insert( _indices.begin(), { i0, i1, i2 } );
    }

private:

    Render::TAttributeVector<float, 3, 8, 0> _dummy;

    TArrayBuffer        _attributes;
    TElementArrayBuffer _indices;

};


std::string sh_vert = R"(
#version 400

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec2 inUV;

out vec3 vertPos;
out vec3 vertNV;
out vec2 vertUV;

uniform mat4 project;
uniform mat4 view;
uniform mat4 model;

void main()
{
    mat4 mv_mat     = view * model;
    
    vec4 view_pos = mv_mat * vec4(inPos, 1.0);

	vertPos     = view_pos.xyz; 
    vertUV      = inUV;

	gl_Position = project * vec4(view_pos.xyz, 1.0);
}
)";

std::string sh_frag = R"(
#version 400

in vec3 vertPos;
in vec2 vertUV;

out vec4 fragColor;

void main()
{
    vec3 color = vec3(vertUV.x, vertUV.y, (1.0-vertUV.x)*(1.0-vertUV.y)); 
    fragColor = vec4(color, 1.0);
}
)";


std::pair<GLuint, GLsizei> CreateVAO( const TMesh::TArrayBuffer &va, const TMesh::TElementArrayBuffer &ia )
{
    GLuint vao;
    glGenVertexArrays( 1, &vao );

    GLuint bo[2];
    glGenBuffers( 2, bo );
   
    glBindVertexArray( vao );

    glBindBuffer( GL_ARRAY_BUFFER, bo[0] );
    glBufferData( GL_ARRAY_BUFFER, va.size()*sizeof(*va.data()), va.data(), GL_STATIC_DRAW );

    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, bo[1] );
    glBufferData( GL_ELEMENT_ARRAY_BUFFER, ia.size()*sizeof(*ia.data()), ia.data(), GL_STATIC_DRAW );
   
    glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 5*sizeof(*va.data()), 0 );
    glVertexAttribPointer( 1, 2, GL_FLOAT, GL_FALSE, 5*sizeof(*va.data()), (void*)(3*sizeof(*va.data())) );
    
    glEnableVertexAttribArray( 0 );
    glEnableVertexAttribArray( 1 );

    glBindBuffer( GL_ARRAY_BUFFER, 0 );
    glBindVertexArray( 0 );

    return std::make_pair(vao, (GLsizei)ia.size());
}


class TShape
{
public:

    TShape( void ) = default;
    TShape( const TShape & ) = default;
    TShape( TShape && ) = default;

    TShape & operator = ( const TShape & ) = default;
    TShape & operator = ( TShape && ) = default;

    TShape( const std::pair<GLuint, GLsizei> &obj )
        : _vao( obj.first )
        , _size( obj.second )
        , _model(1.0f)
    {}

    GLuint    _vao;
    GLsizei   _size;
    glm::mat4 _model;
};

std::vector<TShape> shapes;

int main(void)
{
    if ( glfwInit() == 0 )
        throw std::runtime_error( "error initializing glfw" );

    //glfwWindowHint(GLFW_REFRESH_RATE, 10);

    GLFWwindow *wnd = glfwCreateWindow( 800, 600, "OGL window", nullptr, nullptr );
    if ( wnd == nullptr )
    {
        glfwTerminate();
        throw std::runtime_error( "error initializing window" ); 
    }

    glfwMakeContextCurrent(wnd);

    //glfwSwapInterval( 2 );

    if ( glewInit() != GLEW_OK )
        throw std::runtime_error( "error initializing glew" );

    OpenGL::CContext::TDebugLevel debug_level = OpenGL::CContext::TDebugLevel::all;
    OpenGL::CContext context;
    context.Init( debug_level );

    
    OpenGL::ShaderProgramSimple draw_prog(
    {
      { sh_vert, GL_VERTEX_SHADER },
      { sh_frag, GL_FRAGMENT_SHADER }
    } );
   
    draw_prog.Use();  

    TMesh::TArrayBuffer vertices{ 
        -0.5f,  0.5f, -0.5f, 0, 0, 
        -0.5f, -0.5f, -0.5f, 0, 1,
         0.5f, -0.5f, -0.5f, 1, 1,
         0.5f,  0.5f, -0.5f, 1, 0,

        -0.5f,  0.5f,  0.5f, 0, 0,
        -0.5f, -0.5f,  0.5f, 0, 1,
         0.5f, -0.5f,  0.5f, 1, 1,
         0.5f,  0.5f,  0.5f, 1, 0,
         
         0.5f,  0.5f, -0.5f, 0, 0,
         0.5f, -0.5f, -0.5f, 0, 1,
         0.5f, -0.5f,  0.5f, 1, 1,
         0.5f,  0.5f,  0.5f, 1, 0,
        
        -0.5f,  0.5f, -0.5f, 0, 0,
        -0.5f, -0.5f, -0.5f, 0, 1,
        -0.5f, -0.5f,  0.5f, 1, 1,
        -0.5f,  0.5f,  0.5f, 1, 0,
        
        -0.5f,  0.5f,  0.5f, 0, 0,
        -0.5f,  0.5f, -0.5f, 0, 1,
         0.5f,  0.5f, -0.5f, 1, 1,
         0.5f,  0.5f,  0.5f, 1, 0, 
        
        -0.5f, -0.5f,  0.5f, 0, 0,
        -0.5f, -0.5f, -0.5f, 0, 1,
         0.5f, -0.5f, -0.5f, 1, 1,
         0.5f, -0.5f,  0.5f, 1, 0
    };

    /*
    TMesh::TElementArrayBuffer indices{
         0,  1,  3,  3,  1,  2,
         4,  5,  7,  7,  5,  6,
         8,  9, 11, 11,  9, 10,
        12, 13, 15, 15, 13, 14,
        16, 17, 19, 19, 17, 18,
        20, 21, 23, 23, 21, 22
    };
    */

    TMesh::TElementArrayBuffer indices{
         0,  3,  1,  3,  2,  1,
         4,  5,  7,  7,  5,  6,
         8, 11,  9, 11,  10, 9,
        12, 13, 15, 15, 13, 14,
        16, 19, 17, 19, 18, 17,
        20, 21, 23, 23, 21, 22
    };

    shapes.emplace_back( CreateVAO( vertices, indices ) ); 

    glEnable( GL_DEPTH_TEST );
    glEnable( GL_CULL_FACE );
    glCullFace( GL_BACK );
    glFrontFace( GL_CCW );

    while (!glfwWindowShouldClose(wnd))
    {
        int vpSize[2];
        glfwGetFramebufferSize( wnd, &vpSize[0], &vpSize[1] );
        
        float ascpect = (float)vpSize[0] / (float)vpSize[1];
        float orthoX = ascpect > 1.0f ? ascpect : 1.0f;
        float orthoY = ascpect > 1.0f ? 1.0f : ascpect;
        //glm::mat4 project = glm::ortho( -orthoX, orthoX, -orthoY, orthoY, -1.0f, 1.0f );
        glm::mat4 project = glm::perspective( glm::radians(60.0f), ascpect, 0.1f, 100.0f  );
        
        glm::mat4 view = glm::lookAt(
            glm::vec3(0.0f, -2.0f, 0.0f),
            glm::vec3(0.0f, 0.0f, 0.0f),
            glm::vec3(0.0f, 0.0f, 1.0f));

        glViewport( 0, 0, vpSize[0], vpSize[1] );
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
      
        draw_prog.Use();  
        draw_prog.SetUniformM44( "project", glm::value_ptr(project) );
        draw_prog.SetUniformM44( "view", glm::value_ptr(view) );

        static float angle = 0.0f;
        for ( const auto & shape : shapes )
        {
            glm::mat4 model = glm::rotate( glm::mat4(1.0f), glm::radians(angle), glm::vec3( 1.0f, 0.0f, 0.0f ) );
            model = glm::rotate( model, glm::radians(angle * 0.25f), glm::vec3( 0.0f, 1.0f, 0.0f ) );

            draw_prog.SetUniformM44( "model", glm::value_ptr(model) );
            glBindVertexArray( shape._vao );
            glDrawElements( GL_TRIANGLES, shape._size, GL_UNSIGNED_INT, nullptr );
        }
        angle += 1.0f;

        glfwSwapBuffers(wnd);
        glfwPollEvents();
    }

    glfwDestroyWindow( wnd );
    wnd = nullptr;
    glfwTerminate();

    return 0;
}