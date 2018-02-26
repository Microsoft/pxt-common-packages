namespace game {
    let isOver = false
    let _score: number = null
    let _waitAnyKey: () => void

    export function setWaitAnyKey(f: () => void) {
        _waitAnyKey = f
    }

    export function waitAnyKey() {
        if (_waitAnyKey) _waitAnyKey()
        else loops.pause(2000)
    }

    export function freeze() {
        sprite.setBackgroundCallback(() => { })
        loops.frame(() => { })
        sprite.reset()
    }

    function showBackground(h: number, c: number) {
        let top = (screen.height - h) / 2
        if (screen.isMono) {
            screen.fillRect(0, top, screen.width, h, 0)
            screen.drawLine(0, top, screen.width, top, c)
            screen.drawLine(0, top + h - 1, screen.width, top + h - 1, c)
        } else {
            screen.fillRect(0, top, screen.width, h, c)
        }
        return top
    }

    export function showDialog(name: string, content: string) {
        let lines = 1
        if (!content) lines = 0
        else
            for (let i = 0; i < content.length; ++i)
                if (content[i] == '\n') lines++

        let h = 28 + lines * (image.font5.charHeight + 2)
        let top = showBackground(h, 9)
        screen.print(name, 8, top + 8, 14, image.font8)
        screen.print(content, 8, top + 23, 13, image.font5)
    }

    export function splash(name: string, help: string) {
        showDialog(name, help)
        waitAnyKey()
    }

    export function meltScreen() {
        freeze()
        for (let i = 0; i < 10; ++i) {
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

    export function over(effect?: () => void) {
        if (isOver) return
        isOver = true
        control.clearHandlers()
        control.runInBackground(() => {
            if (effect) effect()
            let top = showBackground(44, 4)
            screen.printCenter("GAME OVER!", top + 8, 5, image.font8)
            if (hasScore())
                screen.printCenter("Score:" + game.score(), top + 23, 2, image.font5)
            if (!effect)
                loops.pause(1000) // wait for users to stop pressing keys
            waitAnyKey()
            control.reset()
        })
    }

    export function score() {
        initScore()
        return _score
    }

    export function hasScore() {
        return _score !== null
    }

    function initScore() {
        if (_score !== null) return
        _score = 0
        let font = image.font8
        let color = 15
        let maxW = 8
        control.addFrameHandler(95, () => {
            let s = _score + ""
            while (s.length < maxW) s = " " + s
            screen.print(s, screen.width - font.charWidth * maxW - 10, font.charHeight, color, font)
        })
    }

    export function setScore(score: number) {
        initScore()
        _score = score | 0
    }

    export function addToScore(points: number) {
        setScore(_score + points)
    }
}
