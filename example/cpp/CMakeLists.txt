cmake_minimum_required (VERSION 3.12)

project(cpp_examples)

#set_property(TARGET tgt PROPERTY CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD 20)

set(SUBMODULES_PATH _submodules)
add_subdirectory(${SUBMODULES_PATH})

set(EXTERN_LIBRARY_PATH ../../../_cpp_extern)
set(STB_PATH ${SUBMODULES_PATH}/stb)
#set(STB_PATH ${EXTERN_LIBRARY_PATH}/stb)
set(GLM_PATH ${EXTERN_LIBRARY_PATH}/glm-0.9.9.8)
set(GLEW_PATH ${SUBMODULES_PATH}/glew_opengl_4_6)
#set(GLEW_PATH ${EXTERN_LIBRARY_PATH}/glew-2.1.0)
set(FREETYPE_PATH ${SUBMODULES_PATH}/freetype)
#set(FREETYPE_PATH ${EXTERN_LIBRARY_PATH}/freetype-2.11.1)
set(GLFW_PATH ${EXTERN_LIBRARY_PATH}/glfw-3.3.6.bin.WIN64)
set(WXWIDGET_PATH ${EXTERN_LIBRARY_PATH}/wxMSW-3.1.5_vc14x_x64)

# compiler options
if(MAKE_CONFIG STREQUAL "MSVC")
    set(GLUT_PATH ${EXTERN_LIBRARY_PATH}/freeglut/freeglut-3.0.0)
elseif (MAKE_CONFIG STREQUAL "MINGW")
    set(GLUT_PATH ${EXTERN_LIBRARY_PATH}/freeglut/freeglut-MinGW-3.0.0-1.mp/freeglut)
elseif (MAKE_CONFIG STREQUAL "Clang")
    # [...]
endif()

# compiler options
if(MAKE_CONFIG STREQUAL "MSVC")
    # [...]
elseif(MAKE_CONFIG STREQUAL "MINGW")
    add_compile_options(
        -Wno-volatile
    )
elseif(MAKE_CONFIG STREQUAL "Clang")
    # [...]
endif()

# include directories
include_directories(
    ${STB_PATH}
    ${GLEW_PATH}/include
    ${GLM_PATH}/glm
    ${GLUT_PATH}/include
    ${GLFW_PATH}/include
    ${WXWIDGET_PATH}/include
    ${WXWIDGET_PATH}/include/msvc
    ${FREETYPE_PATH}/include
    _util/include
    _render_util/include
)

# link directories
#set(GLEW_LIBRARY_PATH ${GLEW_PATH}/lib/Release/x64)
if(MAKE_CONFIG STREQUAL "MSVC")
    set(GLFW_LIBRARY_PATH ${GLFW_PATH}/lib-vc2022)
elseif (MAKE_CONFIG STREQUAL "MINGW")
    set(GLFW_LIBRARY_PATH ${GLFW_PATH}/lib-mingw-w64)
elseif (MAKE_CONFIG STREQUAL "Clang")
    # [...]
endif()
set(WXWIDGET_LIBRARY_PATH ${WXWIDGET_PATH}/lib/vc_x64_dll)
link_directories(
    ${GLEW_LIBRARY_PATH}
    ${GLUT_PATH}/lib/x64
    ${GLFW_LIBRARY_PATH}
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
endif()

# GLUT
set(GLUT_LIB freeglut)

# GLFW
set(GLFW_LIB glfw3)

# wxWidget
set(WXWIDGET_LIB wxbase31ud)

# freetype
set(FREETYPE_LIB freetype)

#add_library(glew SHARED STATIC IMPORTED)
#set_target_properties(glew PROPERTIES IMPORTED_LOCATION ${GLEW_PATH}/lib/Release/x64/glew32s.lib)

# include sub-projects
add_subdirectory(opengl)
add_subdirectory(opengl_legacy)
if(MAKE_CONFIG STREQUAL "MSVC")
    add_subdirectory(opengl_line_stipple)
    add_subdirectory(opengl_line_thickness)
    add_subdirectory(wx_widget_opengl_meshs)
    add_subdirectory(wx_widget_opengl_platonic_solid)
    add_subdirectory(opengl_render_utility)
elseif (MAKE_CONFIG STREQUAL "MINGW")
    
endif()
