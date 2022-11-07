import * as THREE from 'three'
import { OrbitControls } from 'three/examples/jsm/controls/OrbitControls'
import { TransformControls } from 'three/examples/jsm/controls/TransformControls.js';

export class Controls {
    public renderer: THREE.WebGLRenderer
    public camera: THREE.Camera
    public orbitControls: OrbitControls

    constructor(renderer: THREE.WebGLRenderer, camera: THREE.Camera) {
        this.renderer = renderer
        this.camera = camera
        this.orbitControls = new OrbitControls(camera, renderer.domElement)
    }

    public addTransformControl(object: THREE.Object3D, target: THREE.Object3D) {
        const control = new TransformControls(this.camera, this.renderer.domElement);
        control.addEventListener( 'dragging-changed', (event: any) => {
            this.orbitControls.enabled = !event.value;
        } );
        control.attach(object);
        target.add(control);
    }

    public update() {
        this.orbitControls.update()
    }
}