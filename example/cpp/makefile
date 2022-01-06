# GNU make [https://www.gnu.org/software/make/manual/make.html]
# makefile:4: *** missing separator. Stop [https://stackoverflow.com/questions/16931770/makefile4-missing-separator-stop]
# Create directories using make file [https://stackoverflow.com/questions/1950926/create-directories-using-make-file]
# Can I search directory and subdirectories for header files? [https://stackoverflow.com/questions/19738956/can-i-search-directory-and-subdirectories-for-header-files]
# How to run pre- and post-recipes for every target using GNU Make? [https://stackoverflow.com/questions/37952098/how-to-run-pre-and-post-recipes-for-every-target-using-gnu-make]

# make: Nothing to be done for `all' [https://stackoverflow.com/questions/8561640/make-nothing-to-be-done-for-all]
# Sometimes "Nothing to be done for all" error can be caused by spaces before command in makefile rule instead of tab. Please ensure that you use tabs instead of spaces inside of your rules.

filePathName = $(fileDirname)/$(fileBasenameNoExtension)
filePathNameExt = $(fileDirname)/$(fileBasename)
dirNameOut = $(workspaceRoot)/bin
filePathNameOut = $(dirNameOut)/$(fileBasenameNoExtension)

# project includes
utilDir = $(workspaceRoot)/example/cpp/_util
renderDir = $(workspaceRoot)/cpp/include/render
utilInc = -I./ -I$(utilDir)/include -I$(renderDir)

# GLM
glmDir = ${workspaceRoot}/../_cpp_extern/glm-0.9.9.8/glm
glmInc = -I$(glmDir)

# STB
stbDir = ${workspaceRoot}/../_cpp_extern/stb
stbInc = -I$(stbDir)

# GLEW
glewDir = $(workspaceRoot)/example/cpp/_glew
glewInc = -D GLEW_STATIC -I$(glewDir)/include
#glewLib = -L$(glewDir)/lib -lglew32
glewLib = -L$(glewDir)/lib -lglew64

# GLUT [http://freeglut.sourceforge.net/]
# Using freeglut or glut with MinGW [https://www.transmissionzero.co.uk/computing/using-glut-with-mingw/]
freeglutDir = ${workspaceRoot}/../_cpp_extern/freeglut/freeglut-3.0.0
#freeglutDir = ${workspaceRoot}/../_cpp_extern/freeglut/freeglut-MinGW-3.0.0-1.mp/freeglut
#freeglutInc = -D FREEGLUT_STATIC -I${freeglutDir}/include
glutInc = -I${freeglutDir}/include
#glutLib = -L${freeglutDir}/lib -lfreeglut_static -Wl,--subsystem,windows
#glutLib = -L${freeglutDir}/lib -lfreeglut -Wl,--subsystem,windows
glutLib = -L${freeglutDir}/lib/x64 -lfreeglut -Wl,--subsystem,windows

# GLFW [https://www.glfw.org/]
#glfwDir = ${workspaceRoot}/../_cpp_extern/glfw-3.2.1.bin.WIN32
glfwDir = ${workspaceRoot}/../_cpp_extern/glfw-3.3.6.bin.WIN64
glfwInc = -I${glfwDir}/include
#glfwLib = -L${glfwDir}/lib-mingw -lglfw3
glfwLib = -L${glfwDir}/lib-mingw-w64 -lglfw3

INCLUDEALL = $(utilInc) $(glmInc) $(stbInc) $(glewInc) $(glutInc) $(glfwInc)

LIBALL = $(glewLib) $(glutLib) $(glfwLib)

all: target	$(filePathNameOut).exe

$(filePathNameOut).exe: $(filePathNameOut).o
	g++ -g -std=c++17 -Og -static -o $@ $< $(LIBALL) -lgdi32 -lopengl32 -lglu32

$(filePathNameOut).o: $(filePathNameExt)
	g++ -g -std=c++17 -Og -c -o $@ $< $(INCLUDEALL)

clean:
	rm $(filePathNameOut).o $(filePathNameOut).exe	

target: ${dirNameOut}	${dirNameOut}/freeglut.dll	${dirNameOut}/glfw3.dll

${dirNameOut}:
	mkdir -p $@

#${dirNameOut}/freeglut.dll: ${freeglutDir}/bin/freeglut.dll
#	cp $< $@
${dirNameOut}/freeglut.dll: ${freeglutDir}/bin/x64/freeglut.dll
	cp $< $@

#${dirNameOut}/glfw3.dll: ${glfwDir}/lib-mingw/glfw3.dll
#	cp $< $@
${dirNameOut}/glfw3.dll: ${glfwDir}/lib-mingw-w64/glfw3.dll
	cp $< $@