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

project(cpp_examples)

#set_property(TARGET tgt PROPERTY CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD 20)
if(MAKE_CONFIG STREQUAL "MacXCode")
    enable_language(OBJC)
endif()

set(SUBMODULES_PATH _submodules)
add_subdirectory(${SUBMODULES_PATH})
set(GLEW_PATH ${SUBMODULES_PATH}/glew_opengl_4_6)
set(GLM_PATH ${SUBMODULES_PATH}/glm)
set(STB_PATH ${SUBMODULES_PATH}/stb)
set(FREETYPE_PATH ${SUBMODULES_PATH}/freetype)
set(GLUT_PATH ${SUBMODULES_PATH}/freeglut)
set(GLFW_PATH ${SUBMODULES_PATH}/glfw)
set(SDL_PATH ${SUBMODULES_PATH}/sdl)
set(SFML_PATH ${SUBMODULES_PATH}/sfml)

set(EXTERN_LIBRARY_PATH ../../../_cpp_extern)
#set(GLM_PATH ${EXTERN_LIBRARY_PATH}/glm-0.9.9.8)
#set(STB_PATH ${EXTERN_LIBRARY_PATH}/stb)
#set(GLEW_PATH ${EXTERN_LIBRARY_PATH}/glew-2.1.0)
#set(FREETYPE_PATH ${EXTERN_LIBRARY_PATH}/freetype-2.11.1)
#set(GLUT_PATH ${EXTERN_LIBRARY_PATH}/freeglut/freeglut-3.0.0)
#set(GLFW_PATH ${EXTERN_LIBRARY_PATH}/glfw-3.3.6.bin.WIN64)
set(WXWIDGET_PATH ${EXTERN_LIBRARY_PATH}/wxMSW-3.1.5_vc14x_x64)

# compiler options
if(MAKE_CONFIG STREQUAL "MSVC")
    # [...]
elseif(MAKE_CONFIG STREQUAL "MINGW")
    #add_compile_options(
    #    -Wno-volatile
    #    -Wno-narrowing
    #    -Wno-return-type
    #)
elseif(MAKE_CONFIG STREQUAL "Clang")
    # [...]
elseif(MAKE_CONFIG STREQUAL "MacXCode")
    # [...]    
endif()

# include directories
include_directories(
    ${STB_PATH}
    ${GLM_PATH}
    ${GLEW_PATH}/include
    ${GLUT_PATH}/include
    ${GLFW_PATH}/include
    ${SDL_PATH}/include
    ${SFML_PATH}/include
    ${WXWIDGET_PATH}/include
    ${WXWIDGET_PATH}/include/msvc
    ${FREETYPE_PATH}/include
    _util/include
    _render_util/include
)

# global prprocessor defintions
if(MAKE_CONFIG STREQUAL "MSVC" OR MAKE_CONFIG STREQUAL "MINGW")
	add_definitions(-DWIN32 -DGLEW_STATIC -DFREEGLUT_STATIC)
elseif(MAKE_CONFIG STREQUAL "MacXCode")
	add_definitions(-DGL_SILENCE_DEPRECATION)
endif()

# link directories
#set(GLEW_LIBRARY_PATH ${GLEW_PATH}/lib/Release/x64)
set(WXWIDGET_LIBRARY_PATH "unknown")
if(MAKE_CONFIG STREQUAL "MSVC")
    #set(GLFW_LIBRARY_PATH ${GLFW_PATH}/lib-vc2022)
    set(WXWIDGET_LIBRARY_PATH ${WXWIDGET_PATH}/lib/vc_x64_dll)
elseif (MAKE_CONFIG STREQUAL "MINGW")
    #set(GLFW_LIBRARY_PATH ${GLFW_PATH}/lib-mingw-w64)
elseif (MAKE_CONFIG STREQUAL "Clang")
    # [...]
endif()
link_directories(
    #${GLEW_LIBRARY_PATH}
    #${GLUT_PATH}/lib/x64
    #${GLFW_LIBRARY_PATH}
    ${WXWIDGET_LIBRARY_PATH}
)

# GLEW
set(GLEW_LIB glew)
#set(GLEW_LIB glew32s)

# default libraries
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

# freetype
set(FREETYPE_LIB freetype)

# GLUT
set(GLUT_LIB freeglut_static)

# GLFW
set(GLFW_LIB glfw)

# SDL
set(SDL_LIB SDL3-static)

# SFML
set(SFML_LIB SFML::Window SFML::Graphics SFML::System)

# wxWidget
get_filename_component(FULL_WXWIDGET_LIBRARY_PATH "${WXWIDGET_LIBRARY_PATH}" REALPATH)
if(EXISTS ${FULL_WXWIDGET_LIBRARY_PATH})
    set(WXWIDGET_LIB wxbase31ud)
endif()

#add_library(glew SHARED STATIC IMPORTED)
#set_target_properties(glew PROPERTIES IMPORTED_LOCATION ${GLEW_PATH}/lib/Release/x64/glew32s.lib)

# include sub-projects
add_subdirectory(opengl)

if(NOT MAKE_CONFIG STREQUAL "MacXCode")
    add_subdirectory(opengl_legacy)
    add_subdirectory(opengl_line_stipple)
    add_subdirectory(opengl_line_thickness)
    add_subdirectory(opengl_render_utility)
endif()

if(EXISTS ${FULL_WXWIDGET_LIBRARY_PATH})
    add_subdirectory(wx_widget_opengl_meshs)
    add_subdirectory(wx_widget_opengl_platonic_solid)
endif()

if(MAKE_CONFIG STREQUAL "MSVC" OR MAKE_CONFIG STREQUAL "MINGW")
    add_subdirectory(windows_opengl)
endif()