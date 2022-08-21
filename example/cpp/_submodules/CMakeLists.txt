
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
if(DEFINED WIN32)
target_compile_definitions(
	glew PUBLIC
	-DWIN32 -D_LIB -DWIN32_LEAN_AND_MEAN -DGLEW_STATIC
)
elseif(DEFINED APPLE)
target_compile_definitions(
	glew PUBLIC
	#-DGLEW_OSMESA -D_LIB -DGLEW_STATIC
	-D_LIB -DGLEW_STATIC
)
endif()

# freetype
add_subdirectory(freetype)
if(MAKE_CONFIG STREQUAL "MSVC") 
	target_compile_options(freetype PUBLIC /wd4244 /wd4267)
endif()

# freeglut
if(NOT MAKE_CONFIG STREQUAL "MacXCode")
add_subdirectory(freeglut)
if(MAKE_CONFIG STREQUAL "MSVC") 
    target_compile_options(freeglut PUBLIC /wd4312)
	target_compile_options(freeglut_static PUBLIC /wd4312)
endif()
endif()

# glfw
add_subdirectory(glfw)

# sdl
if(NOT MAKE_CONFIG STREQUAL "MacXCode")
add_subdirectory(sdl)
endif()

# sfml
if(NOT MAKE_CONFIG STREQUAL "MacXCode")
add_subdirectory(sfml)
if(MAKE_CONFIG STREQUAL "MSVC") 
    #target_compile_options(SFML PRIVATE /wd9025)
endif()
endif()

# wxwindgets
#add_subdirectory(wxwidgets)