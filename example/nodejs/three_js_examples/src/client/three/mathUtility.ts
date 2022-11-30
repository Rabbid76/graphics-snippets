import { MeshSpecification } from './../geometry/geometryBuffer'
import * as THREE from 'three';
import { Box3 } from 'three';

export interface Ray {
    origin: THREE.Vector3;
    direction: THREE.Vector3;
}

export interface Plane {
    pointOnPlane: THREE.Vector3;
    normal: THREE.Vector3;
}

export interface Intersection {
    distance: number;
    point: THREE.Vector3;
}

interface TrianglePlanIntersection {
    index: number,
    intersectToPeak: Intersection,
    intersectFromPeak: Intersection
}

interface TriangleTriangleIntersection {
    intersectionTriangle1: TrianglePlanIntersection,
    intersectionTriangle2: TrianglePlanIntersection,
}

export const fMod = (a:number, b: number): number => {
    return Math.round(a / b) * b;
};

export const barycentricCoordinate = (ab: THREE.Vector3, ac: THREE.Vector3, ax: THREE.Vector3): THREE.Vector3 => {
    const d00 = ab.dot(ab);
    const d01 = ab.dot(ac);
    const d11 = ac.dot(ac);
    const d20 = ax.dot(ab);
    const d21 = ax.dot(ac);
    const denom = d00 * d11 - d01 * d01; // determinant
    const bx = (d11 * d20 - d01 * d21) / denom;
    const by = (d00 * d21 - d01 * d20) / denom;
    const bz = 1 - bx - by;
    return new THREE.Vector3(bx, by, bz);
}

export const interpolateBarycentric = (b: THREE.Vector3, t: THREE.Vector3[]): THREE.Vector3 => {
    const v = new THREE.Vector3(0, 0, 0);
    v.add(t[0].clone().multiplyScalar(b.x)).add(t[1].clone().multiplyScalar(b.y)).add(t[2].clone().multiplyScalar(b.z));
    return v;
}

export const planeFromTriangle = (p0: THREE.Vector3, p1: THREE.Vector3, p2: THREE.Vector3): Plane => {
    return { 
        pointOnPlane: p0.clone(), 
        normal: p1.clone().sub(p0).cross(p2.clone().sub(p0))
    }
}

export const intersectRayPlane = (ray: Ray, plane: Plane): Intersection | null => {
    if (ray.direction.dot(plane.normal) === 0) {
        return null;
    }
    const t = plane.pointOnPlane.clone().sub(ray.origin).dot(plane.normal) / ray.direction.clone().dot(plane.normal);
    const X = ray.origin.clone().add(ray.direction.clone().multiplyScalar(t));
    return { distance: t, point : X };
}

export const intersectRayPlaneByThreePoints = (ray: Ray, p0: THREE.Vector3, p1: THREE.Vector3, p2: THREE.Vector3): Intersection | null => {
    return intersectRayPlane(ray, planeFromTriangle(p0, p1, p2));
}

export const transformRay = (ray: Ray, m: THREE.Matrix4) => {
    return {
        origin: ray.origin.applyMatrix4(m), 
        direction: ray.direction.applyMatrix3(new THREE.Matrix3().setFromMatrix4(m))
    }
}

// https://stackoverflow.com/questions/46749675/opengl-mouse-coordinates-to-space-coordinates/46752492#46752492
export const createViewSpaceRay = (camera: THREE.Camera, ndc2dCoordinate: THREE.Vector2): Ray => {
    if (camera instanceof THREE.OrthographicCamera) {
        const orthographicCamera = camera as THREE.OrthographicCamera;
        const pointOnNearPlane = new THREE.Vector3(ndc2dCoordinate.x, ndc2dCoordinate.y, -1).applyMatrix4(orthographicCamera.projectionMatrixInverse);
        const pointOnFarPlane = new THREE.Vector3(ndc2dCoordinate.x, ndc2dCoordinate.y, 1).applyMatrix4(orthographicCamera.projectionMatrixInverse);
        const direction = pointOnFarPlane.sub(pointOnNearPlane);
        return { origin: pointOnNearPlane, direction };
    } 
    const perspectiveCamera = camera as THREE.PerspectiveCamera;
    const origin = new THREE.Vector3(0, 0, 0);
    const pointOnRay = new THREE.Vector3(ndc2dCoordinate.x, ndc2dCoordinate.y, 0).applyMatrix4(perspectiveCamera.projectionMatrixInverse);
    const direction = pointOnRay.sub(origin);
    return { origin, direction };
}

