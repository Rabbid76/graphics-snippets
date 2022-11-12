export class ElapsedTime {
    private start?: number;
    private previousTimeStamp?: number;
    private currentTime?: number
    public allTimeMs: number = 0;
    public deltaTimeMs: number = 0;

    public update(timestamp: number) {
        this.currentTime = timestamp;
        if (this.start === undefined) {
            this.start = this.currentTime;
        }
        if (this.previousTimeStamp === undefined) {
            this.previousTimeStamp = this.currentTime;
        }
        this.allTimeMs = this.currentTime - this.start;
        this.deltaTimeMs = this.currentTime - this.previousTimeStamp;
        this.previousTimeStamp = this.currentTime
    }

    public getDegreePerSecond(angleDegree: number, delta: boolean): number {
        return angleDegree * Math.PI / 180 * (delta ? this.deltaTimeMs : this.allTimeMs) / 1000
    }
};