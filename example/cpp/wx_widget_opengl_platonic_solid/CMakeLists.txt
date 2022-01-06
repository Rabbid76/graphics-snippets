include_directories(
	../
)

add_executable(
	wx_widget_opengl_platonic_solid 
	wx_widget_opengl_platonic_solid.cpp
	../_util/source/controls/spinning_controls.cpp
	../_util/source/gl/opengl_mesh_single.cpp
	../_util/source/gl/opengl_mesh_single_separate_attribute.cpp
	../_util/source/wxutil/wx_opengl_canvas.cpp
)

target_link_libraries(
	wx_widget_opengl_platonic_solid 
	${DEFAULT_LIB} 
	${GLEW_LIB}
	${WXWIDGET_LIB}
)

if (MAKE_CONFIG STREQUAL "MSVC")
	target_compile_definitions(wx_widget_opengl_platonic_solid PUBLIC -DWIN32 -DGLEW_STATIC -DWXUSINGDLL -D_UNICODE -D_CONSOLE)
	target_link_options(wx_widget_opengl_platonic_solid PUBLIC /SUBSYSTEM:WINDOWS)
	#file(COPY ../${WXWIDGET_LIBRARY_PATH}/wxbase*.dll DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
	#file(COPY ../${WXWIDGET_LIBRARY_PATH}/wxbase*.pdb DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
	#file(COPY ../${WXWIDGET_LIBRARY_PATH}/wxmsw*.dll DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
	#file(COPY ../${WXWIDGET_LIBRARY_PATH}/wxmsw*.pdb DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
endif()