export const createWorldSpaceRay = (camera: THREE.Camera, ndc2dCoordinate: THREE.Vector2): Ray => {
    const viewSpaceRay = createViewSpaceRay(camera, ndc2dCoordinate);
    return transformRay(viewSpaceRay, camera.matrixWorld);
}

// https://www.scratchapixel.com/lessons/3d-basic-rendering/minimal-ray-tracer-rendering-simple-shapes/ray-box-intersection
// https://gamedev.stackexchange.com/questions/18436/most-efficient-aabb-vs-ray-collision-algorithms
export const intersectRayBox = (ray: Ray, box: THREE.Box3): boolean => { 
    
    let tMin = (box.min.x - ray.origin.x) / ray.direction.x; 
    let tMax = (box.max.x - ray.origin.x) / ray.direction.x; 
    if (tMin > tMax) [tMin, tMax] = [tMax, tMin]; 
 
    let tyMin = (box.min.y - ray.origin.y) / ray.direction.y; 
    let tyMax = (box.max.y - ray.origin.y) / ray.direction.y; 
    if (tyMin > tyMax) [tyMin, tyMax] = [tyMax, tyMin]; 
    if ((tMin > tyMax) || (tyMin > tMax)) return false; 
    if (tyMin > tMin) tMin = tyMin; 
    if (tyMax < tMax) tMax = tyMax; 
 
    let tzMin = (box.min.z - ray.origin.z) / ray.direction.z; 
    let tzMax = (box.max.z - ray.origin.z) / ray.direction.z; 
    if (tzMin > tzMax) [tzMin, tzMax] = [tzMax, tzMin]; 
    if ((tMin > tzMax) || (tzMin > tMax)) return false; 
    if (tzMin > tMin) tMin = tzMin; 
    if (tzMax < tMax) tMax = tzMax; 
 
    return true; 
} 

const pointInOrOn = (px: THREE.Vector3, p0: THREE.Vector3, a: THREE.Vector3, b: THREE.Vector3): boolean => {
    const cp1 = b.clone().sub(a).cross(px.clone().sub(a));
    const cp2 = b.clone().sub(a).cross(p0.clone().sub(a));
    return cp1.dot(cp2) >= 0;
}

const pointInTriangle = (px: THREE.Vector3, p0: THREE.Vector3, p1: THREE.Vector3, p2: THREE.Vector3): boolean => {
    return pointInOrOn(px, p0, p1, p2) && pointInOrOn(px, p1, p2, p0) && pointInOrOn(px, p2, p0, p1);
}

// https://stackoverflow.com/questions/59257678/intersect-a-ray-with-a-triangle-in-glsl-c/59261241#59261241
export const intersectTriangle = (ray: Ray, tPts: THREE.Vector3[], maxDistance?: number): Intersection | null => {
    const intersection = intersectRayPlaneByThreePoints(ray, tPts[0], tPts[1], tPts[2]);
    if (!intersection || intersection.distance < 0) {
        return null;
    }
    if (maxDistance && maxDistance <= intersection.distance) {
        return null;
    }
    if (!pointInTriangle(intersection.point, tPts[0], tPts[1], tPts[2])) {
        return null;
    }
    return intersection;
}

export const findTriangleInMesh = (mesh: THREE.Mesh, camera: THREE.Camera, ndc2dCoordinate: THREE.Vector2): THREE.Vector3[] | null => {
    mesh.updateMatrixWorld();
    const objectSpaceRay = transformRay(createWorldSpaceRay(camera, ndc2dCoordinate), mesh.matrixWorld.clone().invert());
    if (!mesh.geometry.boundingBox) {
        mesh.geometry.computeBoundingBox();
    }
    objectSpaceRay.direction.normalize();
    const rayIntersectsBox = mesh.geometry.boundingBox && intersectRayBox(objectSpaceRay, mesh.geometry.boundingBox);
    if (!rayIntersectsBox) {
        return null;
    }
    const vertices = mesh.geometry.attributes.position.array;
    const indices = mesh.geometry.index?.array;
    
    const vertex = (i: number) => new THREE.Vector3(vertices[i*3], vertices[i*3+1], vertices[i*3+2]);
    let minimumIntersection: Intersection | undefined;
    let triangleVertices: THREE.Vector3[] = [new THREE.Vector3(0, 0, 0)];
    for (let ti = 0; ti < (indices ? indices.length : vertices.length); ti += 3) {
        const tV = indices ? [0, 1, 2].map(vi => vertex(indices[ti+vi])) : [0, 1, 2].map(vi => vertex(ti+vi));
        const intersection = intersectTriangle(objectSpaceRay, tV);
        if (intersection && (!minimumIntersection || minimumIntersection.distance > intersection.distance)) {
            minimumIntersection = intersection;
            triangleVertices = tV;
        }
    }
    return triangleVertices;
}

