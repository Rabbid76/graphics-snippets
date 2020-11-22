# Creating a shared and static library with the gnu compiler (gcc) [https://renenyffenegger.ch/notes/development/languages/C-C-plus-plus/GCC/create-libraries/index]
# The extension of the library file is ".a". "make.exe" assumes that library files start with "lib"

all : ./lib/libglew32.a

./lib/libglew32.a : ./lib/glew32.o
	ar rcs $@ $<
	rm $<

./lib/glew32.o : ./src/glew.c
	g++ -c -o $@ $< -D GLEW_STATIC -D WIN32 -I./include

clean:
	rm ./lib/glew32.o ./lib/libglew32.a


