# for mingw, the order of the libraries seems to matter: glew before opengl32
set(WINDOWS_LINK_LIBRARIES ${GLEW_LIB} ${DEFAULT_LIB})

function(windows_exe target_name)
	add_executable(${target_name} ${ARGN})
	target_compile_definitions(${target_name} PUBLIC UNICODE _UNICODE)
	target_link_libraries(${target_name} ${WINDOWS_LINK_LIBRARIES})
endfunction()

function(windows_winmain_exe target_name)
    # If WIN32 is given the property WIN32_EXECUTABLE will be set on the target created.
	# see https://cmake.org/cmake/help/v3.0/command/add_executable.html
	add_executable(${target_name} WIN32 ${ARGN})
	target_compile_definitions(${target_name} PUBLIC UNICODE _UNICODE)
	target_link_libraries(${target_name} ${WINDOWS_LINK_LIBRARIES})
endfunction()

windows_exe(example_wgl_1 example_wgl_1.cpp)
windows_winmain_exe(example_wglew_1 example_wglew_1.cpp)
