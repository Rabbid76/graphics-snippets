# for mingw, the order of the libraries seems to matter: glew before opengl32
set(WINDOWS_LINK_LIBRARIES ${GLEW_LIB} ${DEFAULT_LIB})

function(windows_exe target_name)
	add_executable(${target_name} ${ARGN})
	target_link_libraries(${target_name} ${WINDOWS_LINK_LIBRARIES})
	target_compile_definitions(${target_name} PUBLIC UNICODE _UNICODE)
endfunction()

windows_exe(example_wgl_1 example_wgl_1.cpp)
windows_exe(example_wglew_1 example_wglew_1.cpp)
