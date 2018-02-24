namespace game {
    let isOver = false
    let _score = 0
    let scoreSprite: Sprite

    export function meltScreen() {
        control.clearHandlers()
        for (let i = 0; i < 50; ++i) {
            for (let j = 0; j < 1000; ++j) {
                let x = Math.randomRange(0, screen.width - 1)
                let y = Math.randomRange(0, screen.height - 3)
                let c = screen.get(x, y)
                screen.set(x, y + 1, c)
                screen.set(x, y + 2, c)
            }
            loops.pause(100)
        }
    }

    export function over() {
        if (isOver) return
        isOver = true
        control.clearHandlers()
        control.runInBackground(() => {
            let font = image.doubledFont(image.defaultFont)
            for (let i = 0; i < 40; ++i) {
                screen.print("GAME\nOVER", 30, 50, Math.randomRange(1, 15), font)
                loops.pause(40)
            }
            control.reset()
        })
    }

    export function score() {
        return _score
    }

    function initScore() {
        if (scoreSprite) return
        let font = image.defaultFont
        let color = 15
        let maxW = 8
        scoreSprite = sprite.create(image.create(font.charWidth * maxW, font.charHeight))
        scoreSprite.x = screen.width - font.charWidth * maxW / 2 - 10
        scoreSprite.y = font.charHeight
        scoreSprite.makeGhost()
        scoreSprite.z = 1000
        control.addFrameHandler(85, () => {
            let s = _score + ""
            while (s.length < maxW) s = " " + s
            scoreSprite.image.fill(0)
            scoreSprite.image.print(s, 0, 0, color, font)
        })
    }

    export function setScore(score: number) {
        initScore()
        _score = score
    }

    export function addToScore(points: number) {
        setScore(_score + points)
    }
}
