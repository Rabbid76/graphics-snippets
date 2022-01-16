
include_directories(
	glew_opengl_4_6/include
)

# glew
# see [Automatic Code Generation](http://glew.sourceforge.net/advanced.html)
add_library(
	glew STATIC
	glew_opengl_4_6/include/gl/glew.h
	glew_opengl_4_6/include/gl/wglew.h
	glew_opengl_4_6/include/gl/eglew.h
	glew_opengl_4_6/include/gl/glxew.h
	glew_opengl_4_6/src/glew.c
	glew_opengl_4_6/src/glewinfo.c
)
target_compile_definitions(
	glew PUBLIC
	-DWIN32 -D_LIB -DWIN32_LEAN_AND_MEAN -DGLEW_STATIC
)

# freetype
add_subdirectory(freetype)
if(MAKE_CONFIG STREQUAL "MSVC") 
	target_compile_options(freetype PUBLIC /wd4244 /wd4267)
endif()

# freeglut
add_subdirectory(freeglut/freeglut/freeglut)
if(MAKE_CONFIG STREQUAL "MSVC") 
    target_compile_options(freeglut PUBLIC /wd4312)
	target_compile_options(freeglut_static PUBLIC /wd4312)
endif()

# glfw
#add_subdirectory(glfw)