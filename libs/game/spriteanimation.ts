//%
class SpriteAnimation {
    frames: Image[]
    frameIdx: number
    time: number
    step: number

    constructor(f: Image[]) {
        this.frames = f
        this.frameIdx = 0
        this.time = 0
        this.step = 0.2
    }

    reset() {
        this.frameIdx = 0
        this.time = 0
    }

    update(parent: Sprite, dt: number) {
        this.time += dt;
        let f = (this.time / this.step) | 0
        if (f != this.frameIdx) {
            if (f >= this.frames.length)
                this.reset()
            else
                this.frameIdx = f
            parent.setImage(this.frames[this.frameIdx]);
        }
    }
}