
# directories
dirNameOut = ../../../bin
libraryDir = ../../../../_cpp_extern

# project includes
utilDir = ../../../example/cpp/_util
renderDir = ../../../cpp/include/render
utilInc = -I./ -I../ -I$(utilDir)/include -I$(renderDir)

# GLM
glmDir = ${libraryDir}/glm-0.9.9.3/glm
glmInc = -I$(glmDir)

# STB
stbDir = ${libraryDir}/stb
stbInc = -I$(stbDir)

# GLEW
glewDir = ${libraryDir}/glew-2.1.0
glewInc = -D GLEW_STATIC -I$(glewDir)/include
glewLib = -L$(glewDir)/lib\Release\x64 -lglew32s

# wxWidget
wxDir = ${libraryDir}/wxMSW-3.1.5_gcc1020
wxInc = --define __WXMSW__ -D WXUSINGDLL -I$(wxDir)/include -I$(wxDir)/lib/gcc1020_x64_dll/mswu
wxLib = -L$(wxDir)/lib/gcc1020_x64_dll -lwxbase31ud

# includes
INCLUDEALL = $(utilInc) $(glmInc) $(stbInc) $(glewInc) $(wxInc)

# öibraries
LIBALL = $(glewLib) $(wxLib)

all: target	$(dirNameOut)/wx_widget_opengl_meshs.exe

$(dirNameOut)/wx_widget_opengl_meshs.exe: $(dirNameOut)/opengl_mesh_single_separate_attribute.o $(dirNameOut)/wx_opengl_canvas.o $(dirNameOut)/wx_widget_opengl_meshs.o
	g++ -g -std=c++17 -Og -static -Wl,--subsystem,windows -mwindows -o $@ $< $(LIBALL) -lgdi32 -lopengl32 -lglu32

$(dirNameOut)/opengl_mesh_single_separate_attribute.o: $(utilDir)/source/gl/opengl_mesh_single_separate_attribute.cpp
	g++ -g -std=c++17 -Og -c -o $@ $< $(INCLUDEALL)

$(dirNameOut)/wx_opengl_canvas.o: $(utilDir)/source/wxutil/wx_opengl_canvas.cpp
	g++ -g -std=c++17 -Og -c -o $@ $< $(INCLUDEALL)

$(dirNameOut)/wx_widget_opengl_meshs.o: wx_widget_opengl_meshs.cpp
	g++ -g -std=c++17 -Og -c -o $@ $< $(INCLUDEALL)

target: ${dirNameOut}

${dirNameOut}:
	mkdir -p $@


