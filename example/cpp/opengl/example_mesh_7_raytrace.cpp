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


std::string draw_vert = R"(
#version 450

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

std::string draw_frag = R"(
#version 450

in vec3 vertPos;
in vec3 vertNV;
in vec2 vertUV;

out vec4 fragColor;

void main()
{
    fragColor = vec4(abs(vertNV), 1.0);
}
)";

std::string filter_vert = R"(
#version 460

layout (location = 0) in vec2 inPos;

out vec2 vertPos;

void main()
{
    vertPos     = inPos;
    gl_Position = vec4(inPos.xy, 0.0, 1.0);
}
)";

std::string filter_frag = R"(
#version 460

in vec2 vertPos;

out vec4 fragColor;

layout (binding = 0) uniform sampler2D u_ambient;
layout (binding = 1) uniform sampler2D u_depth;
layout (binding = 2) uniform sampler2D u_vertex;

uniform mat4 project;
uniform mat4 view;
uniform mat4 model;


bool isectTriangle(vec3 R0, vec3 D, vec3 A, vec3 B, vec3 C, out vec3 N, out vec3 X, out float d)
{   
    const float eps = 1e-8;

    vec3 AB = B - A;
    vec3 AC = C - A;

    N = normalize(cross(AB, AC));

    d = dot(A - R0, N) / dot(D, N);
    X = R0 + D * d;

    vec3 AX = X - A;

    float d00 = dot(AB, AB);
    float d01 = dot(AB, AC);
    float d11 = dot(AC, AC);
    float d20 = dot(AX, AB);
    float d21 = dot(AX, AC);
    
    float denom = d00 * d11 - d01 * d01; // determinant

    // if the determinant is negative the triangle is backfacing
    // if the determinant is close to 0, the ray misses the triangl
    //if ( denom <= eps )
    //    return false;
    
    vec3 b_coord; // barycentric coordinate
    b_coord.y = (d11 * d20 - d01 * d21) / denom;
    if ( b_coord.y < 0.0 || b_coord.y > 1.0 )
        return false;
    b_coord.z = (d00 * d21 - d01 * d20) / denom;
    if ( b_coord.z < 0.0 || b_coord.z > 1.0 )
        return false;
    b_coord.x = 1.0 - b_coord.y - b_coord.z;
    if ( b_coord.x < 0.0 || b_coord.x > 1.0 )
        return false;

    return true;
}


bool hitScene(vec3 R0, vec3 D, out vec3 X, out vec3 N)
{     
    bool hit = false;
    float min_dist = 1000.0; 
    X = vec3(0.0);
    N = vec3(0.0);
    
    ivec2 vSize = textureSize(u_vertex, 0);
    for( int i=0; i < vSize.y; ++i )
    {
        vec3 A = texelFetch(u_vertex, ivec2(0, i), 0).xyz;
        vec3 B = texelFetch(u_vertex, ivec2(1, i), 0).xyz;
        vec3 C = texelFetch(u_vertex, ivec2(2, i), 0).xyz;

        vec3 tri_N, tri_X;
        float tri_d;  
        bool tri_hit = isectTriangle(R0, D, A, B, C, tri_N, tri_X, tri_d);
        if (tri_hit && tri_d >= 0.0 && tri_d < min_dist) 
        {
            min_dist = abs(tri_d);
            X = tri_X;
            N = tri_N;
            hit = true;
        }
    }
    return hit;
}


void main()
{
    vec2 uv = vertPos * 0.5 + 0.5;

    mat4 invProject = inverse(project);
    mat4 invView    = inverse(view);
    mat4 invModel   = inverse(model);

    vec4 far_h = invModel * invView * invProject * vec4(vertPos.xy, 1.0, 1.0);
    vec3 R0    = (invModel * vec4(invView[3].xyz, 1.0)).xyz;
    vec3 D     = normalize(far_h.xyz/far_h.w - R0);

    float diff = 0.0;
    vec3 X = vec3(0.0);
    vec3 N = vec3(0.0);
    if (hitScene(R0, D, X, N)) 
    {
        diff = max(0.0, abs(dot(N, -D)));
    }
    vec3 col_diff = vec3(diff);
    
    vec3 color = texture(u_ambient, uv).rgb;
    float depth = texture(u_depth, uv).x;

    float depthMix = 1.0 - pow(depth, 10.0);
    vec3 colorMix = mix(color, vec3(depthMix), 0.95);

    //fragColor = vec4(colorMix + col_diff, 1.0);
    fragColor = vec4(col_diff, 1.0);
}
)";


