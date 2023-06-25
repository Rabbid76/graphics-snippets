cmake_minimum_required (VERSION 3.12)

message("MSVC " ${MSVC})
message("APPLE " ${APPLE})
message("LINUX " ${LINUX})
message("UNIX " ${UNIX})
message("WIN32 " ${WIN32})
message("MSYS " ${MSYS})
message("MINGW " ${MINGW})

if (NOT DEFINED MAKE_CONFIG)
    if (CMAKE_CXX_COMPILER MATCHES .*mingw.*)
        set(MAKE_CONFIG "MINGW")
    elseif(DEFINED WIN32)
        set(MAKE_CONFIG "MSVC")
    elseif(DEFINED APPLE)
        set(MAKE_CONFIG "MacXCode")    
    endif()
endif()

message(STATUS "configuration: " ${MAKE_CONFIG}, ${CMAKE_CXX_COMPILER})

project(hello_triangle_glfw)
set(CMAKE_CXX_STANDARD 20)
set(SUBMODULES_PATH ../../_submodules)

# glew
# see [Automatic Code Generation](http://glew.sourceforge.net/advanced.html)
set(GLEW_PATH ${SUBMODULES_PATH}/glew_opengl_4_6)
add_library(
	glew STATIC
	${GLEW_PATH}/include/gl/glew.h
	${GLEW_PATH}/include/gl/wglew.h
	${GLEW_PATH}/include/gl/eglew.h
	${GLEW_PATH}/include/gl/glxew.h
	${GLEW_PATH}/src/glew.c
	${GLEW_PATH}/src/glewinfo.c
)
target_include_directories(
    glew PUBLIC
    ${SUBMODULES_PATH}/glew_opengl_4_6/include
)
if(DEFINED WIN32)
target_compile_definitions(
	glew PUBLIC
	-DWIN32 -D_LIB -DWIN32_LEAN_AND_MEAN -DFREEGLUT_STATIC
)
elseif(DEFINED APPLE)
target_compile_definitions(
	glew PUBLIC
	#-DGLEW_OSMESA -D_LIB -DFREEGLUT_STATIC
	-D_LIB -DFREEGLUT_STATIC
)
endif()

# glut
set(GLUT_PATH ${SUBMODULES_PATH}/freeglut)
add_subdirectory(${SUBMODULES_PATH}/freeglut _submodules/freeglut)
if(MAKE_CONFIG STREQUAL "MSVC") 
    target_compile_options(freeglut PUBLIC /wd4312)
	target_compile_options(freeglut_static PUBLIC /wd4312)
endif()

if(MAKE_CONFIG STREQUAL "MSVC" OR MAKE_CONFIG STREQUAL "MINGW")
	add_definitions(-DWIN32 -DFREEGLUT_STATIC -D_WIN32)
elseif(MAKE_CONFIG STREQUAL "MacXCode")
	add_definitions(-DGL_SILENCE_DEPRECATION)
endif()

if(MAKE_CONFIG STREQUAL "MSVC")
    set(DEFAULT_LIB opengl32 glu32)
elseif (MAKE_CONFIG STREQUAL "MINGW")
    set(DEFAULT_LIB gdi32 opengl32 glu32)
elseif (MAKE_CONFIG STREQUAL "Clang")
    # [...]
elseif (MAKE_CONFIG STREQUAL "MacXCode")
set(DEFAULT_LIB "-framework OpenGL")
    # [...]    
endif()

set(GLEW_LIB glew)
set(GLUT_LIB freeglut_static)

set(GLFW_LINK_LIBRARIES ${GLEW_LIB} ${DEFAULT_LIB} ${GLUT_LIB})

add_executable(
    hello_triangle
    hello_triangle_glut.cpp
)
target_link_libraries(
    hello_triangle PUBLIC
    ${GLEW_LIB} ${DEFAULT_LIB} ${GLUT_LIB}
)
target_include_directories(
    hello_triangle PUBLIC
    ${SUBMODULES_PATH}/glm
    ${GLEW_PATH}/include
    ${GLUT_PATH}/include
)
