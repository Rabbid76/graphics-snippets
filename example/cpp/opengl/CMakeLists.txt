set(GLUT_LINK_LIBRARIES opengl32 ${GLEW_LIB} ${GLUT_LIB})
set(GLFW_LINK_LIBRARIES opengl32 ${GLEW_LIB} ${GLFW_LIB})

if (MAKE_CONFIG STREQUAL "MSVC")
	add_definitions(-DWIN32 -DGLEW_STATIC)
	# set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /NODEFAULTLIB:\"LIBCMT.LIB\"")
	file(COPY ../${GLUT_PATH}/bin/x64/freeglut.dll DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
	file(COPY ../${GLFW_LIBRARY_PATH}/glfw3.dll DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
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
glut_exe(example_freeglut_1 example_freeglut_1.cpp)
glut_exe(example_concave_polygon_stencil example_concave_polygon_stencil.cpp)
glut_exe(example_fistperson_1 example_fistperson_1.cpp)
glut_exe(example_framebuffer_1 example_framebuffer_1.cpp)
glut_exe(example_map_buffer_1 example_map_buffer_1.cpp)

# GLFW
glfw_exe(example_glfw_1 example_glfw_1.cpp)
glfw_exe(example_glfw_2_perpspcetive example_glfw_2_perpspcetive.cpp)
glfw_exe(example_glfw_fps example_glfw_fps.cpp)
glfw_exe(example_glm_animation_1 example_glm_animation_1.cpp)
glfw_exe(example_barycentric_cordinates_print example_barycentric_cordinates_print.cpp)

# deprecated
# glfw_exe(example_camera_and_matrix example_camera_and_matrix.cpp)
