set(GLFW_LINK_LIBRARIES ${DEFAULT_LIB} ${GLEW_LIB} ${GLFW_LIB})

if (MAKE_CONFIG STREQUAL "MSVC" OR MAKE_CONFIG STREQUAL "MINGW")
	add_definitions(-DWIN32 -DGLEW_STATIC)
	# set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /NODEFAULTLIB:\"LIBCMT.LIB\"")
	file(COPY ../${GLFW_LIBRARY_PATH}/glfw3.dll DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
endif()

function(glfw_exe target_name)
	add_executable(${target_name} ${ARGN})
	target_link_libraries(${target_name} ${GLFW_LINK_LIBRARIES})
endfunction()

# GLFW
glfw_exe(opengl_thick_line_linewidth thick_line_linewidth.cpp)
glfw_exe(opengl_thick_line_strip_ssbo thick_line_strip_ssbo.cpp)