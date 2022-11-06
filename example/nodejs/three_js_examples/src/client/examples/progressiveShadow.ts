import * as THREE from 'three';
import { OrbitControls } from 'three/examples/jsm/controls/OrbitControls'
import { TransformControls } from 'three/examples/jsm/controls/TransformControls.js';
import { ProgressiveLightMap } from 'three/examples/jsm/misc/ProgressiveLightMap.js';
import Stats from 'three/examples/jsm/libs/stats.module' 
import { GUI } from 'dat.gui'

export const progressiveShadow = (canvas: any) => {
    const shadowMapRes = 512, lightMapRes = 1024, lightCount = 8;
    const dirLights: any = [], lightmapObjects: any = [];
    const params = { 
        'Enable': true, 
        'Blur Edges': true, 
        'Blend Window': 200,                
        'Light Radius': 50, 
        'Ambient Weight': 0.5, 
        'Debug Lightmap': false 
    };

    const renderer = new THREE.WebGLRenderer( { antialias: true } );
    renderer.setPixelRatio( window.devicePixelRatio );
    renderer.setSize( window.innerWidth, window.innerHeight );
    renderer.shadowMap.enabled = true;
    document.body.appendChild( renderer.domElement );
    // @ts-ignore
    const stats = new Stats();
    document.body.appendChild(stats.dom);

    const camera = new THREE.PerspectiveCamera( 70, window.innerWidth / window.innerHeight, 1, 1000 );
    camera.position.set(0, 100, 1000);
    camera.name = 'Camera';

    const scene = new THREE.Scene();
    scene.background = new THREE.Color( 0x949494 );
    //scene.fog = new THREE.Fog( 0x949494, 1000, 3000 );

    const progressiveLightMap = new ProgressiveLightMap( renderer, lightMapRes );

    const lightOrigin = new THREE.Group();
    lightOrigin.position.set(60, 250, 100);
    scene.add(lightOrigin);

    const control = new TransformControls( camera, renderer.domElement );
    control.addEventListener( 'dragging-changed', ( event: any ) => {
        controls.enabled = ! event.value;
    } );
    control.attach(lightOrigin);
    scene.add( control );

    for ( let l = 0; l < lightCount; l ++ ) {
        const dirLight = new THREE.DirectionalLight( 0xffffff, 1.0 / lightCount );
        dirLight.name = 'Dir. Light ' + l;
        dirLight.position.set( 200, 200, 200 );
        dirLight.castShadow = true;
        dirLight.shadow.camera.near = 100;
        dirLight.shadow.camera.far = 5000;
        dirLight.shadow.camera.right = 150;
        dirLight.shadow.camera.left = - 150;
        dirLight.shadow.camera.top = 150;
        dirLight.shadow.camera.bottom = - 150;
        dirLight.shadow.mapSize.width = shadowMapRes;
        dirLight.shadow.mapSize.height = shadowMapRes;
        lightmapObjects.push( dirLight );
        dirLights.push( dirLight );
    }

    // ground
    const groundMesh = new THREE.Mesh(
        new THREE.PlaneGeometry( 1000, 1000 ),
        new THREE.MeshPhongMaterial( { color: 0xffffff, depthWrite: true } )
    );
    groundMesh.position.y = - 0.1;
    groundMesh.rotation.x = - Math.PI / 2;
    groundMesh.name = 'Ground Mesh';
    lightmapObjects.push( groundMesh );
    scene.add( groundMesh );

    const torusKnot = new THREE.Mesh(new THREE.TorusKnotGeometry(50, 10, 100, 32), new THREE.MeshLambertMaterial({color: 0xff80ff}))
    const object = torusKnot
    torusKnot.position.set(0, 100, 0)
    scene.add(torusKnot)
    loadModel(torusKnot, lightmapObjects)

    const control2 = new TransformControls(camera, renderer.domElement);
    control2.addEventListener( 'dragging-changed', ( event: any ) => {
        controls.enabled = ! event.value;
    } );
    control2.attach(torusKnot);
    scene.add(control2);

    progressiveLightMap.addObjectsToLightMap(lightmapObjects);

    // controls
    const controls = new OrbitControls( camera, renderer.domElement );
    controls.enableDamping = true; // an animation loop is required when either damping or auto-rotation are enabled
    controls.dampingFactor = 0.05;
    controls.screenSpacePanning = true;
    controls.minDistance = 100;
    controls.maxDistance = 500;
    controls.maxPolarAngle = Math.PI / 1.5;
    controls.target.set( 0, 100, 0 );
    
    const gui = new GUI( { name: 'Accumulation Settings' } );
    gui.add( params, 'Enable' );
    gui.add( params, 'Blur Edges' );
    gui.add( params, 'Blend Window', 1, 500 ).step( 1 );
    gui.add( params, 'Light Radius', 0, 200 ).step( 10 );
    gui.add( params, 'Ambient Weight', 0, 1 ).step( 0.1 );
    gui.add( params, 'Debug Lightmap' );

    const onWindowResize = () => {
        camera.aspect = window.innerWidth / window.innerHeight
        camera.updateProjectionMatrix()
        renderer.setSize(window.innerWidth, window.innerHeight)
    }
    window.addEventListener( 'resize', onWindowResize );
    
    
    let start: number, previousTimeStamp: number;
    const animate = (timestamp: number) => {
        if (start === undefined) {
            start = timestamp;
        }
        if (previousTimeStamp === undefined) {
            previousTimeStamp = timestamp;
        }
        const allTimeMs = timestamp - start;
        const elapsedMs = timestamp - previousTimeStamp;
        previousTimeStamp = timestamp
        requestAnimationFrame(animate)

        controls.update();

        // Accumulate Surface Maps
        if ( params[ 'Enable' ] ) {
            progressiveLightMap.update( camera, params[ 'Blend Window' ], params[ 'Blur Edges' ] );
            if ( ! progressiveLightMap.firstUpdate ) {        
                progressiveLightMap.showDebugLightmap( params[ 'Debug Lightmap' ] );
            }
        }
        
        for ( let l = 0; l < dirLights.length; l ++ ) {
            // Sometimes they will be sampled from the target direction
            // Sometimes they will be uniformly sampled from the upper hemisphere
            if ( Math.random() > params[ 'Ambient Weight' ] ) {
                dirLights[ l ].position.set(
                    lightOrigin.position.x + ( Math.random() * params[ 'Light Radius' ] ),
                    lightOrigin.position.y + ( Math.random() * params[ 'Light Radius' ] ),
                    lightOrigin.position.z + ( Math.random() * params[ 'Light Radius' ] ) );
            } else {
                // Uniform Hemispherical Surface Distribution for Ambient Occlusion
                const lambda = Math.acos( 2 * Math.random() - 1 ) - ( 3.14159 / 2.0 );
                const phi = 2 * 3.14159 * Math.random();
                dirLights[ l ].position.set(
                            ( ( Math.cos( lambda ) * Math.cos( phi ) ) * 300 ) + object.position.x,
                    Math.abs( ( Math.cos( lambda ) * Math.sin( phi ) ) * 300 ) + object.position.y + 20,
                                ( Math.sin( lambda ) * 300 ) + object.position.z
                );
            }
        }

        render()
        stats.update()
    }

    const render = () => {
        renderer.render(scene, camera)
    }
    animate(0)    
}


const loadModel = (object: THREE.Object3D, lightmapObjects: any[]) => {

    object.traverse(child => {
        // @ts-ignore
        if ( child.isMesh ) {
            child.name = 'Loaded Mesh';
            child.castShadow = true;
            child.receiveShadow = true;
            // @ts-ignore
            //child.material = new THREE.MeshPhongMaterial();
            //child.material = new THREE.MeshPhysicalMaterial();
            // @ts-ignore
            //child.material.color = new THREE.Color(1, 0.5, 0.7)
            // This adds the model to the lightmap
            lightmapObjects.push( child );
        } else {
            child.layers.disableAll(); // Disable Rendering for this
        }
    });
    
    /*
    const lightTarget = new THREE.Group();
    lightTarget.position.set( 0, 20, 0 );
    for ( let l = 0; l < dirLights.length; l ++ ) {
        dirLights[ l ].target = lightTarget;
    }
    object.add( lightTarget );
    */

    // @ts-ignore
    /*
    if ( typeof TESTING !== 'undefined' ) {
        for ( let i = 0; i < 300; i ++ ) {
            render();
        }
    }
    */
}