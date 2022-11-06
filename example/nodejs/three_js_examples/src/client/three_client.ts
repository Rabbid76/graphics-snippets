import { helloCube } from './examples/helloCube'
import { contactShadow } from './examples/contactShadow'
import { progressiveShadow } from './examples/progressiveShadow'

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
    case 'progressive_shadow': progressiveShadow(canvas); break
}