from window.glfw_window import GlfwWindow
from model_scene import *
import utility.assimp_loader
import os

resource_path = os.path.join(os.path.dirname(os.path.abspath(__file__)), '../../../resource')
resources = [
    ('model/wavefront/monkey.obj', ModelOrientation.Y_UP),
    ('model/wavefront/buddha.obj', ModelOrientation.Y_UP),
    ('model/wavefront/dragon.obj', ModelOrientation.Y_UP),
    ('model/wavefront/bunny.obj', ModelOrientation.Y_UP),
    ('model/wavefront/venusv.obj', ModelOrientation.Y_UP),
    ('model/wavefront/ateneav.obj', ModelOrientation.Y_UP),
    ('model/wavefront/elephav.obj', ModelOrientation.Y_UP),
    ('model/wavefront/pig_triangulated.obj', ModelOrientation.Y_UP),
    ('model/wavefront/sphere.obj', ModelOrientation.Y_UP),
    ('model/wavefront/cube_simple.obj', ModelOrientation.Y_UP),
]

if __name__ == '__main__':
    reources_index = 0
    # todo pathlib
    model_filename = os.path.join(resource_path, resources[reources_index][0])
    model_specification = utility.assimp_loader.AssimpModel(model_filename)   
    scene = ModelScene(model_specification, resources[reources_index][1])
    max_size = max(*scene.size)
    look_z = scene.size.z/2
    window = GlfwWindow(800, 600, "OpenGL turntable", scene, max_size, look_z, "c:/temp/turntable_")
    window.run()
