import {
    ColorSpace,
    CubeTexture,
    DataTexture,
    FileLoader,
    LinearSRGBColorSpace,
    LinearFilter,
    LinearMipMapLinearFilter,
    RGBAFormat,
    TextureDataType,
    UnsignedByteType,
    Vector3,
    SRGBColorSpace
} from 'three';

const _HEADER_BYTES_ = 30;

export class EnvMapReader {
    private _cubeTextures: any[];
    private _lightDirection: any;
    private _size: number = 256;
    private _maxLod: number = 0;
    private _colorSpace: ColorSpace = SRGBColorSpace;
    private _type: number = UnsignedByteType;
    private _format: number = RGBAFormat;
    private _cubeTexture: any;
    private _path: string;

    constructor() {
        this._cubeTextures = [];
        this._lightDirection = new Vector3();
        this._path = '';
    }

    public setPath(path: string) {
        this._path = path;
    }

    public load(fileName: string) {
        return new Promise((resolve) => {
            new FileLoader()
                .setResponseType('arraybuffer')
                .load(this._path + fileName, (buffer: any) => {
                    this.parseData_(buffer);
                    resolve(this._cubeTexture);
                });
        });
    }

    private parseHeader_(buffer: ArrayBuffer) {
        let view = new DataView(buffer, 0, _HEADER_BYTES_);
        this._size = view.getUint16(0);
        this._maxLod = view.getUint8(2);
        this._colorSpace = view.getUint16(3) ? LinearSRGBColorSpace : SRGBColorSpace;
        this._type = view.getUint16(5);
        this._format = view.getUint16(7);
        this._lightDirection.x = view.getFloat32(9);
        this._lightDirection.y = view.getFloat32(13);
        this._lightDirection.z = view.getFloat32(17);
    }

    private parseBufferData_(buffer: ArrayBuffer, offsetBytes: number) {
        let size = this._size;
        let cpp = 4;
        const numMips = Math.log2(this._size) + 1;
        this._cubeTextures = [];

        for (let i = 0; i < numMips; i++) {
            let cubeTexture = new CubeTexture();
            this._cubeTextures.push(cubeTexture);
            cubeTexture.format = RGBAFormat;
            cubeTexture.colorSpace = this._colorSpace;
            cubeTexture.type = this._type as TextureDataType;
            cubeTexture.minFilter = LinearMipMapLinearFilter;
            cubeTexture.magFilter = LinearFilter;
            cubeTexture.generateMipmaps = false;

            for (let j = 0; j < 6; j++) {
                let view = new Uint8Array(buffer, offsetBytes, cpp * size * size);
                let dataTexture = new DataTexture(view, size, size);
                dataTexture.format = cubeTexture.format;
                dataTexture.colorSpace = cubeTexture.colorSpace;
                dataTexture.type = cubeTexture.type;
                dataTexture.generateMipmaps = false;

                offsetBytes += cpp * size * size;
                cubeTexture.images[j] = dataTexture;
                dataTexture.needsUpdate = true;
            }

            cubeTexture.needsUpdate = true;
            size /= 2;
        }

        this._cubeTexture = new CubeTexture();
        this._cubeTexture.format = RGBAFormat;
        this._cubeTexture.colorSpace = this._colorSpace;
        this._cubeTexture.type = this._type;
        this._cubeTexture.minFilter = LinearMipMapLinearFilter;
        this._cubeTexture.magFilter = LinearFilter;
        this._cubeTexture.generateMipmaps = false;

        for (let i = 0; i < 6; i++) {
            this._cubeTexture.image[i] = this._cubeTextures[0].images[i];
            for (let m = 1; m < numMips; m++) {
                this._cubeTexture.mipmaps[m - 1] = this._cubeTextures[m];
                this._cubeTextures[m].needsUpdate = true;
            }
        }
        this._cubeTexture.needsUpdate = true;
        this._cubeTexture.maxLod = this._maxLod - 1;
    }

    private convertToRGBABuffer(buffer: ArrayBuffer) {
        let view = new Uint8Array(buffer, _HEADER_BYTES_, buffer.byteLength - _HEADER_BYTES_);
        const stride = this._format === 1022 ? 3 : 4;
        const newBufferLength = 4 * view.length / stride;
        let newBuffer = new ArrayBuffer(newBufferLength);
        let newView = new DataView(newBuffer);
        let n = 0;
        let offset = view.length / stride;
        if (stride === 3) {
            for (let i = 0; i < offset; i++) {
                newView.setUint8(n++, view[i]);
                newView.setUint8(n++, view[i + offset]);
                newView.setUint8(n++, view[i + 2 * offset]);
                newView.setUint8(n++, 255);
            }
        } else {
            for (let i = 0; i < offset; i++) {
                newView.setUint8(n++, view[i]);
                newView.setUint8(n++, view[i + offset]);
                newView.setUint8(n++, view[i + 2 * offset]);
                newView.setUint8(n++, view[i + 3 * offset]);
            }
        }

        return newBuffer;
    }

    private parseData_(buffer: ArrayBuffer) {
        this.parseHeader_(buffer);
        let offsetByes = _HEADER_BYTES_;
        offsetByes = 0;
        let newBuffer = this.convertToRGBABuffer(buffer);
        this.parseBufferData_(newBuffer, offsetByes);
    }
}
