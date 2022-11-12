import Stats from 'three/examples/jsm/libs/stats.module' 
import { GUI } from 'dat.gui'

export class Statistic {
    public stats: Stats;

    constructor() {
        // @ts-ignore
        this.stats = new Stats();
        document.body.appendChild(this.stats.dom);
    }

    public update() {
        this.stats.update();
    }
}

export class DataGUI {
    public gui: GUI

    constructor(parameter?: any) {
        this.gui = new GUI(parameter);
    }
}