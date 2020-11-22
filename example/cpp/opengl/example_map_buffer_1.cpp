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

// preprocessor definitions

#ifndef M_PI
#define M_PI 3.14159265358979323846 
#endif

// forward declarations

void display( void );


// shader

std::string sh_vert = R"(
#version 420

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec4 inColor;

out vec3 vertPos;
out vec4 vertCol;

layout(binding = 1, std140) uniform Matrices
{                
     mat4 project;
     mat4 view;
     mat4 model;
};

void main()
{
    vec4 view_pos =  view * model * vec4(inPos, 1.0);
 
    vertCol     = inColor;
		vertPos     = view_pos.xyz;
		gl_Position = project * vec4(view_pos.xyz, 1.0);
}
)";

std::string sh_frag = R"(
#version 400

in vec3 vertPos;
in vec4 vertCol;

out vec4 fragColor;

void main()
{
    fragColor = vertCol;
}
)";


// globale variables

std::chrono::high_resolution_clock::time_point start_time;

OpenGL::ShaderProgramSimple *g_prog;
GLuint uniformIndices[3];

template<typename T>
size_t vector_size( const T &v ) { return v.size() * sizeof(*v.data()); }

bool map_uniform_buffer = true;

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

    g_prog = new OpenGL::ShaderProgramSimple(
    {
      { sh_vert, GL_VERTEX_SHADER },
      { sh_frag, GL_FRAGMENT_SHADER }
    } );                                            

    const float sin120 = 0.8660254f;
    static const std::vector<float> varray
    { 
       0.0f,             0.0f,           1.0f,    1.0f, 0.0f, 0.0f, 1.0f, 
       0.0f,            -sin120,        -0.5f,    1.0f, 1.0f, 0.0f, 1.0f,
       sin120 * sin120,  0.5f * sin120, -0.5f,    0.0f, 0.0f, 1.0f, 1.0f,
      -sin120 * sin120,  0.5f * sin120, -0.5f,    0.0f, 1.0f, 0.0f, 1.0f 
    };
    const std::vector<unsigned int> iarray
    {
      0, 1, 2, 0, 2, 3, 0, 3, 1, 1, 3, 2 
    };

    GLuint vertex_binding_index = 0;
    GLuint color_binding_index = 1;

    GLuint vbo;
    glGenBuffers( 1, &vbo );
    glBindBuffer( GL_ARRAY_BUFFER, vbo );

    // TODO documentation
      
//! [`glBindBuffer`](https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glBindBuffer.xhtml)

//! [`glBindBufferBase`](https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glBindBufferBase.xhtml)
//! [`glBindBufferRange`](https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glBindBufferRange.xhtml)

//! [`glBufferData`](https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glBufferData.xhtml)
//! [`glBufferSubData`](https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glBufferSubData.xhtml)

//! [`glBufferStorage`](https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glBufferStorage.xhtml)
//! [`glMapBuffer`](https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glMapBuffer.xhtml)
//! [`glUnmapBuffer`](https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glUnmapBuffer.xhtml)
//! [`glMapBufferRange`](https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glMapBufferRange.xhtml)
//! [`glFlushMappedBufferRange`](https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glFlushMappedBufferRange.xhtml)
//! [`glMemoryBarrier`](https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glMemoryBarrier.xhtml)

//! [`glClearBufferSubData`](https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glClearBufferSubData.xhtml)
//! [`glCopyBufferSubData`](https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glCopyBufferSubData.xhtml)
//! [`glGetBufferSubData`](https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glGetBufferSubData.xhtml)
//! [`glInvalidateBufferSubData`](https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glInvalidateBufferSubData.xhtml)


