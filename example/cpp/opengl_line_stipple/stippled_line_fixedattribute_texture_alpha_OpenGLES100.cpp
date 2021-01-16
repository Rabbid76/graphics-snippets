// Drawing rectangle with dashed boundary in openGL android
// [https://stackoverflow.com/questions/60188402/drawing-rectangle-with-dashed-boundary-in-opengl-android]
//
// OpenGL ES - Dashed Lines
// [https://stackoverflow.com/questions/37975618/opengl-es-dashed-lines]

#include <stdafx.h>

// OpenGL
#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glu.h>

// GLM [https://glm.g-truc.net/0.9.9/api/index.html]
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// GLFW [https://www.glfw.org/]
#include <GLFW/glfw3.h>


// STL
#include <iostream>
#include <string>
#include <stdexcept>
#include <chrono>
#include <vector>

#define _USE_MATH_DEFINES
#include <cmath>
#include <math.h>

// preprocessor definitions

#ifndef M_PI
#define M_PI 3.14159265358979323846 
#endif

void GLAPIENTRY DebugCallback( 
    unsigned int  source,     //!< I - 
    unsigned int  type,       //!< I - 
    unsigned int  id,         //!< I - 
    unsigned int  severity,   //!< I - 
    int           length,     //!< I - length of debug message
    const char   *message,    //!< I - debug message
    const void   *userParam ) //!< I - user parameter
{
   std::cout << message << std::endl;
}


void init_opengl_debug() {
    if ( glewInit() != GLEW_OK )
        throw std::runtime_error( "error initializing glew" );

    if ( glDebugMessageCallback != nullptr && glDebugMessageControl != nullptr )
    {
        glDebugMessageCallback( &DebugCallback, nullptr );
        glDebugMessageControl( GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE );
        //glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_FALSE);
        //glDebugMessageControl(GL_DEBUG_SOURCE_API, GL_DEBUG_TYPE_ERROR, GL_DONT_CARE, 0, NULL, GL_TRUE);
        glEnable( GL_DEBUG_OUTPUT );
        glEnable( GL_DEBUG_OUTPUT_SYNCHRONOUS );
    }
}

// main

// [Alpha test](https://www.khronos.org/opengl/wiki/Transparency_Sorting#Alpha_test)

int main(void)
{
    if ( glfwInit() == 0 )
        throw std::runtime_error( "error initializing glfw" );

    //glfwWindowHint(GLFW_REFRESH_RATE, 10);

    GLFWwindow *window = glfwCreateWindow( 800, 600, "GLFW OGL window", nullptr, nullptr );
    if ( window == nullptr )
    {
        glfwTerminate();
        throw std::runtime_error( "error initializing window" ); 
    }

    glfwMakeContextCurrent(window);

    init_opengl_debug();

    std::vector<glm::vec3> attribarray1{
        glm::vec3(-1.0f, -1.0f, 0.0f),
        glm::vec3( 1.0f, -1.0f, 5.0f),
        glm::vec3( 1.0f,  1.0f, 0.0f),
        glm::vec3(-1.0f,  1.0f, 5.0f)
    };

    std::vector<glm::vec3> attribarray2;
    for (size_t u=0; u <= 360; u += 8)
    {
        double a = u*M_PI/180.0;
        double c = cos(a), s = sin(a);
        attribarray2.emplace_back(glm::vec3((float)c, (float)s, (float)u / 20.0f));
    }

    GLuint stippleTexObj;
    glGenTextures(1, &stippleTexObj);
    glActiveTexture( GL_TEXTURE0 );
    glBindTexture(GL_TEXTURE_2D, stippleTexObj);

    GLbyte lineStipple[4]{ (GLbyte)255, 0, 0, (GLbyte)255 };
    glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, 4, 1, 0, GL_ALPHA, GL_UNSIGNED_BYTE, lineStipple);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

    glm::mat4(project);
    int vpSize[2]{0, 0};
    while (!glfwWindowShouldClose(window))
    {
        int w, h;
        glfwGetFramebufferSize(window, &w, &h);
        if (w != vpSize[0] ||  h != vpSize[1])
        {
            vpSize[0] = w; vpSize[1] = h;
            glViewport(0, 0, vpSize[0], vpSize[1]);
            float aspect = (float)w/(float)h;
            project = glm::ortho(-aspect, aspect, -1.0f, 1.0f, -10.0f, 10.0f);
        }
             
        glMatrixMode(GL_PROJECTION);
        glLoadMatrixf(glm::value_ptr(project));
        glMatrixMode(GL_MODELVIEW);
             
        glClear(GL_COLOR_BUFFER_BIT);

        glAlphaFunc(GL_GREATER, 0.5);
        glEnable( GL_ALPHA_TEST );
        glEnable(GL_TEXTURE_2D);

        glEnableClientState(GL_VERTEX_ARRAY);
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);

        glPushMatrix();
        glTranslatef( -0.6f, 0.0f, 0.0f );
        glScalef( 0.5f, 0.5f, 0.5f );
        glVertexPointer(3, GL_FLOAT, sizeof(*attribarray1.data()), attribarray1.data());
        glTexCoordPointer(1, GL_FLOAT, sizeof(*attribarray1.data()), ((GLfloat*)attribarray1.data())+2);
        glDrawArrays(GL_LINE_LOOP, 0, (GLsizei)attribarray1.size());
        glPopMatrix();

        glPushMatrix();
        glTranslatef( 0.6f, 0.0f, 0.0f );
        glScalef( 0.5f, 0.5f, 0.5f );
        glVertexPointer(3, GL_FLOAT, sizeof(*attribarray2.data()), attribarray2.data());
        glTexCoordPointer(1, GL_FLOAT, sizeof(*attribarray2.data()), ((GLfloat*)attribarray2.data())+2);
        glDrawArrays(GL_LINE_STRIP, 0, (GLsizei)attribarray2.size());
        glPopMatrix();

        glDisableClientState(GL_VERTEX_ARRAY);
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);
        
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glfwTerminate();

    return 0;
}
    

