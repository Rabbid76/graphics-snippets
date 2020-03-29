let Util = {
    fract: function (val) {
        return val - Math.trunc(val);
    },
    angle: function (deltaTime, intervall) {
        return this.fract(deltaTime / (1000 * intervall)) * 2.0 * Math.PI;
    },
    move: function (deltaTime, intervall, range) {
        let pos = this.fract(deltaTime / (1000 * intervall)) * 2.0;
        pos = pos < 1.0 ? pos : (2.0 - pos);
        return range[0] + (range[1] - range[0]) * pos;
    },
    ellipticalPosition: function (a, b, angRag) {
        let a_b = a * a - b * b;
        let ea = (a_b <= 0) ? 0 : Math.sqrt(a_b);
        let eb = (a_b >= 0) ? 0 : Math.sqrt(-a_b);
        return [a * Math.sin(angRag) - ea, b * Math.cos(angRag) - eb, 0];
    }
};
let Vec3 = {
    cross: function (a, b) { return [a[1] * b[2] - a[2] * b[1], a[2] * b[0] - a[0] * b[2], a[0] * b[1] - a[1] * b[0], 0.0]; },
    dot: function (a, b) { return a[0] * b[0] + a[1] * b[1] + a[2] * b[2]; },
    normalize: function (v) {
        var len = Math.sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
        return [v[0] / len, v[1] / len, v[2] / len];
    }
};
let Mat44 = {
    ident: function () { return [1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1]; },
    rotateAxis: function (matA, angRad, axis) {
        var aMap = [[1, 2], [2, 0], [0, 1]];
        var a0 = aMap[axis][0], a1 = aMap[axis][1];
        var sinAng = Math.sin(angRad), cosAng = Math.cos(angRad);
        var matB = matA.slice(0);
        for (var i = 0; i < 3; ++i) {
            matB[a0 * 4 + i] = matA[a0 * 4 + i] * cosAng + matA[a1 * 4 + i] * sinAng;
            matB[a1 * 4 + i] = matA[a0 * 4 + i] * -sinAng + matA[a1 * 4 + i] * cosAng;
        }
        return matB;
    },
    rotate: function (matA, angRad, axis) {
        let s = Math.sin(-angRad), c = Math.cos(-angRad);
        let x = axis[0], y = axis[1], z = axis[2];
        let matB = [
            x * x * (1 - c) + c, x * y * (1 - c) - z * s, x * z * (1 - c) + y * s, 0,
            y * x * (1 - c) + z * s, y * y * (1 - c) + c, y * z * (1 - c) - x * s, 0,
            z * x * (1 - c) - y * s, z * y * (1 - c) + x * s, z * z * (1 - c) + c, 0,
            0, 0, 0, 1
        ];
        return this.multiply(matA, matB);
    },
    multiply: function (matA, matB) {
        let matC = this.ident();
        for (var i0 = 0; i0 < 4; ++i0)
            for (var i1 = 0; i1 < 4; ++i1)
                matC[i0 * 4 + i1] = matB[i0 * 4 + 0] * matA[0 * 4 + i1] + matB[i0 * 4 + 1] * matA[1 * 4 + i1] + matB[i0 * 4 + 2] * matA[2 * 4 + i1] + matB[i0 * 4 + 3] * matA[3 * 4 + i1];
        return matC;
    }
};
//# sourceMappingURL=MathUtility.js.map