from OpenGL.GL import *
from OpenGL.GLU import *
import sdl2

sdl2.SDL_Init(sdl2.SDL_INIT_EVERYTHING)

#sdl2.SDL_GL_SetAttribute(sdl2.SDL_GL_SHARE_WITH_CURRENT_CONTEXT, 1)
window = sdl2.SDL_CreateWindow(
        b"SDL2 OGL window", sdl2.SDL_WINDOWPOS_UNDEFINED, sdl2.SDL_WINDOWPOS_UNDEFINED, 400, 400, sdl2.SDL_WINDOW_OPENGL)

sdl2.SDL_GL_CreateContext(window)

event = sdl2.SDL_Event()
running = True
while running:
    while sdl2.SDL_PollEvent(ctypes.byref(event)) != 0:
        if event.type == sdl2.SDL_QUIT:
            running = False

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)
   
    glBegin(GL_QUADS)
    glColor3f(1, 0, 0)
    glVertex2f(-0.5, 0.5)
    glColor3f(1, 1, 0)
    glVertex2f(-0.5, -0.5)
    glColor3f(0, 1, 0)
    glVertex2f(0.5, -0.5)
    glColor3f(0, 0, 1)
    glVertex2f(0.5, 0.5)
    glEnd()

    sdl2.SDL_GL_SwapWindow(window)
    sdl2.SDL_Delay(10)