/*! [OpenGL 4.6 API Core Profile Specification - 6.2 Creating and Modifying Buffer Object Data Stores - page 67](https://www.khronos.org/registry/OpenGL/specs/gl/glspec46.core.pdf)<
    
> `BufferStorage` and `NamedBufferStorage` delete any existing data store, and set the values of the buffer object?s state variables as shown in table 6.3.  
If any portion of the buffer object is mapped in the current context or any context current to another thread, it is as though `UnmapBuffer` (see section 6.3.1) is executed in each such context prior to deleting the existing data store.

> Calling `*BufferData` is equivalent to calling `BufferStorage` with *target*, *size* and *data* as specified, and *flags* set to the logical `OR of DYNAMIC_STORAGE_BIT,
`MAP_READ_BIT` and `MAP_WRITE_BIT`. The GL will use the value of the *usage* parameter to `*BufferData` as a hint to further determine the intended use of the buffer.
However, `BufferStorage` allocates immutable storage whereas `*BufferData` allocates mutable storage. Thus, when a buffer?s data store is allocated through a call to `*BufferData`, the buffer?s `BUFFER_IMMUTABLE_STORAGE` flag is set to FALSE.

*/    


    enum class TBufferAssign
    {
      data, sub_data, data_map, storage_sub_data,
      map_unmap, map_persistent_flush, map_persistent_memory_barrier, map_persistent_coherent,
      test_1, test_2
    };

    static TBufferAssign assign_type = TBufferAssign::map_persistent_coherent;
    switch (assign_type)
    {
      default:
      case TBufferAssign::data:
        {
          // directly allocat GPU memory and initialize buffer
          glBufferData( GL_ARRAY_BUFFER, vector_size(varray), varray.data(), GL_STATIC_DRAW );
        }
        break;

      case TBufferAssign::sub_data:
        {
          // allocate buffer and assigne buffer data  
          glBufferData( GL_ARRAY_BUFFER, vector_size(varray), nullptr, GL_STATIC_DRAW );
          glBufferSubData( GL_ARRAY_BUFFER, 0, vector_size(varray), varray.data() );
        }
        break;

      case TBufferAssign::data_map:
        {
          // allocate buffer and assigne buffer data  
          glBufferData( GL_ARRAY_BUFFER, vector_size(varray), nullptr, GL_STATIC_DRAW );
          //void *buffer_ptr = glMapBuffer( GL_ARRAY_BUFFER, GL_READ_ONLY );
          //void *buffer_ptr = glMapBuffer( GL_ARRAY_BUFFER, GL_WRITE_ONLY );
          //void *buffer_ptr = glMapNamedBuffer(vbo, GL_READ_ONLY);
          //void *buffer_ptr = glMapNamedBuffer(vbo, GL_WRITE_ONLY);
          void *buffer_ptr = glMapBufferRange( GL_ARRAY_BUFFER, 0, vector_size(varray), GL_MAP_WRITE_BIT );
          std::memcpy( reinterpret_cast<float*>( buffer_ptr ), varray.data(), vector_size(varray) );
          glUnmapBuffer( GL_ARRAY_BUFFER );
        }
        break;

      case TBufferAssign::storage_sub_data:
        {
          // create buffer storage and assigne buffer data  
          glBufferStorage( GL_ARRAY_BUFFER, vector_size( varray ), nullptr, GL_DYNAMIC_STORAGE_BIT );
          glBufferSubData( GL_ARRAY_BUFFER, 0, vector_size(varray), varray.data() );
        }
        break;

      case TBufferAssign::map_unmap:
        {
          // map and unmap buffer
          glBufferStorage( GL_ARRAY_BUFFER, vector_size( varray ), nullptr, GL_MAP_WRITE_BIT );
          void *buffer_ptr = glMapBufferRange( GL_ARRAY_BUFFER, 0, vector_size(varray), GL_MAP_WRITE_BIT );
          std::memcpy( reinterpret_cast<float*>( buffer_ptr ), varray.data(), vector_size(varray) );
          glUnmapBuffer( GL_ARRAY_BUFFER );
        }
        break;

      case TBufferAssign::map_persistent_flush:
        {
          // map coherent buffer and flush the buffer
          glBufferStorage( GL_ARRAY_BUFFER, vector_size( varray ), nullptr, GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT );
          void *buffer_ptr = glMapBufferRange( GL_ARRAY_BUFFER, 0, vector_size(varray), GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_FLUSH_EXPLICIT_BIT );
          std::memcpy( reinterpret_cast<float*>( buffer_ptr ), varray.data(), vector_size(varray) );
          glFlushMappedBufferRange( GL_ARRAY_BUFFER, 0, vector_size( varray ) );
        }
        break;

      case TBufferAssign::map_persistent_memory_barrier:
        {
          // map coherent buffer and flush the buffer
          glBufferStorage( GL_ARRAY_BUFFER, vector_size( varray ), nullptr, GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT );
          void *buffer_ptr = glMapBufferRange( GL_ARRAY_BUFFER, 0, vector_size(varray), GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT );
          std::memcpy( reinterpret_cast<float*>( buffer_ptr ), varray.data(), vector_size(varray) );
          glMemoryBarrier( GL_CLIENT_MAPPED_BUFFER_BARRIER_BIT );
        }
        break;

      case TBufferAssign::map_persistent_coherent:
        {
          // map a persistent and coherent buffer 
          glBufferStorage( GL_ARRAY_BUFFER, vector_size( varray ), nullptr, GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT );
          void *buffer_ptr = glMapBufferRange( GL_ARRAY_BUFFER, 0, vector_size(varray), GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT );
          std::memcpy( reinterpret_cast<float*>( buffer_ptr ), varray.data(), vector_size(varray) );
        }
        break;

      case TBufferAssign::test_1:
        {  
          // does not work!
          static std::vector<float> buffer( varray.size() );
          glBufferStorage( GL_ARRAY_BUFFER, vector_size( varray ), buffer.data(), GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT );
          std::copy( varray.begin(), varray.end(), buffer.begin() );
          glMemoryBarrier( GL_CLIENT_MAPPED_BUFFER_BARRIER_BIT );
        }
        break;

      case TBufferAssign::test_2:
        {
          // does not work!
          glBufferStorage( GL_ARRAY_BUFFER, vector_size( varray ), nullptr, GL_MAP_WRITE_BIT );
          void *buffer_ptr = glMapBufferRange( GL_ARRAY_BUFFER, 0, vector_size(varray), GL_MAP_WRITE_BIT | GL_MAP_FLUSH_EXPLICIT_BIT );
          std::memcpy( reinterpret_cast<float*>( buffer_ptr ), varray.data(), vector_size(varray) );
          glFlushMappedBufferRange( GL_ARRAY_BUFFER, 0, vector_size( varray ) );
          glMemoryBarrier( GL_CLIENT_MAPPED_BUFFER_BARRIER_BIT );
        }
        break;
    }
    
    glBindBuffer( GL_ARRAY_BUFFER, 0 );


    GLuint vertex_attrib_inx = 0;
    GLuint color_attrib_inx  = 1;

    /*
    GLuint vao;
    glGenVertexArrays( 1, &vao );
    glBindVertexArray( vao );
    glBindBuffer( GL_ARRAY_BUFFER, vbo );
    glVertexAttribPointer( vertex_attrib_inx, 3, GL_FLOAT, GL_FALSE, 7*sizeof(*varray.data()), 0 );
    glEnableVertexAttribArray( vertex_attrib_inx );
    glVertexAttribPointer( color_attrib_inx, 4, GL_FLOAT, GL_FALSE, 7*sizeof(*varray.data()), (void*)(3*sizeof(*varray.data())) );
    glEnableVertexAttribArray( color_attrib_inx );
    glBindBuffer( GL_ARRAY_BUFFER, 0 );
    */

    GLuint vao;
    glGenVertexArrays( 1, &vao );
    glBindVertexArray( vao );
    
    glBindVertexBuffer( vertex_binding_index, vbo, 0, 7*sizeof(*varray.data()) );
    glVertexAttribFormat( vertex_attrib_inx, 3, GL_FLOAT, GL_FALSE, 0 );

    glBindVertexBuffer( color_binding_index, vbo, 3*sizeof(*varray.data()), 7*sizeof(*varray.data()) );
    glVertexAttribFormat( color_attrib_inx, 4, GL_FLOAT, GL_FALSE, 0 );
    //glBindVertexBuffer( color_binding_index, vbo, 0, 7*sizeof(*varray.data()) );
    //glVertexAttribFormat( color_attrib_inx, 4, GL_FLOAT, GL_FALSE, 3*sizeof(*varray.data()) );
    
    glVertexAttribBinding( vertex_attrib_inx, vertex_binding_index );
    glVertexAttribBinding( color_attrib_inx,  color_binding_index );

    glEnableVertexAttribArray( vertex_attrib_inx );
    glEnableVertexAttribArray( color_attrib_inx );
    
    GLuint ibo;
    glGenBuffers( 1, &ibo );
    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, ibo );
    //glBufferData( GL_ELEMENT_ARRAY_BUFFER, varray.size()*sizeof(*varray.data()), varray.data(), GL_STATIC_DRAW );
    glBufferData( GL_ELEMENT_ARRAY_BUFFER, iarray.size()*sizeof(*iarray.data()), nullptr, GL_STATIC_DRAW );
    glBufferSubData( GL_ELEMENT_ARRAY_BUFFER, GL_ZERO, iarray.size()*sizeof(*iarray.data()), iarray.data() );

    glBindVertexArray( 0 );

    // ...

    glBindVertexArray( vao );


    static const GLchar * uniformNames[3] =
    {
        "project",
        "view",
        "model",
    };
    glGetUniformIndices( g_prog->Prog(), 3, uniformNames, uniformIndices );

    GLuint binding_point = 1;

    //GLuint ub_index = glGetUniformBlockIndex( g_prog->Prog(), "Matrices" );
    //glUniformBlockBinding( g_prog->Prog(), ub_index, binding_point );

    GLuint ubo;
    glCreateBuffers( 1, &ubo );
    
    glBindBufferBase( GL_UNIFORM_BUFFER, binding_point, ubo );

    if ( map_uniform_buffer )
    {
      glNamedBufferStorage( ubo, 3*sizeof(glm::mat4), nullptr, GL_MAP_WRITE_BIT );
      glBindBuffer( GL_UNIFORM_BUFFER, ubo );
    }
    else
    {
      glBufferData( GL_UNIFORM_BUFFER, 3*sizeof(glm::mat4), nullptr, GL_STATIC_DRAW );
    }


    g_prog->Use();

    start_time = std::chrono::high_resolution_clock::now();
    
    std::cout << "main loop" << std::endl;
    glutMainLoop(); 

    std::cout << "terminate" << std::endl;
    glutDestroyWindow(wnd);
    return 0;    
} 