export const calculateIntersectionBox = (mesh1: THREE.Mesh, mesh2: THREE.Mesh, relativeToMesh1?: boolean): THREE.Box3 => {
    if (!mesh1.geometry.boundingBox) {
        mesh1.geometry.computeBoundingBox();
    }
    if (!mesh2.geometry.boundingBox) {
        mesh2.geometry.computeBoundingBox();
    }
    let boxMesh1 = mesh1.geometry.boundingBox;
    let boxMesh2 = mesh2.geometry.boundingBox;
    if (!boxMesh1 || !boxMesh2) {
        return new THREE.Box3();
    }
    if (relativeToMesh1) {
        boxMesh2 = boxMesh2.clone().applyMatrix4(mesh1.matrixWorld.clone().invert().multiply(mesh2.matrixWorld));
        boxMesh1 = boxMesh1.clone();
    } else {
        boxMesh2 = boxMesh2.clone().applyMatrix4(mesh2.matrixWorld);
        boxMesh1 = boxMesh1.clone().applyMatrix4(mesh1.matrixWorld);
    }
    return boxMesh1.intersect(boxMesh2);
}   

export const trianglesInBox = (mesh: THREE.Mesh, searchBox: THREE.Box3): number[] => {
    if (!mesh.geometry.boundingBox) {
        mesh.geometry.computeBoundingBox();
    }
    if (!mesh.geometry.boundingBox) {
        return [];
    }
    
    const box = mesh.geometry.boundingBox.clone().intersect(searchBox);
    if (box.isEmpty()) {
        return [];
    }
    const vertices = mesh.geometry.attributes.position.array;
    const indices = mesh.geometry.index?.array;
    const vertex = (i: number) => new THREE.Vector3(vertices[i*3], vertices[i*3+1], vertices[i*3+2]);
    if (!indices) {
        return [];
    }
    const triangleIndices: number[] = [];
    for (let ti = 0; ti < indices.length; ti += 3) {
        const v0 = vertex(indices[ti]);
        const v1 = vertex(indices[ti+1]);
        const v2 = vertex(indices[ti+2]);
        const triangleBox = new THREE.Box3().expandByPoint(v0).expandByPoint(v1).expandByPoint(v2);
        if (triangleBox.intersectsBox(box)) {
            triangleIndices.push(indices[ti], indices[ti+1], indices[ti+2])
        }
    }
    return triangleIndices;
}

export const intersectTrianglePlane = (t: THREE.Vector3[], plane: Plane): TrianglePlanIntersection | null => {
    const intersect0 = intersectRayPlane({origin: t[0], direction: t[1].clone().sub(t[0])}, plane);
    const intersect0inBounds = intersect0 && intersect0.distance >= 0 && intersect0.distance <= 1;
    const intersect1 = intersectRayPlane({origin: t[1], direction: t[2].clone().sub(t[1])}, plane);
    const intersect1inBounds = intersect1 && intersect1.distance >= 0 && intersect1.distance <= 1;
    if (intersect0inBounds && intersect1inBounds) {
        return { index: 1, intersectToPeak: intersect0, intersectFromPeak: intersect1 };
    } else if (intersect0inBounds) {
        const intersect2 = intersectRayPlane({origin: t[2], direction: t[0].clone().sub(t[2])}, plane);
        if (intersect2 && intersect2.distance >= 0 && intersect2.distance <= 1) {
            return { index: 0, intersectToPeak: intersect2, intersectFromPeak: intersect0 };
        }
    } else if (intersect1inBounds) {
        const intersect2 = intersectRayPlane({origin: t[2], direction: t[0].clone().sub(t[2])}, plane);
        if (intersect2 && intersect2.distance >= 0 && intersect2.distance <= 1) {
            return { index: 2, intersectToPeak: intersect1, intersectFromPeak: intersect2 };
        }
    }
    return null;
}

