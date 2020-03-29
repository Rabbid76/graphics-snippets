class ShProg {
    // TODO $$$ delete shader and program
    constructor(gl, shaderList) {
        this.gl = gl;
        let shaderObjs = [];
        for (let i_sh = 0; i_sh < shaderList.length; ++i_sh) {
            let shderObj = this.Compile(shaderList[i_sh].source, shaderList[i_sh].stage);
            if (shderObj)
                shaderObjs.push(shderObj);
        }
        this.progObj = this.Link(shaderObjs);
        if (this.progObj) {
            this.attrInx = {};
            var noOfAttributes = this.gl.getProgramParameter(this.progObj, this.gl.ACTIVE_ATTRIBUTES);
            for (var i_n = 0; i_n < noOfAttributes; ++i_n) {
                var name = this.gl.getActiveAttrib(this.progObj, i_n).name;
                this.attrInx[name] = this.gl.getAttribLocation(this.progObj, name);
            }
            this.uniLoc = {};
            var noOfUniforms = this.gl.getProgramParameter(this.progObj, this.gl.ACTIVE_UNIFORMS);
            for (var i_n = 0; i_n < noOfUniforms; ++i_n) {
                var name = this.gl.getActiveUniform(this.progObj, i_n).name;
                this.uniLoc[name] = this.gl.getUniformLocation(this.progObj, name);
            }
        }
    }
    AttrI(name) { return this.attrInx[name]; }
    UniformL(name) { return this.uniLoc[name]; }
    Use() { this.gl.useProgram(this.progObj); }
    SetI1(name, val) { if (this.uniLoc[name])
        this.gl.uniform1i(this.uniLoc[name], val); }
    SetF1(name, val) { if (this.uniLoc[name])
        this.gl.uniform1f(this.uniLoc[name], val); }
    SetF2(name, arr) { if (this.uniLoc[name])
        this.gl.uniform2fv(this.uniLoc[name], arr); }
    SetF3(name, arr) { if (this.uniLoc[name])
        this.gl.uniform3fv(this.uniLoc[name], arr); }
    SetF4(name, arr) { if (this.uniLoc[name])
        this.gl.uniform4fv(this.uniLoc[name], arr); }
    SetM33(name, mat) { if (this.uniLoc[name])
        this.gl.uniformMatrix3fv(this.uniLoc[name], false, mat); }
    SetM44(name, mat) { if (this.uniLoc[name])
        this.gl.uniformMatrix4fv(this.uniLoc[name], false, mat); }
    Compile(source, shaderStage) {
        let shaderScript = document.getElementById(source);
        if (shaderScript)
            source = shaderScript.text;
        let shaderObj = this.gl.createShader(shaderStage);
        this.gl.shaderSource(shaderObj, source);
        this.gl.compileShader(shaderObj);
        let status = this.gl.getShaderParameter(shaderObj, this.gl.COMPILE_STATUS);
        if (!status)
            alert(this.gl.getShaderInfoLog(shaderObj));
        return status ? shaderObj : null;
    }
    Link(shaderObjs) {
        let prog = this.gl.createProgram();
        for (let i_sh = 0; i_sh < shaderObjs.length; ++i_sh)
            this.gl.attachShader(prog, shaderObjs[i_sh]);
        this.gl.linkProgram(prog);
        status = this.gl.getProgramParameter(prog, this.gl.LINK_STATUS);
        if (!status)
            alert(this.gl.getProgramInfoLog(prog));
        return status ? prog : null;
    }
}
//# sourceMappingURL=ShaderProgram.js.map