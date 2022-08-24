import assimp_py
import glm

class AssimpModel:
    def __init__(self, filename):
        self.meshs = []

        # http://assimp.sourceforge.net/lib_html/postprocess_8h.html
        process_flags = (
            assimp_py.Process_Triangulate | 
            assimp_py.Process_CalcTangentSpace |
            #assimp_py.Process_GenNormals |
            assimp_py.Process_GenSmoothNormals |
            assimp_py.Process_GenUVCoords
        )
        scene = assimp_py.ImportFile(filename, process_flags)

        for mesh in scene.meshes:
            material = scene.materials[mesh.material_index]
            self.add_mesh(mesh, material)

        self.box_min = None
        self.box_max = None
        for mesh_specification in self.meshs:
            mb_min = mesh_specification.model_matrix * glm.vec4(*mesh_specification.box_min, 1)
            mb_max = mesh_specification.model_matrix * glm.vec4(*mesh_specification.box_max, 1) 
            if self.box_min:
                self.box_min = glm.vec3(
                    min(self.box_min[0], mb_min[0], mb_max[0]),
                    min(self.box_min[1], mb_min[1], mb_max[1]),
                    min(self.box_min[2], mb_min[2], mb_max[2]))
                self.box_max = glm.vec3(
                    max(self.box_max[0], mb_min[0], mb_max[0]),
                    max(self.box_max[1], mb_min[1], mb_max[1]),
                    max(self.box_max[2], mb_min[2], mb_max[2]))    
            else:
                self.box_min = glm.vec3(mb_min)
                self.box_max = glm.vec3(mb_max)

    def add_mesh(self, mesh_data, material_data):
        diffuse_color = material_data.get("COLOR_DIFFUSE")
        textures = material_data.get("TEXTURES")
        diffuse_tex = textures.get(assimp_py.TextureType_DIFFUSE) if textures else None

        vertices = mesh_data.vertices
        normals = [] or mesh_data.normals
        texcoords = [] or mesh_data.texcoords
        tangents = [] or mesh_data.tangents
        bitangent = [] or mesh_data.bitangents

        mesh = MeshSpecification()
        mesh.set_attributes(vertices, normals, texcoords)
        if diffuse_color:
            mesh.set_color(diffuse_color)

        self.meshs.append(mesh)

class MeshSpecification:
    def __init__(self):  
        self.box_min = [0, 0, 0]
        self.box_max = [0, 0, 0]
        self.model_matrix = glm.mat4(1)
        self.attributes = []
        self.indices = None
        self.format = 0, []
        self.color = (0, 0, 0)
        
    def set_attributes(self, vertices, normals, texcoords):
        self.box_min = [*vertices[0]]
        self.box_max = [*vertices[0]]
        uvs = texcoords if texcoords else None
        for i in range(len(vertices)):
            v = vertices[i]
            for j in range(3):
                self.box_min[j] = min(self.box_min[j], v[j])
                self.box_max[j] = max(self.box_max[j], v[j])
            self.attributes += [*v]
            if normals:
                nv = normals[i]
                self.attributes += [*nv]
            if uvs:
                uv = uvs[i]
                self.attributes += [*uv]
    
        format = [['vertex', 3]]
        if normals:
            format.append(['normal', 3])
        if uvs:
            format.append(['uv', 2])
        self.format = sum([info[1] for info in format]), format
        
        print(format)
        print(f'box: {self.box_min}, {self.box_max}')

    def set_color(self, color):
        self.color = color