export const intersectTriangles = (t1: THREE.Vector3[], t2: THREE.Vector3[]): TriangleTriangleIntersection | null => {
    const plan2 = planeFromTriangle(t2[0], t2[1], t2[2]);
    const intersectionTriangle1 = intersectTrianglePlane(t1, plan2);
    if (!intersectionTriangle1) {
        return null;
    } 
    const plan1 = planeFromTriangle(t1[0], t1[1], t1[2]);
    const intersectionTriangle2 = intersectTrianglePlane(t2, plan1);
    if (!intersectionTriangle2) {
        return null;
    } 
    // TODO pointInTriangle?
    return { intersectionTriangle1, intersectionTriangle2 };
}

export class Triangle {
    public vertexArray: ArrayLike<number>;
    private _indices: number[];
    private _vertices: THREE.Vector3[] | undefined;
    private _box: THREE.Box3 | undefined; 
    private _normalVector: THREE.Vector3 | undefined; 

    get vertices(): THREE.Vector3[] {
        this._vertices ??= [0, 1, 2].map(i => this.createVertexByIndex(this._indices[i]));
        return this._vertices;
    }

    get faceNormal(): THREE.Vector3 {
        this._normalVector ??= this.vertices[1].clone().sub(this.vertices[0]).cross(this.vertices[2].clone().sub(this.vertices[0]));
        return this._normalVector;
    }

    get indices(): number[] {
        return this._indices;
    }

    get box(): THREE.Box3 {
        if (!this._box) {
            this._box = new THREE.Box3();
            this.vertices.forEach(v => this.box.expandByPoint(v));
        }
        return this._box;
    }

    constructor(vertexArray: ArrayLike<number>, i0: number, i1: number, i2: number) {
        this.vertexArray = vertexArray;
        this._indices = [i0, i1, i2];
    }

    public applyMatrix(transformation: THREE.Matrix4) {
        this._vertices = this.vertices.map(v => v.applyMatrix4(transformation));
    }

    public createRay(from: number, to: number) {
        const r0 = this.vertices[from];
        const r1 = this.vertices[to];
        return{origin: r0, direction: r1.clone().sub(r0) };
    }

    public barycentricCoordinate(p: THREE.Vector3): THREE.Vector3 {
        return barycentricCoordinate(
            this.vertices[1].clone().sub(this.vertices[0]),
            this.vertices[2].clone().sub(this.vertices[0]),
            p.clone().sub(this.vertices[0])
        );
    }

    private createVertexByIndex(i: number) {
        return new THREE.Vector3(this.vertexArray[i*3], this.vertexArray[i*3+1], this.vertexArray[i*3+2]);
    }  
}

export class UniqueIndices {
    public nextIndex: number = 0;
    public vertexToIndexMap = new Map<number, Map<number, Map<number, number>>>();
    
    private getIndex(x: number, y: number, z: number):number {
        let yzMap = this.vertexToIndexMap.get(x);
        if (yzMap === undefined) {
            yzMap = new Map<number, Map<number, number>>();
            this.vertexToIndexMap.set(x, yzMap);
        }
        let zMap = yzMap.get(y);
        if (zMap === undefined) {
            zMap = new Map<number, number>();
            yzMap.set(y, zMap);
        }
        let index = zMap.get(z);
        if (index === undefined) {
            index = this.nextIndex;
            zMap.set(z, index);
            this.nextIndex ++;
        }
        return index;
    }

    public getVertexIndex(v: THREE.Vector3, epsilon: number = 0.001): number {
        return this.getIndex(Math.round(v.x / epsilon), Math.round(v.y / epsilon), Math.round(v.z / epsilon));
    }

    public createUniqueIndices(vertices: ArrayLike<number>, indices: ArrayLike<number>, epsilon: number = 0.001): Map<number, number> {
        const indexMap = new Map<number, number>;
        for (let i=0; i < indices.length; ++i) {
            const index = indices[i];
            if (indexMap.has(index)) {
                continue;
            }
            const uniqueIndex = this.getIndex(
                Math.round(vertices[index*3] / epsilon), 
                Math.round(vertices[index*3 + 1] / epsilon), 
                Math.round(vertices[index*3 + 2] / epsilon));
            indexMap.set(index, uniqueIndex);
        }
        return indexMap;
    }
}

