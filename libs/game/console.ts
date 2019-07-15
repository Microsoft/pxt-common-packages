namespace game.consoleOverlay {
    let consoleColor = 1;
    let consoleStrings: string[];
    const marginx = 4;
    const marginy = 2;
    const consoleFont = image.font5;
    const MAX_CONSOLE_LINES = Math.floor(screen.height / (consoleFont.charHeight + marginy)) - 1;
    console.addListener(listener);

    export function isVisible() {
        return !!consoleStrings;
    }

    export function clear() {
        consoleStrings = [];
    }

    export function setVisible(value: boolean, col?: number) {
        if (value != !!consoleStrings)
            consoleStrings = value ? [] : undefined;
        if (col !== undefined)
            consoleColor = col;
    }

    function listener(priority: ConsolePriority, text: string) {
        if (!consoleStrings)
            return;

        text.split("\n")
            .filter(line => !!line)
            .forEach(line => consoleStrings.push(line));

        if (consoleStrings.length > MAX_CONSOLE_LINES) {
            consoleStrings.splice(0, consoleStrings.length - MAX_CONSOLE_LINES);
        }
    }

    export function draw() {
        if (!consoleStrings || scene.systemMenu.isVisible()) return;
        const height = consoleFont.charHeight + marginy;
        const top = 2 + (game.stats ? height : 0);
        for (let i = 0; i < consoleStrings.length; ++i) {
            const t = consoleStrings[i];
            screen.print(t, marginx, top + i * height, consoleColor, consoleFont);
        }
    }
}