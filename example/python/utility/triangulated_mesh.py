import math

def dot(a, b):
    return sum(t[0]*t[1] for t in zip(a, b)) 

def cross( a, b ):
    return [a[1] * b[2] - a[2] * b[1], a[2] * b[0] - a[0] * b[2], a[0] * b[1] - a[1] * b[0]]

def normalize(n):
    l_sq = dot(n, n)
    if l_sq == 0:
        return n
    l = math.sqrt(l_sq)
    return [val/l for val in n]

def mul_s(v, s):
    return [val*s for val in v]

def add(a, b):
    return [t[0]+t[1] for t in zip(a, b)] 

def sub(a, b):
    return [t[0]-t[1] for t in zip(a, b)] 

class TriangulatedMeshBase:
    __format = [['vertex', 3], ['normal', 3], ['uvw', 3]]
    __stride = sum([info[1] for info in __format])
    def __init__(self):
        self._attributes = []
        self._indices = []
    @property
    def attributes(self):
        return self._attributes
    @property
    def indices(self):
        return self._indices
    @property
    def format(self):
        return TriangulatedMeshBase.__stride, TriangulatedMeshBase.__format

class Quad(TriangulatedMeshBase):
    def __init__(self, l=1):
        super().__init__()
        v = [-l/2,-l/2,0, l/2,-l/2,0, l/2,l/2,0, -l/2,l/2,0]
        t = [0,1,0, 1,1,0, 1,0,0, 0,0,0]
        self._attributes = []
        for i in range(4):
            self._attributes += v[i*3:i*3+3] + [0, 0, 1] + t[i*3:i*3+3] 
        self._indices = [0,1,2, 0,2,3]