void display( void )
{
    static float ang_x = 0.0f;
    static float ang_y = 0.1f;

    int vpSize[2]
    {
        glutGet(GLUT_WINDOW_WIDTH),
        glutGet(GLUT_WINDOW_HEIGHT)
    };
    glViewport( 0, 0, vpSize[0], vpSize[1] );
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    float aspect = (float)vpSize[0]/(float)vpSize[1];
    glm::mat4 project = glm::perspective( (float)M_PI/3.0f, aspect, 0.01f, 1000.0f );
        
    static bool invert = false;
    glm::mat4 view = glm::lookAt(
      glm::vec3(-1.0f, -3.0f, 0.0f),
      glm::vec3(0.0f, 0.0f,  0.0f),
      glm::vec3(0.0f, 0.0f, 1.0f));

    glm::mat4 model( 1.0f );
    model = glm::rotate( model, ang_x, glm::vec3( 1.0f, 0.0f, 0.0f ) );
    model = glm::rotate( model, ang_y, glm::vec3( 0.0f, 0.1f, 0.0f ) );
    ang_x += 0.013f;
    ang_y += 0.017f;
        
    std::array<glm::mat4, 3> matrices;
    matrices[0] = project;
    matrices[1] = view;
    matrices[2] = model;
    
    if ( map_uniform_buffer )
    {
      unsigned char * ptr = (unsigned char *) glMapBuffer(GL_UNIFORM_BUFFER, GL_WRITE_ONLY);
      std::memcpy( ptr, matrices.data(), 3 * sizeof( glm::mat4 ) );
      glUnmapBuffer( GL_UNIFORM_BUFFER );
    }
    else
    {
      glBufferSubData( GL_UNIFORM_BUFFER, 0, 3*sizeof(glm::mat4), glm::value_ptr(matrices[0]) );
    }

    glEnable( GL_DEPTH_TEST );
      
    glDrawElements( GL_TRIANGLES, 12, GL_UNSIGNED_INT, nullptr );

    glutSwapBuffers();
    glutPostRedisplay();
} 