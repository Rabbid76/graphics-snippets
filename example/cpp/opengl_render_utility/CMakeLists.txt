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

glut_exe(
	cone_step_map_generator
	cone_step_map_generator.cpp
)

glut_exe(
	example_map_generator
	example_map_generator.cpp
)

if(MAKE_CONFIG STREQUAL "MSVC") 
glfw_exe(
	depth_pack_test
	depth_pack_test.cpp
	../_render_util/source/OpenGL/OpenGLProgram.cpp
	../_render_util/source/OpenGL/OpenGLFramebuffer.cpp
	../_render_util/source/OpenGL/OpenGLVertexBuffer.cpp
)
endif()

glfw_exe(
	simple_glsl_subroutine
	simple_glsl_subroutine.cpp
)

glut_exe(
	fixed_function_pipeline_matrix_stack
	fixed_function_pipeline_matrix_stack.cpp
)

glfw_exe(
	fixed_function_attibutes_glfw
	fixed_function_attibutes_glfw.cpp
)

glfw_exe(
	line_render
	line_render.cpp
	../_render_util/source/OpenGL/OpenGLError.cpp
	../_render_util/source/OpenGL/OpenGLProgram.cpp
	../_render_util/source/OpenGL/OpenGLDataBuffer_std140.cpp
	../_render_util/source/OpenGL/OpenGLVertexBuffer.cpp
	../_render_util/source/OpenGL/OpenGLPrimitive_2_0.cpp
	../_render_util/source/OpenGL/OpenGLPrimitive_core_and_es.cpp
	../_render_util/source/OpenGL/OpenGLLine_1_0.cpp
	../_render_util/source/OpenGL/OpenGLLine_2_0.cpp
	../_render_util/source/OpenGL/OpenGLLine_core_and_es.cpp
	../_render_util/source/OpenGL/OpenGLLine_highquality.cpp
)

glfw_exe(
	polygon_render
	polygon_render.cpp
	../_render_util/source/OpenGL/OpenGLError.cpp
	../_render_util/source/OpenGL/OpenGLProgram.cpp
	../_render_util/source/OpenGL/OpenGLDataBuffer_std140.cpp
	../_render_util/source/OpenGL/OpenGLVertexBuffer.cpp
	../_render_util/source/OpenGL/OpenGLPrimitive_2_0.cpp
	../_render_util/source/OpenGL/OpenGLPrimitive_core_and_es.cpp
	../_render_util/source/OpenGL/OpenGLPolygon_1_0.cpp
	../_render_util/source/OpenGL/OpenGLPolygon_2_0.cpp
	../_render_util/source/OpenGL/OpenGLPolygon_core_and_es.cpp
)

glfw_exe(
	rubiks_cube_glfw
	rubiks_cube_glfw.cpp
	../_render_util/source/OpenGL/OpenGLError.cpp
	../_render_util/source/OpenGL/OpenGLProgram.cpp
)

if(MAKE_CONFIG STREQUAL "MSVC") 
glfw_exe(
	draw_library_scenes
	draw_library_scenes.cpp
	../_render_util/source/OpenGL/OpenGLError.cpp
	../_render_util/source/OpenGL/OpenGLProgram.cpp
	../_render_util/source/OpenGL/OpenGLVertexBuffer.cpp
	../_render_util/source/OpenGL/OpenGLFramebuffer.cpp
	../_render_util/source/OpenGL/OpenGLTextureLoader.cpp
	../_render_util/source/OpenGL/OpenGLBasicDraw.cpp
	../_render_util/source/util/RenderUtil_FreetypeFont.cpp
)
endif()