class Tetrahedron(TriangulatedMeshBase):
    def __init__(self, l = 1):
        super().__init__()
        s_8_9, s_2_9, s_2_3 = math.sqrt(8/9), math.sqrt(2/9), math.sqrt(2/3)
        v = [0,0,1, s_8_9,0,-1/3, -s_2_9,s_2_3,-1/3, -s_2_9,-s_2_3,-1/3]
        t = [0,0, 0,1, 1,1, 1,0]
        ni = [3, 1, 2, 0]
        e = [0,1,2, 0,2,3, 0,3,1, 1,3,2]
        self._attributes = []
        for si in range(4):
            for pi in range (3):
                i = si*3 + pi
                self._attributes += [
                    l*v[e[i]*3], l*v[e[i]*3+1], l*v[e[i]*3+2],
                    -v[ni[si]*3], -v[ni[si]*3+1], -v[ni[si]*3+2],
                    t[e[i]*2], t[e[i]*2+1], si/4]
        self._indices = range(len(self._attributes) // 9)

class Cube(TriangulatedMeshBase):
    def __init__(self, l = 1/math.sqrt(2)):
        super().__init__()
        v = [[-1,-1,1], [1,-1,1], [1,1,1], [-1,1,1], [-1,-1,-1], [1,-1,-1], [1,1,-1], [-1,1,-1]]
        t = [[0, 1], [1, 1], [1, 0], [0, 0]]
        n = [[0,0,1], [1,0,0], [0,0,-1], [-1,0,0], [0,1,0], [0,-1,0]]
        e = [[0,1,2,3], [1,5,6,2], [5,4,7,6], [4,0,3,7], [3,2,6,7], [1,0,4,5]]
        self._indices = [si*4+[0, 1, 2, 0, 2, 3][vi] for si in range(6) for vi in range(6)]
        self._attributes = []
        for si in range(len(e)):
            for qi, vi in enumerate(e[si]):
                self._attributes += [v[vi][0]*l, v[vi][1]*l,v[vi][2]*l, *n[si], *t[qi], si/6]

class Octahedron(TriangulatedMeshBase):
    def __init__(self, l = 1):
        super().__init__()
        v = [-1,0,0, 0,-1,0, 0,0,-1, 1,0,0, 0,1,0, 0,0,1]
        t = [0,0, 1,0, 0.5,0.5, 1,1, 0,1, 0.5, 0.5]
        e = [0,2,1, 1,2,3, 3,2,4, 4,2,0, 0,1,5, 1,3,5, 3,4,5, 4,0,5]
        nf = [-1,-1,-1, 1,-1,-1, 1,1,-1, -1,1,-1, -1,-1,1, 1,-1,1, 1,1,1, -1,1,1]
        self._attributes = []
        for si in range(8):
            nv = normalize([nf[si*3], nf[si*3+1], nf[si*3+2]])
            for pi in range (3):
                i = si*3 + pi
                self._attributes += [
                    l*v[e[i]*3], l*v[e[i]*3+1], l*v[e[i]*3+2],
                    nv[0], nv[1], nv[2],
                    t[e[i]*2], t[e[i]*2+1], si/8]
        self._indices = range(len(self._attributes) // 9)


class Dodecahedron(TriangulatedMeshBase):
    def __init__(self, l = 1):
        super().__init__()
        phi = (1 + math.sqrt(5)) / 2 # φ=(1+√5)/2 is the Golden Ratio.
        phi2 = phi*phi
        a, b, c = 1, 1/phi, 1/(phi*phi)
        v = [-b,-b,-b, b,-b,-b, b,b,-b, -b,b,-b, # 0..7: (±φ, ±φ, ±φ)
             -b,-b,b, b,-b,b, b,b,b, -b,b,b,
             0,-a,-c, 0,a,-c, 0,a,c, 0,-a,c,     # 8..11: (0, ±φ^2, ±1)
             -a,-c,0, a,-c,0, a,c,0, -a,c,0,     # 12..15: (±φ^2, ±1, 0)
             -c,0,-a, c,0,-a, c,0,a, -c,0,a,     # 16..19: (±1, 0, ±φ^2)
        ]
        t = [0,0, 1,0, 1,0.5, 0.5,1, 0,0.5]
        e = [16,17,1,8,0, 17,16,3,9,2, 19,18,6,10,7, 18,19,4,11,5,
             14,13,1,17,2, 13,14,6,18,5, 15,12,4,19,7, 12,15,3,16,0,
             9,10,6,14,2, 10,9,3,15,7, 8,11,4,12,0, 11,8,1,13,5]
        self._attributes = []
        for si in range(len(e) // 5):
            nv = [0, 0, 0]
            for pi in range(5):
                nv[0] += v[e[si*5+pi]*3]
                nv[1] += v[e[si*5+pi]*3+1]
                nv[2] += v[e[si*5+pi]*3+2]
            cpt = [nv[0]/5, nv[1]/5, nv[2]/5]
            nv = normalize(nv)
            w = si/12
            self._attributes += [l*cpt[0], l*cpt[1], l*cpt[2], nv[0], nv[1], nv[2], 0.5, 0.5, w]
            for pi in range(5):
                i = si*5+pi
                self._attributes += [l*v[e[i]*3], l*v[e[i]*3+1], l*v[e[i]*3+2], nv[0], nv[1], nv[2], t[pi*2], t[pi*2+1], w]
            e0 = si*6
            self._indices += [e0,e0+1,e0+2, e0,e0+2,e0+3, e0,e0+3,e0+4, e0,e0+4,e0+5, e0,e0+5,e0+1]    

class Icosahedron(TriangulatedMeshBase):
    def __init__(self, l = 1):
        v = [0,0,1, 0.894,0,0.447, 0.276,0.851,0.447, -0.724,0.526,0.447, -0.724,-0.526,0.447, 0.276,-0.851, 0.447,
             0.724,0.526,-0.447, -0.276,0.851,-0.447, -0.894,0,-0.447, -0.276,-0.851,-0.447, 0.724,-0.526,-0.447, 0,0,-1 ]
        e = [1,2,0, 2,3,0, 3,4,0, 4,5,0, 5,1,0, 7,6,11, 8,7,11, 9,8,11, 10,9,11, 6,10,11,
             1,6,2, 2,7,3, 3,8,4, 4,9,5, 5,10,1, 7,2,6, 8,3,7, 9,4,8, 10,5,9,  6,1,10 ]
        t = [0,0, 1,0, 1,1, 0,1]
        n = []
        for fi in range(len(e) // 3):
            nv = [0, 0, 0]
            for ti in range(3):
                nv[0] += v[e[fi*3+ti]*3]
                nv[1] += v[e[fi*3+ti]*3+1]
                nv[2] += v[e[fi*3+ti]*3+2]
            n += normalize(nv)
        self._attributes = []
        for i in range(len(e)):
            ti, fi = i % 3, i // 3
            shift = 0 if (math.floor(fi) >= 10 or ti == 0) else 2
            self._attributes += [
                l*v[e[i]*3], l*v[e[i]*3+1], l*v[e[i]*3+2],
                n[fi*3], n[fi*3+1], n[fi*3+2],
                t[ti*2+shift], t[ti*2+1+shift], fi*3/len(e)]
        self._indices = range(len(self._attributes) // 9)

class Tube(TriangulatedMeshBase):
    def __init__(self, nc=32, nl=1, h=2, r=2/math.pi):
        self._attributes = []
        self._indices = []
        # top cap
        self._attributes += [0, 0, h/2, 0, 0, 1, 0.5, 0.5, 1]
        for i in range(nc+1):
            a = 2 * math.pi * i / nc
            x, y = math.cos(a), math.sin(a)
            uvs = 1/math.fabs(x if math.fabs(x) > math.fabs(y) else y)
            u = uvs*x*0.5+0.5
            v = uvs*y*0.5+0.5
            self._attributes += [r*x, r*y, h/2, 0, 0, 1, u, v, 1]
            if i < nc:
                self._indices += [0, i+1, i+2]
        # discs
        st = len(self._attributes) // 9
        nc_2 = nc // 2
        ratio = 2*h / (r*2*math.pi)
        for tbI in range(nl+1):
            v = (1.0 - tbI / nl) * ratio
            z = h/2 - h * tbI / nl
            # create 2 hemispheres, so would be possible to wrap 1 texture at both hemispheres 
            for hi in range(2):
                for i in range(nc_2+1):
                    u = i / nc_2
                    a = math.pi * u + hi * math.pi
                    x, y = math.cos(a), math.sin(a)
                    self._attributes += [r*x, r*y, z, x, y, 0, u, v, 1-tbI/nl]
        cs_2 = nc_2 + 1
        for tbI in range(nl):
            for hi in range(2):
                rsi = st +tbI*2*cs_2 + hi*cs_2
                nrsi = rsi + 2*cs_2
                for i in range(nc_2):
                    self._indices += [rsi+i, nrsi+i, nrsi+i+1, rsi+i, nrsi+i+1, rsi+i+1]
        # bottom cap
        st = len(self._attributes) // 9
        for i in range(nc+1):
            a = 2 * math.pi * i / nc
            x, y = math.cos(a), math.sin(a)
            uvs = 1/math.fabs(x if math.fabs(x) > math.fabs(y) else y)
            u = uvs*x*0.5+0.5
            v = uvs*y*0.5+0.5
            self._attributes += [r*x, r*y, -h/2, 0, 0, -1, u, v, 0]
            if i < nc:
                self._indices += [st+nc+1, st+i+1, st+i]
        self._attributes += [0, 0, -h/2, 0, 0, -1, 0.5, 0.5, 0] 

class Cone(TriangulatedMeshBase):
    def __init__(self, nc=32, nl=4, h=2, r=2/math.pi):
        self._attributes = []
        self._indices = []
        # discs
        st = 0
        nc_2 = nc // 2
        ratio = 2*h / (r*2*math.pi)
        for tbI in range(nl+1):
            v = (1.0 - tbI / nl) * ratio * 3/2
            z = h*2/3 - h * tbI / nl
            d = tbI / nl
            # create 2 hemispheres, so would be possible to wrap 1 texture at both hemispheres 
            for hi in range(2):
                for i in range(nc_2+1):
                    u = i / nc_2
                    a = math.pi * u + hi * math.pi
                    x, y = math.cos(a), math.sin(a)
                    n = normalize([x, y, r/h])
                    self._attributes += [d * r*x, d* r*y, z, n[0], n[1], n[2], u, v, 1-tbI/nl]
        cs_2 = nc_2 + 1
        for tbI in range(nl):
            for hi in range(2):
                rsi = st +tbI*2*cs_2 + hi*cs_2
                nrsi = rsi + 2*cs_2
                for i in range(nc_2):
                    self._indices += [rsi+i, nrsi+i, nrsi+i+1]
                    if tbI > 0: 
                        self._indices += [rsi+i, nrsi+i+1, rsi+i+1]
        # bottom cap
        st = len(self._attributes) // 9
        for i in range(nc+1):
            a = 2 * math.pi * i / nc
            x, y = math.cos(a), math.sin(a)
            uvs = 1/math.fabs(x if math.fabs(x) > math.fabs(y) else y)
            u = uvs*x*0.5+0.5
            v = uvs*y*0.5+0.5
            self._attributes += [r*x, r*y, -h/3, 0, 0, -1, u, v, 0] 
            if i < nc:
                self._indices += [st+nc+1, st+i+1, st+i]
        self._attributes += [0, 0, -h/3, 0, 0, -1, 0.5, 0.5, 0] 

class SphereSlice(TriangulatedMeshBase):
    def __init__(self, nc=32, nl=16, r=1):
        super().__init__()
        self._attributes = []
        self._indices = []
        nc_2 = nc // 2
        for tbI in range(nl+1):
            v = 1.0 - tbI / nl
            z = math.sin((1.0 - 2.0 * tbI / nl ) * math.pi/2.0)
            cU = math.sqrt(1.0 - z*z)
            # create 2 hemispheres, so would be possible to wrap 1 texture at both hemispheres
            for hi in range(2): 
                for i in range(nc_2+1): 
                    u = i / nc_2
                    a = math.pi * u + hi * math.pi
                    x, y = math.cos(a) * cU, math.sin(a) * cU
                    self._attributes += [r*x, r*y, r*z, x, y, z, hi*0.5+u*0.5, v, 1.0 - tbI / nl]
        # bottom cap
        cs_2 = nc_2 + 1
        cs = cs_2 * 2
        for i in range(nc_2):
            self._indices += [cs + i, cs + i + 1, i]
        for i in range(nc_2+1, 2*nc_2+1):
            self._indices += [cs+i, cs+i+1, i]
        # discs
        for tbI in range(1, nl-1):
            for hi in range(2): 
                rsi = tbI*cs + hi*cs_2
                nrsi = (tbI+1)*cs + hi*cs_2
                for i in range(nc_2):
                     self._indices += [rsi+i, nrsi+i, nrsi+i+1, rsi+i, nrsi+i+1, rsi+i+1]
        # top cap
        si = (nl-1) * cs
        for i in range(nc_2):
            self._indices += [si + i + 1, si + i, si + i + cs]
        for i in range(nc_2+1, 2*nc_2+1):
            self._indices += [si+i+1, si+i, si+i+cs]

class SphereTessellated(TriangulatedMeshBase):
    def __init__(self, tc=5, r=1):
        super().__init__()
        s_8_9, s_2_9, s_2_3 = math.sqrt(8/9), math.sqrt(2/9), math.sqrt(2/3)
        v = [0,0,1, s_8_9,0,-1/3, -s_2_9,s_2_3,-1/3, -s_2_9,-s_2_3,-1/3]
        t = [0,0, 0,1, 1,1, 1,0]
        e = [0,1,2, 0,2,3, 0,3,1, 1,3,2]
        self.v = []
        self.n = []
        self.t = []
        self._attributes = []
        for si in range(4):
            for pi in range(3):
                i = si*3 + pi
                self.v += [r*v[e[i]*3], r*v[e[i]*3+1], r*v[e[i]*3+2]]
                self.n += [v[e[i]*3], v[e[i]*3+1], v[e[i]*3+2]]
                self.t += [t[e[i]*2], t[e[i]*2+1], si/4]
        self.r = r
        self.e = range(len(self.v) // 3)
        for i in range(tc):
            self.split_triangle()
        self._attributes = []
        for i in range(0, len(self.v), 3):
            self._attributes += self.v[i:i+3] + self.n[i:i+3] + self.t[i:i+3]
        self._indices = self.e
    def split_line(self, i0, i1):
        self.v += mul_s(normalize([(self.v[i0*3]+self.v[i1*3])/2, (self.v[i0*3+1]+self.v[i1*3+1])/2, (self.v[i0*3+2]+self.v[i1*3+2])/2]), self.r)
        self.n += normalize([(self.n[i0*3]+self.n[i1*3])/2, (self.n[i0*3+1]+self.n[i1*3+1])/2, (self.n[i0*3+2]+self.n[i1*3+2])/2])
        self.t += [(self.t[i0*3]+self.t[i1*3])/2, (self.t[i0*3+1]+self.t[i1*3+1])/2, (self.t[i0*3+2]+self.t[i1*3+2])/2]
    def split_triangle(self):
        faces = self.e
        noOfIndices = len(faces)   
        self.e = []
        for faceStart in range(0, noOfIndices, 3):
            f = [faces[faceStart], faces[faceStart+1], faces[faceStart+2]]
            next = len(self.v) // 3
            nf = [next, next+1, next+2]
            self.split_line(f[0], f[1])
            self.split_line(f[1], f[2])
            self.split_line(f[2], f[0])
            self.e += [f[0], nf[0], nf[2], f[1], nf[1], nf[0], f[2], nf[2], nf[1], nf[0], nf[1], nf[2]]

class Torus(TriangulatedMeshBase):
    def __init__(self, nc=32, nl=32, r =0.7, rt = 0.3):
        super().__init__()
        self._attributes = []
        self._indices = []
        for i_l in range(nl+1):
            tcpt = [math.cos(2 * math.pi * i_l / nl), math.sin(2 * math.pi * i_l / nl)]
            for i_c in range(nc+1):
                tX, tY = math.cos(2 * math.pi * i_c / nc), math.sin(2 * math.pi * i_c / nc)
                p = [tcpt[0] * (r + tX*rt), tcpt[1] * (r + tX*rt), tY*rt ]
                n = [p[0] - tcpt[0]*r, p[1] - tcpt[1]*r, p[2]]
                self._attributes += p + n + [3*i_l/nl, i_c/nc, i_l/nl]
                i_l0, i_l1 = i_l * (nc+1), (i_l+1) * (nc+1)
                if i_c < nc and i_l < nl:
                    self._indices += [i_l0+i_c, i_l1+i_c, i_l0+(i_c+1), i_l0+(i_c+1), i_l1+i_c, i_l1+(i_c+1)]

class TrefoilKnot(TriangulatedMeshBase):
    def __init__(self, stacks=32, slices=256, ra=0.6, rb=0.2,rc=0.4, rd=0.175):
        super().__init__()
        self._attributes = []
        self._indices = []
        self.ra, self.rb, self.rc, self.rd = ra, rb, rc, rd
        self.v, self.n, self.t = [], [], [] 
        ds, dt = 1.0 / slices, 1.0 / stacks
        E = 0.01
        s = 0
        while s < 1 + ds/2: 
            t = 0
            while t < 1 + dt/2: 
                p = self.compute(s, t)
                u = self.compute(s + E, t)
                u = sub(u, p)
                v = self.compute(s, t + E)
                v = sub(v, p)
                n = normalize(cross(u, v))
                self._attributes += p + n + [s * 18.0, t, s]
                t += dt
            s += ds
        self.e = []
        vertexCount = len(self._attributes) // 9
        n = 0
        for i in range(slices):
            for j in range(stacks): 
                self._indices += [n + j, (n + j + stacks + 1) % vertexCount, n + j + 1]
                self._indices += [(n + j + stacks + 1) % vertexCount, (n + j + 1 + stacks + 1) % vertexCount, (n + j + 1) % vertexCount]
            n += stacks+1
    def compute(self, s, t):
        TwoPi = math.pi * 2
        a, b, c, d = self.ra, self.rb, self.rc, self.rd
        u = ( 1 - s ) * 2 * TwoPi
        v = t * TwoPi
        r = a + b * math.cos(1.5 * u)
        x = r * math.cos(u)
        y = r * math.sin(u)
        z = c * math.sin(1.5 * u)
        dv = [
            -1.5 * b * math.sin(1.5 * u) * math.cos(u) - (a + b * math.cos(1.5 * u)) * math.sin(u),
            -1.5 * b * math.sin(1.5 * u) * math.sin(u) + (a + b * math.cos(1.5 * u)) * math.cos(u),
            1.5 * c * math.cos(1.5 * u)]
        q = normalize(dv)
        qvn = normalize([q[1], -q[0], 0])
        ww = cross(q, qvn)
        range = [
            x + d * (qvn[0] * math.cos(v) + ww[0] * math.sin(v)),
            y + d * (qvn[1] * math.cos(v) + ww[1] * math.sin(v)),
            z + d * ww[2] * math.sin(v)]    
        return range

class TorusKnot(TriangulatedMeshBase):
    def __init__(self, tess_v=32, tess_u=512, p_=7, q_=3, ra=0.7, rb=0.3, rc=0.1):
        super().__init__()
        self._attributes = []
        self._indices = []
        for u in range(tess_u+1): 
            phi0 = 2 * math.pi * u / tess_u
            pt_tk0 = self.sample(phi0, p_, q_, ra, rb)
            phi1 = 2 * math.pi * (u + 1) / tess_u
            pt_tk1 = self.sample(phi1, p_, q_, ra, rb)
            pt_c = self.sample(phi0, p_, q_, ra, 0)   
            T = sub(pt_tk1, pt_tk0)
            B = normalize(sub(pt_tk0, pt_c))
            N = normalize(cross(T, B))
            T = normalize(cross(B, N))
            for v in range(tess_v+1): 
                theta = 2 * math.pi * (v / tess_v)
                px = math.sin(theta) * rc 
                py = math.cos(theta) * rc
                n = add(mul_s(N, px), mul_s(B, py))
                p = add(n, pt_tk0)
                n = normalize(n)
                self._attributes += p + n + [3.0 * p_ * q_ * u/tess_u, v/tess_v, u/tess_u]
        for i_strip in range(tess_u):
            for i_v in range(tess_v):
                ei = [i_strip*(tess_v+1)+i_v, i_strip*(tess_v+1)+i_v+1, (i_strip+1)*(tess_v+1)+i_v+1, (i_strip+1)*(tess_v+1)+i_v]
                self._indices += [ei[0], ei[1], ei[2], ei[0], ei[2], ei[3]]
    def sample(self, phi, p, q, r1, r2):
        return [
            (r1 + r2 * math.cos(p * phi)) * math.cos(q * phi),
            (r1 + r2 * math.cos(p * phi)) * math.sin(q * phi), 
            r2 * -math.sin(p * phi)]

class Arrow(TriangulatedMeshBase):
    def __init__(self, nc=32, nls=8, nlp=2, ls=0.75, lp=0.25, rs=0.25/math.pi*0.75, rp=0.25/math.pi*1.5):
        super().__init__()
        self._attributes = []
        self._indices = []
        # cone discs
        st = 0
        nc_2 = nc // 2
        p_ratio = 2*lp / (rp*2*math.pi)
        for tbI in range(nlp+1):
            v = (1.0 - tbI / nlp) * p_ratio * 3/2
            z = -lp * tbI / nlp
            d = tbI / nlp
            # create 2 hemispheres, so would be possible to wrap 1 texture at both hemispheres 
            for hi in range(2):
                for i in range(nc_2+1):
                    u = i / nc_2
                    a = math.pi * u + hi * math.pi
                    x = math.cos(a)
                    y = math.sin(a)
                    n = normalize([x, y, rp/lp])
                    self._attributes += [d * rp*x, d* rp*y, z, *n, u, v, 1 - tbI/nlp * 0.5]
        cs_2 = nc_2 + 1
        for tbI in range(nlp):
            for hi in range(2):
                rpi = st +tbI*2*cs_2 + hi*cs_2
                nrpi = rpi + 2*cs_2
                for i in range(nc_2):
                    self._indices += [rpi+i, nrpi+i, nrpi+i+1]
                    if tbI > 0: 
                        self._indices += [rpi+i, nrpi+i+1, rpi+i+1]            
        # cone bottom cap
        st = len(self._attributes) // 9
        for i in range(nc+1):
            a = 2 * math.pi * i / nc
            x = math.cos(a)
            y = math.sin(a)
            uvs = 1/math.fabs(x if math.fabs(x) > math.fabs(y) else y)
            u_ = uvs*x*0.5+0.5
            v_ = uvs*y*0.5+0.5
            self._attributes += [rp*x, rp*y, -lp, 0, 0, -1, u_, v_, 0.5]
            if i < nc:
                self._indices += [st+nc+1, st+i+1, st+i]
        self._attributes += [0, 0, -lp, 0, 0, -1, 0.5, 0.5, 0.5]
        # shaft discs
        st = len(self._attributes) // 9
        s_ratio = 2*ls / (rs*2*math.pi)
        for tbI in range(nls+1):
            v = (1 - tbI / nls) * s_ratio
            z =   -lp - ls * tbI / nls
            # create 2 hemispheres, so would be possible to wrap 1 texture at both hemispheres 
            for hi in range(2):
                for i in range(nc_2+1):
                    u = i / nc_2
                    a = math.pi * u + hi * math.pi
                    x = math.cos(a)
                    y = math.sin(a)
                    self._attributes += [rs*x, rs*y, z, x, y, 0, u, v, (1-tbI/nls) * 0.5]
        for tbI in range(nls):
            for hi in range(2):
                rsi = st +tbI*2*cs_2 + hi*cs_2
                nrsi = rsi + 2*cs_2
                for i in range(nc_2):
                    self._indices += [rsi+i, nrsi+i, nrsi+i+1, rsi+i, nrsi+i+1, rsi+i+1]
        # bottom cap
        st = len(self._attributes) // 9
        for i in range(nc+1):
            a = 2 * math.pi * i / nc
            x = math.cos(a)
            y = math.sin(a)
            uvs = 1/math.fabs(x if math.fabs(x) > math.fabs(y) else y)
            u_ = uvs*x*0.5+0.5
            v_ = uvs*y*0.5+0.5
            self._attributes += [rs*x, rs*y, -lp-ls, 0, 0, -1, u_, v_, 0]
            if i < nc:
                self._indices += [st+nc+1, st+i+1, st+i]
        self._attributes += [0, 0, -lp-ls, 0, 0, -1, 0.5, 0.5, 0]
