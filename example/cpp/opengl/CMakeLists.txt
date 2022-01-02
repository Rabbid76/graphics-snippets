
set(GLFW_EXECUTABLE_LIBRARIES opengl32 ${GLEW_LIB} ${GLFW_LIB})

if (MAKE_CONFIG STREQUAL "MSVC")
	add_definitions(-DWIN32 -DGLEW_STATIC)
#  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /NODEFAULTLIB:\"LIBCMT.LIB\"")
	file(COPY ../${GLFW_LIBRARY_PATH}/glfw3.dll DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
endif()

function(glfw_exe target_name)
	add_executable(${target_name} ${ARGN})
	target_link_libraries(${target_name} ${GLFW_EXECUTABLE_LIBRARIES})
endfunction()

glfw_exe(example_glfw_1 example_glfw_1.cpp)
glfw_exe(example_glfw_2_perpspcetive example_glfw_2_perpspcetive.cpp)
glfw_exe(example_glfw_fps example_glfw_fps.cpp)
glfw_exe(example_glm_animation_1 example_glm_animation_1.cpp)