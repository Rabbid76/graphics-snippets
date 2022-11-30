export interface MeshSpecification {
    vertices: Float32Array,
    normals?: Float32Array,
    uvs?: Float32Array,
    colors?: Float32Array,
    indices?: Uint32Array,
    error?: boolean,
    anyChanged?: boolean
}

export const float32Concat = (a1: Float32Array, a2: Float32Array): Float32Array => {
    const result = new Float32Array(a1.length + a2.length);
    result.set(a1);
    result.set(a2, a1.length);
    return result;
}

export const mergeIndices = (a1: Uint32Array, offset: number, a2: Uint32Array): Uint32Array => {
    const result = new Uint32Array(a1.length + a2.length);
    result.set(a1);
    for (let i = 0; i < a2.length; ++i) {
        result[a1.length + i] = offset + a2[i];
    }
    return result;
}


export const mergeBuffers = (m1: MeshSpecification, m2: MeshSpecification) : MeshSpecification => {
    const noVertices1 = m1.vertices.length / 3;
    const result: MeshSpecification = { vertices: float32Concat(m1.vertices, m2.vertices) };
    if (m1.normals && m2.normals) {
        result.normals = float32Concat(m1.normals, m2.normals);
    }
    if (m1.uvs && m2.uvs) {
        result.uvs = float32Concat(m1.uvs, m2.uvs);
    }
    if (m1.colors && m2.colors) {
        result.colors = float32Concat(m1.colors, m2.colors);
    }
    if (m1.indices && m2.indices) {
        result.indices = mergeIndices(m1.indices, noVertices1,  m2.indices);
    }
    return result;
}