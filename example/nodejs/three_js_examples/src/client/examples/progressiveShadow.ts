import { ElapsedTime } from '../three/timeUtility'
import { Controls } from '../three/controls'
import { DataGUI, Statistic } from '../three/uiUtility' 
import * as THREE from 'three';
import { ProgressiveLightMap } from 'three/examples/jsm/misc/ProgressiveLightMap.js';

export const progressiveShadow = (canvas: any) => {
    const shadowMapRes = 512, lightMapRes = 1024, lightCount = 8;
    const dirLights: any = [], lightMapObjects: any = [];
    const params = { 
        'Enable': true, 
        'Blur Edges': true, 
        'Blend Window': 200,                
        'Light Radius': 50, 
        'Ambient Weight': 0.5, 
        'Debug Lightmap': false 
    };

    const renderer = new THREE.WebGLRenderer({ canvas: canvas, antialias: true })
    renderer.setPixelRatio(window.devicePixelRatio)
    renderer.setSize(window.innerWidth, window.innerHeight)
    renderer.shadowMap.enabled = true;
    document.body.appendChild( renderer.domElement )
    const statistic = new Statistic();

    const camera = new THREE.PerspectiveCamera(70, window.innerWidth / window.innerHeight, 1, 1000)
    camera.position.set(0, 500, 1000)

    const scene = new THREE.Scene()
    scene.background = new THREE.Color(0x949494)
    //scene.fog = new THREE.Fog(0x949494, 1000, 3000)

    const controls = new Controls(renderer, camera)
    controls.orbitControls.enableDamping = true // an animation loop is required when either damping or auto-rotation are enabled
    controls.orbitControls.dampingFactor = 0.05
    controls.orbitControls.screenSpacePanning = true
    controls.orbitControls.minDistance = 100
    controls.orbitControls.maxDistance = 500
    controls.orbitControls.maxPolarAngle = Math.PI / 1.5
    controls.orbitControls.target.set(0, 100, 0)

    const lightOrigin = new THREE.Group()
    lightOrigin.position.set(0, 300, -200)
    scene.add(lightOrigin);
    controls.addTransformControl(lightOrigin, scene)

    for ( let l = 0; l < lightCount; l ++ ) {
        const dirLight = new THREE.DirectionalLight( 0xffffff, 1.0 / lightCount )
        dirLight.name = 'Dir. Light ' + l
        dirLight.position.set(0, 0, 0)
        dirLight.castShadow = true
        dirLight.shadow.camera.near = 100
        dirLight.shadow.camera.far = 5000
        dirLight.shadow.camera.right = 300
        dirLight.shadow.camera.left = -300
        dirLight.shadow.camera.top = 300
        dirLight.shadow.camera.bottom = -300
        dirLight.shadow.mapSize.width = shadowMapRes
        dirLight.shadow.mapSize.height = shadowMapRes
        lightMapObjects.push(dirLight)
        dirLights.push(dirLight)
    }

    // ground
    const groundMesh = new THREE.Mesh(
        new THREE.PlaneGeometry( 1000, 1000 ),
        new THREE.MeshPhongMaterial( { color: 0xffffff, depthWrite: true } )
    );
    groundMesh.position.y = - 0.1;
    groundMesh.rotation.x = - Math.PI / 2;
    groundMesh.name = 'Ground Mesh';
    lightMapObjects.push( groundMesh );
    scene.add( groundMesh );

    const torusKnot = new THREE.Mesh(new THREE.TorusKnotGeometry(50, 15, 100, 32), new THREE.MeshLambertMaterial({color: 0xff80ff}))
    torusKnot.position.set(100, 150, 0)
    scene.add(torusKnot)
    loadModel(torusKnot, lightMapObjects)
    controls.addTransformControl(torusKnot, scene)

    const torusGroup = new THREE.Group() 
    const torus1 = new THREE.Mesh(new THREE.TorusGeometry(50, 15, 32, 100), new THREE.MeshLambertMaterial({color: 0xffff80}))
    torus1.rotation.x = Math.PI / 4
    torus1.position.set(-25, 0, 0)
    torusGroup.add(torus1)
    const torus2 = new THREE.Mesh(new THREE.TorusGeometry(50, 15, 32, 100), new THREE.MeshLambertMaterial({color: 0xff8080}))
    torus2.rotation.x = -Math.PI / 4
    torus2.position.set(25, 0, 0)
    torusGroup.add(torus2)
    torusGroup.position.set(-100, 150, 0)
    scene.add(torusGroup)
    loadModel(torusGroup, lightMapObjects)
    controls.addTransformControl(torusGroup, scene)

    const progressiveLightMap = new ProgressiveLightMap(renderer, lightMapRes)
    progressiveLightMap.addObjectsToLightMap(lightMapObjects);
    const object = torusKnot
    
    const dataGui = new DataGUI({ name: 'Accumulation Settings' });
    dataGui.gui.add( params, 'Enable' );
    dataGui.gui.add( params, 'Blur Edges' );
    dataGui.gui.add( params, 'Blend Window', 1, 500 ).step( 1 );
    dataGui.gui.add( params, 'Light Radius', 0, 200 ).step( 10 );
    dataGui.gui.add( params, 'Ambient Weight', 0, 1 ).step( 0.1 );
    dataGui.gui.add( params, 'Debug Lightmap' );

    const onWindowResize = () => {
        camera.aspect = window.innerWidth / window.innerHeight
        camera.updateProjectionMatrix()
        renderer.setSize(window.innerWidth, window.innerHeight)
    }
    window.addEventListener( 'resize', onWindowResize );
    
    
    const elapsedTime = new ElapsedTime();
    const animate = (timestamp: number) => {
        elapsedTime.update(timestamp);
        requestAnimationFrame(animate)

        controls.update();

        // Accumulate Surface Maps
        if ( params[ 'Enable' ] ) {
            progressiveLightMap.update(camera, params['Blend Window'], params['Blur Edges'])
            if (!progressiveLightMap.firstUpdate) {        
                progressiveLightMap.showDebugLightmap(params['Debug Lightmap'])
            }
        }
        
        for ( let l = 0; l < dirLights.length; l ++ ) {
            // Sometimes they will be sampled from the target direction
            // Sometimes they will be uniformly sampled from the upper hemisphere
            if ( Math.random() > params[ 'Ambient Weight' ] ) {
                dirLights[ l ].position.set(
                    lightOrigin.position.x + ( (Math.random() * 2 - 1) * params['Light Radius']),
                    lightOrigin.position.y + ( (Math.random() * 2 - 1) * params['Light Radius']),
                    lightOrigin.position.z + ( (Math.random() * 2 - 1) * params['Light Radius']))
            } else {
                // Uniform Hemispherical Surface Distribution for Ambient Occlusion
                const lambda = Math.acos(2 * Math.random() - 1) - (3.14159 / 2.0)
                const phi = 2 * 3.14159 * Math.random()
                dirLights[l].position.set(
                            ( ( Math.cos( lambda ) * Math.cos( phi ) ) * 300 ) + object.position.x,
                    Math.abs( ( Math.cos( lambda ) * Math.sin( phi ) ) * 300 ) + object.position.y + 20,
                                ( Math.sin( lambda ) * 300 ) + object.position.z
                );
            }
        }

        render()
        statistic.update();
    }

    const render = () => {
        renderer.render(scene, camera)
    }
    animate(0)    
}

const loadModel = (object: THREE.Object3D, lightMapObjects: any[]) => {

    object.traverse(child => {
        // @ts-ignore
        if ( child.isMesh ) {
            child.name = 'Loaded Mesh';
            child.castShadow = true;
            child.receiveShadow = true;
            lightMapObjects.push( child );
        } else {
            child.layers.disableAll(); // Disable Rendering for this
        }
    });

    // @ts-ignore
    /*
    if ( typeof TESTING !== 'undefined' ) {
        for ( let i = 0; i < 300; i ++ ) {
            render();
        }
    }
    */
}