set(GLUT_LINK_LIBRARIES opengl32 glu32 ${GLEW_LIB} ${GLUT_LIB} ${FREETYPE_LIB})
set(GLFW_LINK_LIBRARIES opengl32 glu32 ${GLEW_LIB} ${GLFW_LIB} ${FREETYPE_LIB})

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

glut_exe(
	simple_glut
	simple_glut.cpp
)

glfw_exe(
	simple_glfw
	simple_glfw.cpp
)

glfw_exe(
	cube_glfw
	cube_glfw.cpp
	../_render_util/source/OpenGL/OpenGLError.cpp
	../_render_util/source/OpenGL/OpenGLProgram.cpp
)

glfw_exe(
	transform_feedback_simple
	transform_feedback_simple.cpp
	../_render_util/source/OpenGL/OpenGLError.cpp
	../_render_util/source/OpenGL/OpenGLProgram.cpp
)

glfw_exe(
	cone_step_map_generator
	cone_step_map_generator.cpp
)

glfw_exe(
	example_map_generator
	example_map_generator.cpp
)

glfw_exe(
	depth_pack_test
	depth_pack_test.cpp
	../_render_util/source/OpenGL/OpenGLProgram.cpp
	../_render_util/source/OpenGL/OpenGLFramebuffer.cpp
	../_render_util/source/OpenGL/OpenGLVertexBuffer.cpp
)

glfw_exe(
	fixed_function_pipeline_matrix_stack
	fixed_function_pipeline_matrix_stack.cpp
)

glfw_exe(
	fixed_function_attibutes_glfw
	fixed_function_attibutes_glfw.cpp
)
