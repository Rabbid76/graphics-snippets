class Camera {
    constructor(canvas, pos, target, up, fov_y, vp, near, far) {
        this.canvas = canvas;
        this.pos = pos;
        this.target = target;
        this.up = up;
        this.fov_y = fov_y;
        this.vp = vp;
        this.near = near;
        this.far = far;
        this.current_model_mat = Mat44.ident();
        this.model_mat = Mat44.ident();
        this.current_orbit_mat = Mat44.ident();
        this.orbit_mat = Mat44.ident();
        this.mouse_drag = false;
        this.auto_spin = false;
        this.auto_rotate = true;
        this.mouse_start = [0, 0];
        this.mouse_drag_axis = [0, 0, 0];
        this.mouse_drag_angle = 0;
        this.mouse_drag_time = 0;
        this.drag_start_T = this.rotate_start_T = this.time;
        this.domElement = document;
        let cam = this;
        this.domElement.addEventListener('mousedown', function (e) { cam.OnMouseDown(e); }, false);
        this.domElement.addEventListener('mouseup', function (e) { cam.OnMouseUp(e); }, false);
        this.domElement.addEventListener('mousemove', function (e) { cam.OnMouseMove(e); }, false);
        //this.domElement.addEventListener( 'contextmenu', function(e: any): void { event.preventDefault(); }, false );
        //this.domElement.addEventListener( 'mousewheel', hid_events.onMouseWheel, false );
        //this.domElement.addEventListener( 'DOMMouseScroll', hid_events.onMouseWheel, false ); // firefox
    }
    get time() { return Date.now(); }
    get ortho() {
        let fn = this.far + this.near;
        let f_n = this.far - this.near;
        let w = this.vp[0];
        let h = this.vp[1];
        return [2 / w, 0, 0, 0, 0, 2 / h, 0, 0, 0, 0, -2 / f_n, 0, 0, 0, -fn / f_n, 1];
    }
    get perspective() {
        let n = this.near;
        let f = this.far;
        let fn = f + n;
        let f_n = f - n;
        let r = this.vp[0] / this.vp[1];
        let t = 1 / Math.tan(Math.PI * this.fov_y / 360);
        return [t / r, 0, 0, 0, 0, t, 0, 0, 0, 0, -fn / f_n, -1, 0, 0, -2 * f * n / f_n, 0];
    }
    get lookAt() {
        let mz = Vec3.normalize([this.pos[0] - this.target[0], this.pos[1] - this.target[1], this.pos[2] - this.target[2]]);
        let mx = Vec3.normalize(Vec3.cross(this.up, mz));
        let my = Vec3.normalize(Vec3.cross(mz, mx));
        let tx = Vec3.dot(mx, this.pos);
        let ty = Vec3.dot(my, this.pos);
        let tz = Vec3.dot([-mz[0], -mz[1], -mz[2]], this.pos);
        return [mx[0], my[0], mz[0], 0, mx[1], my[1], mz[1], 0, mx[2], my[2], mz[2], 0, tx, ty, tz, 1];
    }
    get orbit() {
        return Mat44.multiply(this.lookAt, this.orbitMatrix);
    }
    get orbitMatrix() {
        return (this.mouse_drag || (this.auto_rotate && this.auto_spin)) ? Mat44.multiply(this.current_orbit_mat, this.orbit_mat) : this.orbit_mat;
    }
    get autoModelMatrix() {
        return this.auto_rotate ? Mat44.multiply(this.current_model_mat, this.model_mat) : this.model_mat;
    }
    Update(vp_size = null) {
        if (vp_size)
            this.vp = vp_size;
        let current_T = this.time;
        this.current_model_mat = Mat44.ident();
        if (this.mouse_drag) {
            this.current_orbit_mat = Mat44.rotate(Mat44.ident(), this.mouse_drag_angle, this.mouse_drag_axis);
        }
        else if (this.auto_rotate) {
            if (this.auto_spin) {
                if (this.mouse_drag_time > 0) {
                    let angle = this.mouse_drag_angle * (current_T - this.rotate_start_T) / this.mouse_drag_time;
                    this.current_orbit_mat = Mat44.rotate(Mat44.ident(), angle, this.mouse_drag_axis);
                }
            }
            else {
                let auto_angle_x = Util.fract((current_T - this.rotate_start_T) / 13000.0) * 2.0 * Math.PI;
                let auto_angle_y = Util.fract((current_T - this.rotate_start_T) / 17000.0) * 2.0 * Math.PI;
                this.current_model_mat = Mat44.rotateAxis(this.current_model_mat, auto_angle_x, 0);
                this.current_model_mat = Mat44.rotateAxis(this.current_model_mat, auto_angle_y, 1);
            }
        }
    }
    ChangeMotionMode(drag, spin, auto) {
        let new_drag = drag;
        let new_auto = new_drag ? false : auto;
        let new_spin = new_auto ? spin : false;
        let change = this.mouse_drag != new_drag || this.auto_rotate != new_auto || this.auto_spin != new_spin;
        if (!change)
            return;
        if (new_drag && !this.mouse_drag) {
            this.drag_start_T = this.time;
            this.mouse_drag_angle = 0.0;
            this.mouse_drag_time = 0;
        }
        if (new_auto && !this.auto_rotate)
            this.rotate_start_T = this.time;
        this.mouse_drag = new_drag;
        this.auto_rotate = new_auto;
        this.auto_spin = new_spin;
        this.orbit_mat = Mat44.multiply(this.current_orbit_mat, this.orbit_mat);
        this.current_orbit_mat = Mat44.ident();
        this.model_mat = Mat44.multiply(this.current_model_mat, this.model_mat);
        this.current_model_mat = Mat44.ident();
    }
    OnMouseDown(event) {
        let rect = this.canvas.getBoundingClientRect();
        if (event.clientX < rect.left || event.clientX > rect.right)
            return;
        if (event.clientY < rect.top || event.clientY > rect.bottom)
            return;
        if (event.button == 0) { // left button
            this.mouse_start = [event.clientX, event.clientY];
            this.ChangeMotionMode(true, false, false);
        }
    }
    OnMouseUp(event) {
        if (event.button == 0) { // left button
            this.ChangeMotionMode(false, true, true);
        }
        else if (event.button == 1) { // middle button
            this.ChangeMotionMode(false, false, !this.auto_rotate);
        }
    }
    OnMouseMove(event) {
        let dx = (event.clientX - this.mouse_start[0]) / this.vp[0];
        let dy = (event.clientY - this.mouse_start[1]) / this.vp[1];
        let len = Math.sqrt(dx * dx + dy * dy);
        if (this.mouse_drag && len > 0) {
            this.mouse_drag_angle = Math.PI * len;
            this.mouse_drag_axis = [-dy / len, 0, dx / len];
            this.mouse_drag_time = this.time - this.drag_start_T;
        }
    }
}
//# sourceMappingURL=Camera.js.map