export const intersectMesh = (mesh1: THREE.Mesh, mesh2: THREE.Mesh, epsilon = 0.001): MeshSpecification | null => {
    const vertices1 = mesh1.geometry.attributes.position.array;
    const indices1Buffer = mesh1.geometry.index?.array;
    const vertices2 = mesh2.geometry.attributes.position.array;
    const indices2 = mesh2.geometry.index?.array;
    if (!indices1Buffer || !indices2) {
        return null;
    }
    const normals1 = mesh1.geometry.attributes.normal?.array;
    //const normals2 = mesh1.geometry.attributes.normal?.array;
    const indices1 = Array.from(indices1Buffer);

    const relativeTransform = mesh1.matrixWorld.clone().invert().multiply(mesh2.matrixWorld);
    const intersectionBox = calculateIntersectionBox(mesh1, mesh2, true);
    if (intersectionBox.isEmpty()) {
        return null;
    }
    const trianglesMesh2: Triangle[] = []
    for (let ti2 = 0; ti2 < indices2.length; ti2 += 3) {
        const triangle2 = new Triangle(vertices2, indices2[ti2], indices2[ti2+1], indices2[ti2+2]);
        triangle2.applyMatrix(relativeTransform);
        if (intersectionBox.intersect(triangle2.box)) {
            trianglesMesh2.push(triangle2);
        }
    }
    if (trianglesMesh2.length == 0) {
        return null;
    }

    const pushNormals = (normals: ArrayLike<number> | undefined, indices: number[], barycentric: THREE.Vector3) => {
        if (normals) {
            const nv = (i: number) => new THREE.Vector3(normals[i*3], normals[i*3+1], normals[i*3+2]); 
            const n = interpolateBarycentric(barycentric, indices.map(i => nv(i)));
            newNormals.push(n.x, n.y, n.z);
        }
    }
    const pushNewVertex = (newVertex: THREE.Vector3, triangle: Triangle) => {
        mesh1UniqueIndices.set(newVertices.length / 3, uniqueIndices.getVertexIndex(newVertex));
        newVertices.push(newVertex.x, newVertex.y, newVertex.z);
        pushNormals(newNormals, triangle.indices, triangle.barycentricCoordinate(newVertex));
    }
    const pushTriangle = (intersectionDirection: THREE.Vector3, i0: number, i1: number, i2:number) => {
        const uniqueI = [i0, i1, i2].map(i => mesh1UniqueIndices.get(i)); 
        if (uniqueI[0] == uniqueI[1] || uniqueI[0] == uniqueI[2] || uniqueI[1] == uniqueI[2]) {
            return;
        }
        const v = [i0, i1, i2].map(i => new THREE.Vector3(newVertices[i*3], newVertices[i*3+1], newVertices[i*3+2]));
        indices1.push(i0, i1, i2);
        const d = intersectionDirection.dot(v[2].sub(v[1]));
        windings.push(d < 0 ? -1 : 1);
    }
    const pushWindingAndTriangle = (w: number, i0: number, i1: number, i2:number) => {
        const uniqueI = [i0, i1, i2].map(i => mesh1UniqueIndices.get(i)); 
        if (uniqueI[0] == uniqueI[1] || uniqueI[0] == uniqueI[2] || uniqueI[1] == uniqueI[2]) {
            return;
        }
        const v = [i0, i1, i2].map(i => new THREE.Vector3(newVertices[i*3], newVertices[i*3+1], newVertices[i*3+2]));
        indices1.push(i0, i1, i2);
        windings.push(w);
    }

    const uniqueIndices = new UniqueIndices();
    const mesh1UniqueIndices = uniqueIndices.createUniqueIndices(vertices1, indices1);
    const mesh2UniqueIndices = uniqueIndices.createUniqueIndices(vertices2, indices2);
    const windings: number[] = new Array(indices1.length / 3).fill(0);
    const newVertices: number [] = Array.from(vertices1);
    const newWindings: number [] = [];
    const newIndices: number [] = [];
    const newNormals: number [] = normals1 ? Array.from(normals1) : [];
    let error: boolean = false;
    for (let ti1 = 0; ti1 < indices1.length; ti1 += 3) {
        let winding = windings[ti1 / 3];
        if (indices1.length > indices1Buffer.length * 10) {
            error = true;
            break;
        }
        const triangle1 = new Triangle(newVertices, indices1[ti1], indices1[ti1+1], indices1[ti1+2]);
        if (!intersectionBox.intersect(triangle1.box)) {
            newIndices.push(...triangle1.indices);
            newWindings.push(winding);
            continue;
        }

        let splitted: boolean = false;
        for (let ti2 = 0; ti2 < trianglesMesh2.length; ++ti2) {
            const triangle2 = trianglesMesh2[ti2];
        
            if (!triangle1.box.intersectsBox(triangle2.box)) {
                continue;
            }
            const intersectionDirection = triangle2.faceNormal.clone().cross(triangle1.faceNormal);

            const shardPoints2: number[] = [];
            const sharedPoints = [0, 1, 2].filter(i => {
                const ui = mesh1UniqueIndices.get(triangle1.indices[i]);
                for (let j=0; j < 3; ++j) {
                    if (ui === mesh2UniqueIndices.get(triangle2.indices[j])) {
                        shardPoints2.push(j);
                        return true;
                    }
                }
                return false;
            });
            if (sharedPoints.length >= 2) {
                const ptDir = triangle1.vertices[sharedPoints[1]].clone().sub(triangle1.vertices[sharedPoints[0]]);
                if (sharedPoints[0] === 0 && sharedPoints[1] === 2) {
                    ptDir.multiplyScalar(-1);
                }
                const d = intersectionDirection.dot(ptDir);
                winding = d < 0 ? -1 : 1;
                continue;
            }
            if (sharedPoints.length === 1) {
                let ray = triangle1.createRay((sharedPoints[0]+1) % 3, (sharedPoints[0]+2) % 3);
                let triangleIntersect = intersectTriangle(ray, triangle2.vertices);
                if (triangleIntersect && triangleIntersect.distance > 0 && triangleIntersect.distance < 1) {
                    const uniqueI = uniqueIndices.getVertexIndex(triangleIntersect.point);
                    if (!triangle1.indices.map(i => mesh1UniqueIndices.get(i)).includes(uniqueI)) {  
                        const newI = newVertices.length / 3;
                        pushNewVertex(triangleIntersect.point, triangle1);
                        pushTriangle(intersectionDirection, indices1[ti1 + (sharedPoints[0]+1) % 3], newI, indices1[ti1 + sharedPoints[0]]);
                        pushTriangle(intersectionDirection, indices1[ti1 + (sharedPoints[0]+2) % 3], indices1[ti1 + sharedPoints[0]], newI);
                        splitted = true;
                        break;
                    }
                }
                ray = triangle2.createRay((shardPoints2[0]+1) % 3, (shardPoints2[0]+2) % 3);
                triangleIntersect = intersectTriangle(ray, triangle1.vertices);
                if (triangleIntersect && triangleIntersect.distance > 0 && triangleIntersect.distance < 1) {
                    const uniqueI = uniqueIndices.getVertexIndex(triangleIntersect.point);
                    if (!triangle1.indices.map(i => mesh1UniqueIndices.get(i)).includes(uniqueI)) {  
                        const newI = newVertices.length / 3;
                        pushNewVertex(triangleIntersect.point, triangle1);
                        pushTriangle(intersectionDirection, indices1[ti1 + (sharedPoints[0]+1) % 3], newI, indices1[ti1 + sharedPoints[0]]);
                        pushTriangle(intersectionDirection, indices1[ti1 + (sharedPoints[0]+2) % 3], indices1[ti1 + sharedPoints[0]], newI);
                        pushWindingAndTriangle(windings[ti1 / 3], indices1[ti1 + (sharedPoints[0]+1) % 3], indices1[ti1 + (sharedPoints[0]+2) % 3], newI);
                        splitted = true;
                        break;
                    }
                }
                continue;
            }

            const intersect = intersectTriangles(triangle1.vertices, triangle2.vertices);
            if (!intersect) {
                continue;
            }
            let p = [ 
                { t:0, i: intersect.intersectionTriangle1.intersectToPeak, d: 0 }, 
                { t:0, i: intersect.intersectionTriangle1.intersectFromPeak, d: 0 }, 
                { t:1, i: intersect.intersectionTriangle2.intersectToPeak, d: 0 }, 
                { t:1, i: intersect.intersectionTriangle2.intersectFromPeak, d: 0 }, 
            ];
            const refP = p[0].i.point.clone();
            const directionVector = p[1].i.point.clone().sub(refP);
            if (directionVector.length() < epsilon) {
                continue;
            }
            p.forEach(item => item.d = directionVector.dot(item.i.point.clone().sub(refP)));
            p.sort((a, b) => a.d - b.d);
            if (p[0].t === p[1].t) {
                continue;
            }
            const uniqueI1 = uniqueIndices.getVertexIndex(p[1].i.point);
            const uniqueI2 = uniqueIndices.getVertexIndex(p[2].i.point);
            const orderedIndices = [0, 1, 2].map(i => triangle1.indices[(intersect.intersectionTriangle1.index+i) % 3]);
            const uniqueT = orderedIndices.map(i => mesh1UniqueIndices.get(i));  
            if (uniqueT.includes(uniqueI1) && uniqueT.includes(uniqueI2)) { 
                if (winding == 0) {
                    const d = intersectionDirection.dot(triangle1.vertices[(intersect.intersectionTriangle1.index+2) % 3].clone().sub(triangle1.vertices[(intersect.intersectionTriangle1.index+1) % 3]));
                    winding = d < 0 ? -1 : 1;
                }
                continue;
            }
            const newI = newVertices.length / 3;
            pushNewVertex(p[1].i.point, triangle1);
            pushNewVertex(p[2].i.point, triangle1);
            if (p[0].t === 0 && (p[1].d - p[0].d) > epsilon) {
                pushWindingAndTriangle(windings[ti1 / 3], orderedIndices[2], orderedIndices[0], newI);
            }
            pushTriangle(intersectionDirection, orderedIndices[0], newI+1, newI);
            pushTriangle(intersectionDirection, orderedIndices[2], newI, newI+1);
            pushWindingAndTriangle(windings[ti1 / 3], orderedIndices[1], orderedIndices[2], newI+1);
            if (p[3].t === 0 && (p[3].d - p[2].d) > epsilon) {
                pushWindingAndTriangle(windings[ti1 / 3], orderedIndices[0], orderedIndices[1], newI+1);
            }
            splitted = true;
            break;
        }
        if (!splitted) {
            newIndices.push(...triangle1.indices);
            newWindings.push(winding);
        }
    }
    let orderChanged = true;
    while (orderChanged) {
        orderChanged = false;
        for (let i = 0; i < newWindings.length; ++i) {
            if (newWindings[i] === 0) {
                for (let j = 0; j < newWindings.length; ++j) {
                    if (newWindings[j] == 1 && i !== j) {
                        const uI1 = [0, 1, 2].map(n => mesh1UniqueIndices.get(newIndices[i*3 + n]));
                        const uI2 = [0, 1, 2].map(n => mesh1UniqueIndices.get(newIndices[j*3 + n]));
                        const count = (uI2.includes(uI1[0]) ? 1 : 0) + (uI2.includes(uI1[1]) ? 1 : 0) + (uI2.includes(uI1[2]) ? 1 : 0);
                        if (count === 2) {
                            newWindings[i] = 1;
                            orderChanged = true;
                            break;
                        }
                    }
                }
            }
        }
    }
    //orderChanged = true;
    //while (orderChanged) {
    //    orderChanged = false;
    //    for (let i = 0; i < newWindings.length; ++i) {
    //        if (newWindings[i] == 0) {
    //            for (let j = 0; j < newWindings.length; ++j) {
    //                if (newWindings[j] == -1 && i !== j) {
    //                    const uI1 = [0, 1, 2].map(n => mesh1UniqueIndices.get(newIndices[i*3 + n]));
    //                    const uI2 = [0, 1, 2].map(n => mesh1UniqueIndices.get(newIndices[j*3 + n]));
    //                    const count = (uI2.includes(uI1[0]) ? 1 : 0) + (uI2.includes(uI1[1]) ? 1 : 0) + (uI2.includes(uI1[2]) ? 1 : 0);
    //                    if (count === 2) {
    //                        newWindings[i] = -1;
    //                        orderChanged = true;
    //                        break;
    //                    }
    //                }
    //            }
    //        }
    //    }
    //}
    const anyChanged = newWindings.some(w => w !== 0);
    if (anyChanged) {
        for (let i = 0; i < newWindings.length; ++i) {
            if (newWindings[i] <= 0) {
                [newIndices[i*3], newIndices[i*3+1]] = [newIndices[i*3+1], newIndices[i*3]];
            }
        }
    }

    const newMesh = { 
        vertices: new Float32Array(newVertices), 
        normals: newNormals && newNormals.length > 0 ? new Float32Array(newNormals) : undefined,
        indices: new Uint32Array(newIndices),
        error,
        anyChanged: anyChanged || newVertices.length > vertices1.length
    };
    return newMesh;
}