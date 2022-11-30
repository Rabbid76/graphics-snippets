import { MeshSpecification } from './geometryBuffer'

export const createCircle = (radius: number, height: number, segments: number, reverse?: boolean): MeshSpecification => {
    const d: number = reverse ? -1 : 1;
    const v: number[] = [0, 0, height];
    const n: number[] = [0, 0, d];
    const t: number[] = [0.5, 0.5];
    let i: number[] = [];
    for (let is = 0; is <= segments; ++is) {
        const a = (reverse ? segments - is : is) / segments * Math.PI * 2;
        const c = Math.cos(a);
        const s = Math.sin(a);
        v.push(c * radius, s * radius, height);
        n.push(0, 0, d);
        t.push(c * 0.5 + 0.5, s * 0.5 + 0.5);
    }
    for (let is = 0; is < segments; ++is) {
        i.push(0, is+1, is+2);
    }
    return {
        vertices: new Float32Array(v),
        normals: new Float32Array(n),
        uvs: new Float32Array(t),
        indices: new Uint32Array(i),
    }
}

export const createTube = (radii: number[], heights: number[], segments: number): MeshSpecification => {
    const v: number[] = [];
    const n: number[] = [];
    const t: number[] = [];
    let i: number[] = [];
    const height = heights.reduce((sum, h) => sum + h, 0);
    let h = 0;
    for (let ih = 0; ih < radii.length; ++ih) {
        for (let is = 0; is <= segments; ++is) {
            const a = is / segments * Math.PI * 2;
            const c = Math.cos(a);
            const s = Math.sin(a);
            v.push(c * radii[ih], s * radii[ih], h);
            n.push(c, s, 0);
            t.push(is / segments, h / height);
        }
        h += heights[ih];
    }
    for (let ih = 0; ih < heights.length; ++ih) {
        for (let is = 0; is < segments; ++is) {
            const i0 = ih * (segments + 1) + is;
            const i1 = i0 + 1;
            const i2 = i1 + segments + 1;
            const i3 = i2 - 1;
            i.push(i0, i1, i2, i0, i2, i3);
        }
    }
    return {
        vertices: new Float32Array(v),
        normals: new Float32Array(n),
        uvs: new Float32Array(t),
        indices: new Uint32Array(i),
    }
} 