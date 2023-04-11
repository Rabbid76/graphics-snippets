import {
    Camera,
    Object3D,
    WebGLRenderer
} from 'three'
import { OrbitControls } from 'three/examples/jsm/controls/OrbitControls'
import { TransformControls } from 'three/examples/jsm/controls/TransformControls.js';

export class Controls {
    public renderer: WebGLRenderer
    public camera: Camera
    public orbitControls: OrbitControls

    constructor(renderer: WebGLRenderer, camera: Camera) {
        this.renderer = renderer
        this.camera = camera
        this.orbitControls = new OrbitControls(camera, renderer.domElement)
    }

    public addTransformControl(object: Object3D, target: Object3D): TransformControls {
        const control = new TransformControls(this.camera, this.renderer.domElement);
        control.addEventListener( 'dragging-changed', (event: any) => {
            this.orbitControls.enabled = !event.value;
        } );
        control.attach(object);
        target.add(control);
        return control;
    }

    public update() {
        this.orbitControls.update()
    }
}