import { helloCube } from "./examples/hello_cube"
import { contactShadow } from "./examples/contact_shadow"

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
}