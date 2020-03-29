class Texture {
    constructor(gl, name, texUnit = 0, dflt = [128, 128, 128, 255]) {
        this.gl = gl;
        this.texUnit = texUnit;
        this.dflt = dflt;
        let texture = this;
        let image = { "cx": this.dflt.w || 1, "cy": this.dflt.h || 1, "plane": this.dflt.p || this.dflt };
        this.size = [image.cx, image.cy];
        this.dummyObj = this.createTexture2D(image, true);
        this.image = new Image(64, 64);
        this.image.setAttribute('crossorigin', 'anonymous');
        this.image.onload = function () {
            let cx = 1 << 31 - Math.clz32(texture.image.naturalWidth);
            if (cx < texture.image.naturalWidth)
                cx *= 2;
            let cy = 1 << 31 - Math.clz32(texture.image.naturalHeight);
            if (cy < texture.image.naturalHeight)
                cy *= 2;
            var canvas = document.createElement('canvas');
            canvas.width = cx;
            canvas.height = cy;
            var context = canvas.getContext('2d');
            context.drawImage(texture.image, 0, 0, canvas.width, canvas.height);
            texture.textureObj = texture.createTexture2D(canvas, true);
            texture.size = [cx, cy];
        };
        this.image.src = name;
    }
    delete() {
        this.gl.deleteTexture(this.textureObj);
    }
    createTexture2D(image, flipY) {
        let t = this.gl.createTexture();
        this.gl.activeTexture(this.gl.TEXTURE0 + this.texUnit);
        this.gl.bindTexture(this.gl.TEXTURE_2D, t);
        this.gl.pixelStorei(this.gl.UNPACK_FLIP_Y_WEBGL, flipY != undefined && flipY == true);
        if (image.cx && image.cy && image.plane)
            this.gl.texImage2D(this.gl.TEXTURE_2D, 0, this.gl.RGBA, image.cx, image.cy, 0, this.gl.RGBA, this.gl.UNSIGNED_BYTE, new Uint8Array(image.plane));
        else
            this.gl.texImage2D(this.gl.TEXTURE_2D, 0, this.gl.RGBA, this.gl.RGBA, this.gl.UNSIGNED_BYTE, image);
        this.gl.texParameteri(this.gl.TEXTURE_2D, this.gl.TEXTURE_MAG_FILTER, this.gl.LINEAR);
        this.gl.texParameteri(this.gl.TEXTURE_2D, this.gl.TEXTURE_MIN_FILTER, this.gl.LINEAR);
        this.gl.texParameteri(this.gl.TEXTURE_2D, this.gl.TEXTURE_WRAP_S, this.gl.REPEAT);
        this.gl.texParameteri(this.gl.TEXTURE_2D, this.gl.TEXTURE_WRAP_T, this.gl.REPEAT);
        this.gl.bindTexture(this.gl.TEXTURE_2D, null);
        this.gl.activeTexture(this.gl.TEXTURE0);
        return t;
    }
    bind(texUnit) {
        let unit = texUnit || this.texUnit;
        this.gl.activeTexture(this.gl.TEXTURE0 + unit);
        if (this.textureObj) {
            this.gl.bindTexture(this.gl.TEXTURE_2D, this.textureObj);
            return true;
        }
        this.gl.bindTexture(this.gl.TEXTURE_2D, this.dummyObj);
        return false;
    }
    bindDflt(texUnit) {
        this.gl.activeTexture(this.gl.TEXTURE0 + texUnit);
        this.gl.bindTexture(this.gl.TEXTURE_2D, this.dummyObj);
        return false;
    }
}
;
//# sourceMappingURL=Texture.js.map