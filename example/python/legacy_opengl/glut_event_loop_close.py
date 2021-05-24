# pyOpenGL GLUT window function doesn't close properly
# https://stackoverflow.com/questions/66486310/pyopengl-glut-window-function-doesnt-close-properly

from OpenGL.GLUT import *

class TestEnv:
    def __init__(self):
        self.window = None
        glutInit(sys.argv)
        glutInitWindowSize(100, 100)
        self.window = glutCreateWindow(b"TestEnv")

    def close(self):
        if self.window:
            glutDestroyWindow(self.window)

if __name__ == "__main__":
    i = 0
    while True:
        env = TestEnv()
        env.close()
        glutMainLoopEvent()
        print(i)
        i += 1
        if i == 20:
            break