void CreateFramebuffer( GLuint &fbo, GLuint &cto, GLuint &dto, GLsizei w, GLsizei h )
{
    if (fbo == 0)
        glGenFramebuffers(1, &fbo);
 
    if (cto == 0)
        glGenTextures(1, &cto);
    if (dto == 0)
        glGenTextures(1, &dto);

    glBindTexture(GL_TEXTURE_2D, cto);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    glBindTexture(GL_TEXTURE_2D, dto);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, w, h, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);

    glBindFramebuffer(GL_FRAMEBUFFER, fbo);   
    glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, cto, 0 );
    glFramebufferTexture2D( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, dto, 0 );

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        throw std::runtime_error( "error frambuffer inclomplete" ); 

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
}


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
   
    glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 8*sizeof(*va.data()), nullptr );
    glVertexAttribPointer( 1, 3, GL_FLOAT, GL_FALSE, 8*sizeof(*va.data()), (void*)(3*sizeof(*va.data())) );
    glVertexAttribPointer( 2, 2, GL_FLOAT, GL_FALSE, 8*sizeof(*va.data()), (void*)(6*sizeof(*va.data())) );
    
    glEnableVertexAttribArray( 0 );
    glEnableVertexAttribArray( 1 );
    glEnableVertexAttribArray( 2 );

    glBindBuffer( GL_ARRAY_BUFFER, 0 );
    glBindVertexArray( 0 );

    return std::make_pair(vao, (GLsizei)ia.size());
}


