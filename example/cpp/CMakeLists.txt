cmake_minimum_required (VERSION 3.12)

#set_property(TARGET tgt PROPERTY CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD 20)

set(EXTERN_LIBRARY_PATH ../../../_cpp_extern)
set(STB_PATH ${EXTERN_LIBRARY_PATH}/stb)
set(GLM_PATH ${EXTERN_LIBRARY_PATH}/glm-0.9.9.8)
set(GLUT_PATH ${EXTERN_LIBRARY_PATH}/freeglut/freeglut-3.0.0)
set(GLFW_PATH ${EXTERN_LIBRARY_PATH}/glfw-3.3.6.bin.WIN64)

# compiler options
if(MAKE_CONFIG STREQUAL "MSVC")
    set (GLEW_PATH ${EXTERN_LIBRARY_PATH}/glew-2.1.0)
elseif (MAKE_CONFIG STREQUAL "MINGW")
    set (GLEW_PATH _glew)
elseif (MAKE_CONFIG STREQUAL "Clang")
    # [...]
endif()


project(cpp_examples)

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
    "_util/include"
)

# link directories
link_directories(${GLEW_PATH}/lib/Release/x64)
link_directories(${GLUT_PATH}/lib/x64)
set(GLFW_LIBRARY_PATH ${GLFW_PATH}/lib-vc2022)
link_directories(${GLFW_LIBRARY_PATH})

# GLEW
set(GLEW_LIB glew32s)

# GLUT
set(GLUT_LIB freeglut)

# GLFW
set(GLFW_LIB glfw3)

#add_library(glew SHARED STATIC IMPORTED)
#set_target_properties(glew PROPERTIES IMPORTED_LOCATION ${GLEW_PATH}/lib/Release/x64/glew32s.lib)

# include sub-projects
add_subdirectory("opengl")
add_subdirectory("opengl_legacy")