# for mingw, the order of the libraries seems to matter: glew before opengl32
set(GLFW_LINK_LIBRARIES ${GLEW_LIB} ${DEFAULT_LIB} ${GLFW_LIB} ${FREETYPE_LIB})

if (MAKE_CONFIG STREQUAL "MSVC" OR MAKE_CONFIG STREQUAL "MINGW")
	#file(COPY ../${GLFW_LIBRARY_PATH}/glfw3.dll DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
endif()

function(glfw_exe target_name)
	add_executable(${target_name} ${ARGN})
	target_link_libraries(${target_name} ${GLFW_LINK_LIBRARIES})
endfunction()

# GLFW
glfw_exe(opengl_dashed_line_segments dashed_line_segments.cpp)
glfw_exe(opengl_dashed_line_strip dashed_line_strip.cpp)
glfw_exe(opengl_stippled_line_fixedattribute_glLineStipple stippled_line_fixedattribute_glLineStipple.cpp)
glfw_exe(opengl_stippled_line_fixedattribute_texture_alpha_OpenGLES100 stippled_line_fixedattribute_texture_alpha_OpenGLES100.cpp)
glfw_exe(opengl_stippled_line_gllinestipple stippled_line_gllinestipple.cpp)
glfw_exe(opengl_stippled_line_segments stippled_line_segments.cpp)
glfw_exe(opengl_stippled_line_shader stippled_line_shader.cpp)

