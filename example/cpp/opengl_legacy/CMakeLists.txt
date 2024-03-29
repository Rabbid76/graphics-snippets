# for mingw, the order of the libraries seems to matter: glew before opengl32
set(GLUT_LINK_LIBRARIES ${GLEW_LIB} ${DEFAULT_LIB} ${GLUT_LIB})
set(GLFW_LINK_LIBRARIES ${GLEW_LIB} ${DEFAULT_LIB} ${GLFW_LIB} ${FREETYPE_LIB})

if (MAKE_CONFIG STREQUAL "MSVC" OR MAKE_CONFIG STREQUAL "MINGW")
	#file(COPY ../${GLUT_PATH}/bin/x64/freeglut.dll DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
	#file(COPY ../${GLFW_LIBRARY_PATH}/glfw3.dll DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
endif()

function(glut_exe target_name)
	add_executable(${target_name} ${ARGN})
	target_link_libraries(${target_name} ${GLUT_LINK_LIBRARIES})
endfunction()

function(glfw_exe target_name)
	add_executable(${target_name} ${ARGN})
	target_link_libraries(${target_name} ${GLFW_LINK_LIBRARIES})
endfunction()

# GLUT
glut_exe(example_beginend_freeglut_1 example_beginend_freeglut_1.cpp)
glut_exe(example_beginend_cube example_beginend_cube.cpp)
glut_exe(example_beginend_texture_1 example_beginend_texture_1.cpp)
glut_exe(example_beginend_texture_stb_1 example_beginend_texture_stb_1.cpp)
glut_exe(example_fixedattribute_cube_1 example_fixedattribute_cube_1.cpp)
glut_exe(example_fixedattribute_texture_stb_1 example_fixedattribute_texture_stb_1.cpp)
glut_exe(example_glu_sphere_1 example_glu_sphere_1.cpp)
glut_exe(example_fixedattriubte_spring example_fixedattribute_spring.cpp)

# GLFW
glfw_exe(example_beginend_shader_1 example_beginend_shader_1.cpp)
glfw_exe(example_beginend_text_freetype example_beginend_text_freetype.cpp)
glfw_exe(example_fixedattribute_1 example_fixedattribute_1.cpp)
glfw_exe(example_fixedattribute_text_freetype example_fixedattribute_text_freetype.cpp)
glfw_exe(example_shader_luminance_text_freetype example_shader_luminance_text_freetype.cpp)
glfw_exe(example_fixedattribute_sphere_1_slices example_fixedattribute_sphere_1_slices.cpp)
glfw_exe(example_glfw_fps example_glfw_fps.cpp)