/*

[Drawing rectangle with dashed boundary in openGL android](https://stackoverflow.com/questions/60188402/drawing-rectangle-with-dashed-boundary-in-opengl-android/60190604#60190604)

---

In OpenGL ES 1.1 you can't use `glLineStipple` and you can't use a shader. But you can use a 1 dimensional texture and the [Alpha test](https://www.khronos.org/opengl/wiki/Transparency_Sorting#Alpha_test). See [OpenGL ES 1.1 Full Specification](https://www.khronos.org/registry/OpenGL/index_es.php).
OpenGL ES 1.1 does not support 1 dimensional textures, too. But that can be substituted by a 2 dimensional Nx1 texture, with ease.

Create 2D (4x1) texture with the internal format `GL_ALPHA` and th following pattern:

```lang-non
1 0 0 1
```

The minifying and magnification function is `GL_NEAREST`. The wrap parameters are `GL_REPEAT` (that is default).

```java
byte arr[] = new byte[] { 255, 0, 0, 255 };
ByteBuffer textureBuffer = ByteBuffer.wrap(arr);

gl2.glGenTextures(1, texture_id_, stippleTexObj);
gl.glBindTexture(GL.GL_TEXTURE_2D, stippleTexObj);
gl.glTexParameteri(GL.GL_TEXTURE_2D, GL.GL_TEXTURE_MIN_FILTER, GL.GL_NEAREST);
gl.glTexParameteri(GL.GL_TEXTURE_2D, GL.GL_TEXTURE_MAG_FILTER, GL.GL_NEAREST);
gl.glTexParameteri(GL.GL_TEXTURE_2D, GL.GL_TEXTURE_WRAP_S, GL.GL_REPEAT);
gl.glTexParameteri(GL.GL_TEXTURE_2D, GL.GL_TEXTURE_WRAP_T, GL.GL_REPEAT);
gl.glTexImage2D(GL.GL_TEXTURE_2D, 0, GL.GL_ALPHA, 4, 1, 0, GL.GL_ALPHA, GL.GL_UNSIGNED_BYTE, textureBuffer);
```

When you draw the lines, the you have to enable the [Alpha test](https://www.khronos.org/opengl/wiki/Transparency_Sorting#Alpha_test) and enable 2 dimensional texturing.

```java
gl.glEnable(GL11.GL_ALPHA_TEST);
gl.glAlphaFunc(GL11.GL_GEQUAL, 0.5f);

gl.glEnable(GL11.GL_TEXTURE_2D);
```

Ensure that the texture coordinates which are associated to the vertices are aligned to integral values when ypu draw the line :

e.g. a quad with bottom left of (-0.5 -0.5) and to right of (0.5, 0.5) and the texture coordinates in range [0, 5]:

```lang-non
 x     y       u   
-0.5f -0.5f    0.0f
 0.5f -0.5f    5.0f
 0.5f  0.5f    0.0f
-0.5f  0.5f    5.0f
```

[![][1]][1]


  [1]: https://i.stack.imgur.com/8mUsx.png
*/