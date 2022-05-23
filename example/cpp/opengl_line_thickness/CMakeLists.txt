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
glfw_exe(opengl_thick_line_linewidth thick_line_linewidth.cpp)
glfw_exe(opengl_thick_line_strip_ssbo thick_line_strip_ssbo.cpp)
glfw_exe(opengl_thick_line_strip_ssbo_2 thick_line_strip_ssbo_2.cpp)