import { helloCube } from './examples/helloCube'
import { contactShadow } from './examples/contactShadow'
import { rectAreaLightShadow } from './examples/rectAreaLightShadow'
import { progressiveShadow } from './examples/progressiveShadow'
import { screenSpaceAmbientOcclusion } from './examples/screenSpaceAmbientOcclusion'

// @ts-ignore
const canvas: any = three_canvas
let example: string = ""

const threeCanvas = document.getElementById('three_canvas')
const exampleAttribute = threeCanvas?.getAttribute('example')
if (exampleAttribute) {
    console.log(`load example ${exampleAttribute}`)
    example = exampleAttribute
}

switch(example) {
    default: helloCube(canvas); break
    case 'hello_cube': helloCube(canvas); break
    case 'contact_shadow': contactShadow(canvas); break
    case 'rect_area_light_shadow': rectAreaLightShadow(canvas); break;
    case 'progressive_shadow': progressiveShadow(canvas); break
    case 'screen_space_ambient_occlusion': screenSpaceAmbientOcclusion(canvas); break;
}