# for mingw, the order of the libraries seems to matter: glew before opengl32
set(GLUT_LINK_LIBRARIES ${GLEW_LIB} ${DEFAULT_LIB} ${GLUT_LIB} ${FREETYPE_LIB})
set(GLFW_LINK_LIBRARIES ${GLEW_LIB} ${DEFAULT_LIB} ${GLFW_LIB} ${FREETYPE_LIB})
set(SDL_LINK_LIBRARIES ${GLEW_LIB} ${DEFAULT_LIB} ${SDL_LIB} ${FREETYPE_LIB})
set(SFML_LINK_LIBRARIES ${GLEW_LIB} ${DEFAULT_LIB} ${SFML_LIB} ${FREETYPE_LIB})

if (MAKE_CONFIG STREQUAL "MSVC" OR MAKE_CONFIG STREQUAL "MINGW")
	add_definitions(-DWIN32 -DGLEW_STATIC -D_WIN32 -DFREEGLUT_STATIC -DSFML_STATIC)
	# set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /NODEFAULTLIB:\"LIBCMT.LIB\"")
	#file(COPY ../${GLUT_PATH}/bin/x64/freeglut.dll DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
	#file(COPY ../${GLFW_LIBRARY_PATH}/glfw3.dll DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
elseif (MAKE_CONFIG STREQUAL "MacXCode")  	
	#add_definitions(-DOPENGL_gl_LIBRARY -DDOPENGL_glu_LIBRARY)
	#add_definitions(-DGLEW_STATIC)
	#add_link_options("-lz" "-liconv")
endif()

function(glut_exe target_name)
    add_executable(${target_name} ${ARGN})
	target_link_libraries(${target_name} ${GLUT_LINK_LIBRARIES})
endfunction()

function(glfw_exe target_name)
	add_executable(${target_name} ${ARGN})
	target_link_libraries(${target_name} ${GLFW_LINK_LIBRARIES})
endfunction()

function(sdl_exe target_name)
	add_executable(${target_name} ${ARGN})
	target_link_libraries(${target_name} ${SDL_LINK_LIBRARIES})
endfunction()

function(sfml_exe target_name)
	add_executable(${target_name} ${ARGN})
	target_link_libraries(${target_name} ${SFML_LINK_LIBRARIES})
endfunction()


# GLUT
if(NOT MAKE_CONFIG STREQUAL "MacXCode")
glut_exe(example_freeglut_1 example_freeglut_1.cpp)
glut_exe(example_freeglut_text_freetype example_freeglut_text_freetype.cpp)
glut_exe(example_concave_polygon_stencil example_concave_polygon_stencil.cpp)
glut_exe(example_fistperson_1 example_fistperson_1.cpp)
glut_exe(example_framebuffer_1 example_framebuffer_1.cpp)
glut_exe(example_map_buffer_1 example_map_buffer_1.cpp)
glut_exe(example_texture_1_stb_freeglut example_texture_1_stb_freeglut.cpp)
glut_exe(example_primitive_restart_1 example_primitive_restart_1.cpp)
glut_exe(example_shader_integer_attribute_1 example_shader_integer_attribute_1.cpp)
glut_exe(example_texture_5_test_texsubiamge example_texture_5_test_texsubiamge.cpp)
endif()

# GLFW
glfw_exe(example_glfw_1 example_glfw_1.cpp)
glfw_exe(example_glfw_text_freetype example_glfw_text_freetype.cpp)
glfw_exe(example_glfw_2_perpspcetive example_glfw_2_perpspcetive.cpp)
glfw_exe(example_glm_animation_1 example_glm_animation_1.cpp)
glfw_exe(example_barycentric_cordinates_print example_barycentric_cordinates_print.cpp)
glfw_exe(example_mesh_1_instancing example_mesh_1_instancing.cpp)
glfw_exe(example_mesh_1 example_mesh_1.cpp)

if(NOT MAKE_CONFIG STREQUAL "MacXCode")
glfw_exe(example_mesh_2_tetrahedron example_mesh_2_tetrahedron.cpp)
glfw_exe(example_mesh_3_a_cube example_mesh_3_a_cube.cpp)
glfw_exe(example_mesh_3_b_cube example_mesh_3_b_cube.cpp)
glfw_exe(example_mesh_3_c_cube example_mesh_3_c_cube.cpp)
glfw_exe(example_mesh_4_cube_flat example_mesh_4_cube_flat.cpp)
glfw_exe(example_mesh_5 example_mesh_5.cpp)
glfw_exe(example_mesh_6_filter example_mesh_6_filter.cpp)
glfw_exe(example_mesh_7_raytrace example_mesh_7_raytrace.cpp)
glfw_exe(example_mesh_8_surface_of_revolution example_mesh_8_surface_of_revolution.cpp)
glfw_exe(example_navigate_1_orbit example_navigate_1_orbit.cpp)
glfw_exe(example_noise_1 example_noise_1.cpp)
glfw_exe(example_shader_geometry_1_line example_shader_geometry_1_line.cpp)
glfw_exe(example_shader_tessellation_1 example_shader_tessellation_1.cpp)
glfw_exe(example_shader_vbo_byte example_shader_vbo_byte.cpp)
glfw_exe(example_ssbo_1 example_ssbo_1.cpp)
glfw_exe(example_texture_2_stb_glfw example_texture_2_stb_glfw.cpp)
glfw_exe(example_texture_3_lookup_table example_texture_3_lookup_table.cpp)
glfw_exe(example_thread_1 example_thread_1.cpp)
glfw_exe(example_thread_2_shared_context example_thread_2_shared_context.cpp)
endif()

# deprecated
# glfw_exe(example_camera_and_matrix example_camera_and_matrix.cpp)
# glfw_exe(example_texture_4_transform example_texture_4_transform.cpp)

# SDL
sdl_exe(example_sdl_1 example_sdl_1.cpp)

# SFML
sfml_exe(example_sfml_1 example_sfml_1.cpp)
