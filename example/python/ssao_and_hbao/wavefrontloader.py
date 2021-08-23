import os, math
from PIL import Image
import numpy
from OpenGL.GL import *

def sub(a, b):
    return a[0]-b[0], a[1]-b[1], a[2]-b[2]
def cross(a, b):
    return a[1]*b[2]-a[2]*b[1], a[2]*b[0]-a[0]*b[2], a[0]*b[1]-a[1]*b[0]
def normalize(v):
    l2 = v[0]*v[0] + v[1]*v[1] + v[2]*v[2]
    if l2 == 0:
        return v
    l = math.sqrt(l2)
    return v[0]/l, v[1]/l, v[2]/l

class WaveFrontMaterialLoader:
    
    @classmethod
    def loadTexture(cls, imagefile, texture_unit = 0):
        image = Image.open(imagefile)
        glActiveTexture(GL_TEXTURE0 + texture_unit)
        texture_obj = glGenTextures(1)
        glBindTexture(GL_TEXTURE_2D, texture_obj)
        format = GL_RGBA if image.mode == 'RGBA' else GL_RGB
        glTexImage2D(GL_TEXTURE_2D, 0, format, *image.size, 0, format, GL_UNSIGNED_BYTE, image.tobytes())
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR)
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR)
        image.close()
        return texture_obj

    @classmethod
    def loadMaterial(cls, filename):
        contents = {}
        mtl = None
        dirname = os.path.dirname(filename)
        for line in open(filename, "r"):
            if line.startswith('#'): continue
            values = line.split()
            if not values: continue
            if values[0] == 'newmtl':
                mtl = contents[values[1]] = {}
            elif mtl is None:
                raise ValueError("mtl file doesn't start with newmtl stmt")
            elif values[0] == 'map_Kd':
                mtl[values[0]] = values[1]
                imagefile = os.path.join(dirname, mtl['map_Kd'])
                mtl['texture_Kd'] = cls.loadTexture(imagefile)
            else:
                mtl[values[0]] = list(map(float, values[1:]))
        return contents    

class WavefrontLoader:
    def __init__(self, filename, vertex_normals = True):
        value_list = self.__read_data_from_file(filename)
        self.__load_attributes(value_list)
        self.__load_material(value_list, os.path.dirname(filename))
        self.__load_faces(value_list)
        self.__calcualte_box()
        if vertex_normals:
            self.__create_vertex_normals()
        else:
            self.__create_face_normals()

    def __read_data_from_file(self, filename):
        content = None
        with open(filename, "r") as f:
            content = f.readlines()
        if not content:
            return None
        value_list = [line.split() for line in content if not line.startswith('#')]
        return [vl for vl in value_list if vl]

    def __load_attributes(self, value_list):
        self.vertices = [list(map(float, values[1:4])) for values in value_list if values[0] == 'v']
        self.normals = [list(map(float, values[1:4])) for values in value_list if values[0] == 'vn']
        self.texcoords = [list(map(float, values[1:3])) for values in value_list if values[0] == 'vt']

    def __load_material(self, value_list, dirname):
        try: self.material = value_list[value_list.index('usemtl')][1]
        except:
            try: self.material = value_list[value_list.index('usemat')][1]
            except: self.material = 0
        try: 
            mtlfile = os.path.join(dirname, value_list[value_list.index('mtllib')][1])
            self.mtl = WaveFrontMaterialLoader.loadMaterial(self, mtlfile) if os.path.isfile(mtlfile) else None
        except:
            self.mtl = None

    def __load_faces(self, value_list):
        self.faces = []
        for values in [vl for vl in value_list if vl[0] == 'f']:
            face, texcoords, norms = [], [], []
            for v in values[1:]:
                w = v.split('/')
                face.append(int(w[0]))
                if len(w) >= 2 and len(w[1]) > 0:
                    texcoords.append(int(w[1]))
                else:
                    texcoords.append(0)
                if len(w) >= 3 and len(w[2]) > 0:
                    norms.append(int(w[2]))
                else:
                    norms.append(0)
            self.faces.append([*face, *norms, *texcoords, self.material])
            
    def __calcualte_box(self):
        self.__box = [self.vertices[0][:], self.vertices[0][:]]
        for vi in range(1, len(self.vertices)):
            for i in range(3):
                self.__box[0][i] = min(self.__box[0][i], self.vertices[vi][i])
                self.__box[1][i] = max(self.__box[1][i], self.vertices[vi][i])

    def __create_face_normals(self):
        if not self.normals:
            self.normals = self.__create_face_normal_list()
            face_size = (len(self.faces[0]) - 1) // 3
            for fi, face in enumerate(self.faces):
                face[face_size:face_size*2] = [fi+1] * face_size

    def __create_vertex_normals(self):
        if not self.normals:
            face_normals = self.__create_face_normal_list()
            self.normals = [[0, 0, 0] for _ in range(len(self.vertices))]
            face_size = (len(self.faces[0]) - 1) // 3
            for fi, face in enumerate(self.faces):
                for vi in face[0:face_size]:
                    self.normals[vi-1][0] += face_normals[fi][0]
                    self.normals[vi-1][1] += face_normals[fi][1]
                    self.normals[vi-1][2] += face_normals[fi][2]
                face[face_size:face_size*2] = face[0:face_size]
            for ni in range(len(self.normals)):
                self.normals[ni] = normalize(self.normals[ni])

    def __create_face_normal_list(self):
        face_normals = []
        for face in self.faces:
            v0, v1, v2 = self.vertices[face[0]-1], self.vertices[face[1]-1], self.vertices[face[2]-1]
            nv = normalize(cross(sub(v1, v0), sub(v2, v0)))
            face_normals.append(nv)
        return face_normals

    @property
    def box(self):
        return self

    @property
    def size(self):
        return tuple(self.__box[1][i] - self.__box[0][i] for i in range(3))

    @property
    def center(self):
        return tuple((self.__box[0][i] + self.__box[1][i]) / 2 for i in range(3))


class WavefrontMesh:
    def __init__(self, filename):
        obj = WavefrontLoader(filename)
        self.mtl = obj.mtl
        self.__size = obj.size[:]
        self.__center = obj.center[:]
        self.generate(obj.vertices, obj.normals, obj.texcoords, obj.faces)

    @property
    def size(self):
        return self.__size

    @property
    def center(self):
        return self.__center

    def generate(self, vertices, normals, texcoords, faces):
        face_size = (len(faces[0]) - 1) // 3
        unique_keys = {}
        attributes = []
        indices = []
        for face_inidces in faces:
            for i in range(face_size):
                key = face_inidces[i], face_inidces[i + face_size], face_inidces[i + face_size*2]
                if key in unique_keys:
                    vi = unique_keys[key]
                else:
                    vi = len(attributes)
                    v = vertices[key[0]-1]
                    nv = normals[key[1]-1] if key[1] > 0 else [0, 0, 0]
                    tv = normals[key[2]-1] if key[2] > 0 else [0, 0]
                    attributes.append([*v, *nv, *tv, -1])
                indices.append(vi)
        self.attributes = numpy.array(attributes, dtype = numpy.float32) 
        self.indices = numpy.array(indices, dtype = numpy.uint32)