void AddToData( const TMesh& mesh, const glm::mat4 &model, std::vector<glm::vec4> &vertData )
{
    const auto &va = mesh.AttributeArray();
    const auto &ia = mesh.ElementArray();

    for ( auto i : ia )
    {
        glm::vec4 vert( va[i*8+0], va[i*8+1], va[i*8+2], 1.0f );
        vertData.emplace_back( model * vert );
    }
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
    TMesh icosahedron;
    TMesh tetrahedron;
    //TMesh trefoil;

    //MeshDef::TTest1Def<float, unsigned int>( 3, 1.0f, true ).CreateMesh( mesh_def );
    //MeshDef::TPlaneDef<float, unsigned int>( 1.0f, 1.0f, 10, 10 ).CreateMesh( mesh_def );
    MeshDef::TTetrahedronDef<float, unsigned int>( 1.0f ).CreateMesh( tetrahedron );
    //MeshDef::TCubeDef<float, unsigned int>( 1.0f, 1.0f, 1.0f, MeshDef::TCubeDef<float, unsigned int>::eCT_UV_per_side ).CreateMesh( mesh_def );
    MeshDef::TIcosahedronDef<float, unsigned int>( 1.0f ).CreateMesh( icosahedron );
    //MeshDef::TTriangleSphereDef<float, unsigned int>( 514 /* 4, 10, 34, 130, 514 */, 1.0f, true ).CreateMesh( mesh_def );
    MeshDef::TSphereDef<float, unsigned int>( 1.0f, 18, 9 ).CreateMesh( sphere );
    MeshDef::TCylinderDef<float, unsigned int>( 4.0f, 1.0f, 18, 2 ).CreateMesh( cylinder );
    //MeshDef::TTorusDef<float, unsigned int>( 0.7f, 0.3f, 30, 30 ).CreateMesh( mesh_def );
    //MeshDef::TTrefoilKnotDef<float, unsigned int>( 1.3f, 256, 32 ).CreateMesh( trefoil );
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
      { draw_vert, GL_VERTEX_SHADER },
      { draw_frag, GL_FRAGMENT_SHADER }
    } );

    OpenGL::ShaderProgramSimple filter_prog(
    {
      { filter_vert, GL_VERTEX_SHADER },
      { filter_frag, GL_FRAGMENT_SHADER }
    } );

    filter_prog.Use();

    std::vector<float> scrren_bo{ -1, -1, 1, -1, 1, 1, -1, 1 };

    GLuint scree_vao;
    glGenVertexArrays( 1, &scree_vao );
    GLuint rect_bao;
    glGenBuffers( 1, &rect_bao );
    glBindVertexArray( scree_vao );
    glBindBuffer( GL_ARRAY_BUFFER, rect_bao );
    glBufferData( GL_ARRAY_BUFFER, scrren_bo.size()*sizeof(*scrren_bo.data()), scrren_bo.data(), GL_STATIC_DRAW );
    glVertexAttribPointer( 0, 2, GL_FLOAT, GL_FALSE,0, nullptr );
    glEnableVertexAttribArray( 0 );
    glBindBuffer( GL_ARRAY_BUFFER, 0 );
    glBindVertexArray( 0 );
   
    draw_prog.Use();  

    std::vector<glm::vec4> vertData;

    shapes.emplace_back( CreateVAO( cylinder ) );
    shapes.back()._model = glm::translate( shapes.back()._model, glm::vec3( 1.2f, 0.0f, 0.0f ) );
    AddToData( cylinder, shapes.back()._model, vertData );

    shapes.emplace_back( CreateVAO( sphere ) );
    shapes.back()._model = glm::translate( shapes.back()._model, glm::vec3( -1.2f, 0.0f, -1.0f ) );
    AddToData( sphere, shapes.back()._model, vertData );

    //shapes.emplace_back( CreateVAO( tetrahedron ) );
    //shapes.back()._model = glm::translate( shapes.back()._model, glm::vec3( -1.2f, 0.0f, 1.0f ) );
    //AddToData( tetrahedron, shapes.back()._model, vertData );

    shapes.emplace_back( CreateVAO( icosahedron ) );
    shapes.back()._model = glm::translate( shapes.back()._model, glm::vec3( -1.2f, 0.0f, 1.0f ) );
    AddToData( icosahedron, shapes.back()._model, vertData );

    //shapes.emplace_back( CreateVAO( trefoil ) );
    //shapes.back()._model = glm::translate( shapes.back()._model, glm::vec3( 0.0f, 0.0f, 2.0f ) );
    //AddToData( trefoil, shapes.back()._model, vertData );
    
    GLuint vert_to;
    glGenTextures(1, &vert_to);
    glBindTexture(GL_TEXTURE_2D, vert_to);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, 3, (GLsizei)vertData.size() / 3, 0, GL_RGBA, GL_FLOAT, vertData.data());
    glBindTexture(GL_TEXTURE_2D, 0);

    GLuint fbo = 0;
    GLuint cto = 0;
    GLuint dto = 0;

    glm::mat4 project( 1.0f );
    glm::mat4 view = glm::lookAt(glm::vec3(-3.5f, -5.0f, 3.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));

    glEnable( GL_DEPTH_TEST );

    int vpSize[2]{ 0, 0 };
    while (!glfwWindowShouldClose(wnd))
    {
        int w, h;
        glfwGetFramebufferSize( wnd, &w, &h );
        if ( w != vpSize[0] || h != vpSize[1] )
        {
            vpSize[0] = (int)w;
            vpSize[1] = (int)h;

            CreateFramebuffer( fbo, cto, dto, (GLsizei)vpSize[0], (GLsizei)vpSize[1] );

            glViewport( 0, 0, vpSize[0], vpSize[1] );
            float ascpect = (float)vpSize[0] / (float)vpSize[1];
            float orthoX = ascpect > 1.0f ? ascpect : 1.0f;
            float orthoY = ascpect > 1.0f ? 1.0f : ascpect;
            //project = glm::ortho( -orthoX, orthoX, -orthoY, orthoY, -1.0f, 1.0f );
            project = glm::perspective( glm::radians(60.0f), ascpect, 0.1f, 100.0f  ); 

            draw_prog.Use();  
            draw_prog.SetUniformM44( "project", glm::value_ptr(project) );
            draw_prog.SetUniformM44( "view", glm::value_ptr(view) );

            filter_prog.Use(); 
            filter_prog.SetUniformM44( "project", glm::value_ptr(project) );
            filter_prog.SetUniformM44( "view", glm::value_ptr(view) );
        }

        static float angle = 0.0f;
        glm::mat4 rotate = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(1.0f,0.0f, 0.0f));
        angle += 1.0f;
        
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        
        glBindFramebuffer(GL_FRAMEBUFFER, fbo); 
        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

        draw_prog.Use();  
        for ( const auto & shape : shapes )
        {
            glm::mat4 model = rotate * shape._model;
            draw_prog.SetUniformM44( "model", glm::value_ptr(model) );
            glBindVertexArray( shape._vao );
            glDrawElements( GL_TRIANGLES, shape._size, GL_UNSIGNED_INT, nullptr );
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

        glActiveTexture( GL_TEXTURE2 );
        glBindTexture(GL_TEXTURE_2D, vert_to);
        glActiveTexture( GL_TEXTURE1 );
        glBindTexture(GL_TEXTURE_2D, dto);
        glActiveTexture( GL_TEXTURE0 );
        glBindTexture(GL_TEXTURE_2D, cto);

        filter_prog.Use();
        draw_prog.SetUniformM44( "model", glm::value_ptr(rotate) );

        glBindVertexArray( scree_vao );
        glDrawArrays( GL_TRIANGLE_FAN, 0, 4 );

        glfwSwapBuffers(wnd);
        glfwPollEvents();
    }

    glfwDestroyWindow( wnd );
    wnd = nullptr;
    glfwTerminate();

    return 0;
}