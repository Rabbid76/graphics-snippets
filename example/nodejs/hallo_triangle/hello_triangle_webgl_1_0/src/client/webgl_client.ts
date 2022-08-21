const canvas = document.getElementById("webgl_canvas") as HTMLCanvasElement
const gl = canvas.getContext("experimental-webgl") as WebGLRenderingContext

const vertexShader : string = 
`#version 100
attribute vec3 inPos;
attribute vec4 inColor;
varying vec4 vColor;
uniform mat4 projection;
void main()
{
    vColor = inColor;
    gl_Position = projection * vec4(inPos, 1.0);
}
`

const fragmentShader : string = 
`#version 100
precision mediump float;
varying vec4 vColor;
void main()
{
    gl_FragColor = vColor;
}
`

const progDraw = gl.createProgram() as WebGLProgram
for (let i = 0; i < 2; ++i) {
    const source = i==0 ? vertexShader : fragmentShader
    const shaderObj = gl.createShader(i==0 ? gl.VERTEX_SHADER : gl.FRAGMENT_SHADER) as WebGLShader
    gl.shaderSource(shaderObj, source)
    gl.compileShader(shaderObj)
    let status = gl.getShaderParameter(shaderObj, gl.COMPILE_STATUS)
    if (!status) alert(gl.getShaderInfoLog(shaderObj))
    gl.attachShader(progDraw, shaderObj)
    gl.linkProgram(progDraw)
}
if (!gl.getProgramParameter(progDraw, gl.LINK_STATUS)) alert(gl.getProgramInfoLog(progDraw))
const inPosAttribLoc : number = gl.getAttribLocation(progDraw, "inPos")
const inColorAttribLoc : number = gl.getAttribLocation(progDraw, "inColor")
const projectionLocation = gl.getUniformLocation(progDraw, "projection") as WebGLUniformLocation;
gl.useProgram(progDraw)

var attributes = [
//   x       y     z    R  G  B  A
    -0.866, -0.75, 0,   1, 0, 0, 1, 
     0.866, -0.75, 0,   1, 1, 0, 1,
     0,      0.75, 0,   0, 0, 1, 1
]
const vbo = gl.createBuffer() as WebGLBuffer;
gl.bindBuffer(gl.ARRAY_BUFFER, vbo)
gl.bufferData( gl.ARRAY_BUFFER, new Float32Array(attributes), gl.STATIC_DRAW)
gl.enableVertexAttribArray(inPosAttribLoc)
gl.vertexAttribPointer(inPosAttribLoc, 3, gl.FLOAT, false, 7*4, 0)
gl.enableVertexAttribArray(inColorAttribLoc)
gl.vertexAttribPointer(inColorAttribLoc, 4, gl.FLOAT, false, 7*4, 3*4)

window.addEventListener('resize', onWindowResize, false)
function onWindowResize() {
    const vp_size = [window.innerWidth, window.innerHeight]
    canvas.width = vp_size[0]
    canvas.height = vp_size[1]
}

function animate() {
    requestAnimationFrame(animate)
    render()
}

function ortho(width: number, height: number, near: number, far: number) {
    const fn = far + near;
    const f_n = far - near;
    const a = width/height;
    return [1/a, 0, 0, 0, 0, 1, 0, 0, 0, 0, -2/f_n, 0, 0, 0, -fn/f_n, 1];
}  

function render() {
    
    gl.viewport(0, 0, canvas.width, canvas.height);
    gl.clearColor(0, 0, 0, 1)
    gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT)
    gl.uniformMatrix4fv(projectionLocation, false, ortho(canvas.width, canvas.height, -1, 1));
    gl.drawArrays(gl.TRIANGLES, 0, 3)
}
animate()
