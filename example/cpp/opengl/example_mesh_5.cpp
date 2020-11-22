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
layout (location = 1) in vec3 inNV;
layout (location = 2) in vec2 inUV;

out vec3 vertPos;
out vec3 vertNV;
out vec2 vertUV;

uniform mat4 project;
uniform mat4 view;
uniform mat4 model;

void main()
{
    mat4 mv_mat     = view * model;
    mat3 normal_mat = transpose(inverse(mat3(mv_mat))); 

    vec4 view_pos = mv_mat * vec4(inPos, 1.0);

	vertPos     = view_pos.xyz; 
    vertNV      = normalize(normal_mat * inNV);
    vertUV      = inUV;

	gl_Position = project * vec4(view_pos.xyz, 1.0);
}
)";

std::string sh_frag = R"(
#version 400

in vec3 vertPos;
in vec3 vertNV;
in vec2 vertUV;

out vec4 fragColor;

void main()
{
    fragColor = vec4(abs(vertNV), 1.0);
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
   
    glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 8*sizeof(*va.data()), 0 );
    glVertexAttribPointer( 1, 3, GL_FLOAT, GL_FALSE, 8*sizeof(*va.data()), (void*)(3*sizeof(*va.data())) );
    glVertexAttribPointer( 2, 2, GL_FLOAT, GL_FALSE, 8*sizeof(*va.data()), (void*)(6*sizeof(*va.data())) );
    
    glEnableVertexAttribArray( 0 );
    glEnableVertexAttribArray( 1 );
    glEnableVertexAttribArray( 2 );

    glBindBuffer( GL_ARRAY_BUFFER, 0 );
    glBindVertexArray( 0 );

    return std::make_pair(vao, (GLsizei)ia.size());
}


std::pair<GLuint, GLsizei> CreateVAO( const TMesh &mesh )
{
    const auto &va = mesh.AttributeArray();
    const auto &ia = mesh.ElementArray();

    return CreateVAO(va, ia);
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
    TMesh cylinder;
    TMesh sphere;
    TMesh trefoil;

    //MeshDef::TTest1Def<float, unsigned int>( 3, 1.0f, true ).CreateMesh( mesh_def );
    //MeshDef::TPlaneDef<float, unsigned int>( 1.0f, 1.0f, 10, 10 ).CreateMesh( mesh_def );
    //MeshDef::TTetrahedronDef<float, unsigned int>( 1.0f ).CreateMesh( mesh_def );
    //MeshDef::TCubeDef<float, unsigned int>( 1.0f, 1.0f, 1.0f, MeshDef::TCubeDef<float, unsigned int>::eCT_UV_per_side ).CreateMesh( mesh_def );
    //MeshDef::TIcosahedronDef<float, unsigned int>( 1.0f ).CreateMesh( mesh_def );
    //MeshDef::TTriangleSphereDef<float, unsigned int>( 514 /* 4, 10, 34, 130, 514 */, 1.0f, true ).CreateMesh( mesh_def );
    MeshDef::TSphereDef<float, unsigned int>( 1.0f, 36, 18 ).CreateMesh( sphere );
    MeshDef::TCylinderDef<float, unsigned int>( 2.0f, 1.0f, 36, 2 ).CreateMesh( cylinder );
    //MeshDef::TTorusDef<float, unsigned int>( 0.7f, 0.3f, 30, 30 ).CreateMesh( mesh_def );
    MeshDef::TTrefoilKnotDef<float, unsigned int>( 1.3f, 256, 32 ).CreateMesh( trefoil );
    //MeshDef::TTorusKnotDef<float, unsigned int>(  3, 7, 0.33f, 0.075f, 512, 32  ).CreateMesh( mesh_def );

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

    shapes.emplace_back( CreateVAO( cylinder ) );
    shapes.back()._model = glm::translate( shapes.back()._model, glm::vec3( 1.2f, 0.0f, 0.0f ) );
    shapes.back()._model = glm::scale( shapes.back()._model, glm::vec3( 1.0f, 1.0f, 2.0f ) );

    shapes.emplace_back( CreateVAO( sphere ) );
    shapes.back()._model = glm::translate( shapes.back()._model, glm::vec3( -1.2f, 0.0f, -1.0f ) );

    shapes.emplace_back( CreateVAO( trefoil ) );
    shapes.back()._model = glm::translate( shapes.back()._model, glm::vec3( -1.2f, 0.0f, 1.0f ) );
    

    glEnable( GL_DEPTH_TEST );

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
            glm::vec3(-3.5f, -5.0f, 3.0f),
            glm::vec3(0.0f, 0.0f, 0.0f),
            glm::vec3(0.0f, 0.0f, 1.0f));

        glViewport( 0, 0, vpSize[0], vpSize[1] );
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
      
        draw_prog.Use();  
        draw_prog.SetUniformM44( "project", glm::value_ptr(project) );
        draw_prog.SetUniformM44( "view", glm::value_ptr(view) );

        for ( const auto & shape : shapes )
        {
            draw_prog.SetUniformM44( "model", glm::value_ptr(shape._model) );
            glBindVertexArray( shape._vao );
            glDrawElements( GL_TRIANGLES, shape._size, GL_UNSIGNED_INT, nullptr );
        }

        glfwSwapBuffers(wnd);
        glfwPollEvents();
    }

    glfwDestroyWindow( wnd );
    wnd = nullptr;
    glfwTerminate();

    return 0;
}