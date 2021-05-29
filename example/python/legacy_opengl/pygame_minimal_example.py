from OpenGL.GL import *
from OpenGL.GLU import *
import pygame

pygame.init()
pygame.display.set_mode((400, 400), pygame.DOUBLEBUF | pygame.OPENGL)

run = True
while run:
    for event in pygame.event.get():
        if event.type == pygame.QUIT:
            run = False          

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
   
    pygame.display.flip()

pygame.quit()
exit()
