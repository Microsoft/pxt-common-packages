namespace game {
    export interface PromptTheme {
        colorPrompt: number;
        colorInput: number;
        colorInputHighlighted: number;
        colorInputText: number;
        colorAlphabet: number;
        colorCursor: number;
        colorBackground: number;
        colorBottomBackground: number;
        colorBottomText: number;
    }

    /**
     * Ask the player for a string value.
     * @param message The message to display on the text-entry screen
     * @param answerLength The maximum number of characters the user can enter (1 - 24)
     * @param timeOut The time till the keyboard will time out. You can enter in seconds (0 - 99).
     */
    //% group="Gameplay"
    //% weight=10 help=game/ask-for-string
    //% blockId=gameaskforstring block="ask for string %message || and max length %answerLength and timeout after %timeOut"
    //% message.defl=""
    //% answerLength.defl="12"
    //% answerLength.min=1
    //% answerLength.max=24
    //% timeOut.defl=0
    //% timeOut.min=0
    //% timeOut.max=99
    //% group="Prompt"
    //% expandableArgumentMode="enabled"
    export function askForString(message: string, answerLength = 12, timeOut: number = 0) {
        let p = new game.Prompt();
        const result = p.show(message, answerLength, timeOut * 1000);
        return result;
    }


    //% whenUsed=true
    const font = image.font8; // FONT8-TODO
    //% whenUsed=true
    const PADDING = 4;
    //% whenUsed=true
    const PROMPT_LINE_SPACING = 2;

    //% whenUsed=true
    const NUM_LETTERS = 26;
    //% whenUsed=true
    const ALPHABET_ROW_LENGTH = 12;
    //% whenUsed=true
    const NUM_ROWS = Math.ceil(NUM_LETTERS / ALPHABET_ROW_LENGTH);
    //% whenUsed=true
    const INPUT_ROWS = 2;

    //% whenUsed=true
    const CONTENT_WIDTH = screen.width - PADDING * 2;
    //% whenUsed=true
    const CONTENT_HEIGHT = screen.height - PADDING * 2;
    //% whenUsed=true
    const CONTENT_TOP = PADDING;

    // Dimensions of a "cell" that contains a letter
    //% whenUsed=true
    const CELL_WIDTH = Math.floor(CONTENT_WIDTH / ALPHABET_ROW_LENGTH);
    //% whenUsed=true
    const CELL_HEIGHT = CELL_WIDTH;
    //% whenUsed=true
    const LETTER_OFFSET_X = Math.floor((CELL_WIDTH - font.charWidth) / 2);
    //% whenUsed=true
    const LETTER_OFFSET_Y = Math.floor((CELL_HEIGHT - font.charHeight) / 2);
    //% whenUsed=true
    const BLANK_PADDING = 1;
    //% whenUsed=true
    const ROW_LEFT = PADDING + CELL_WIDTH / 2 + Math.floor((CONTENT_WIDTH - (CELL_WIDTH * ALPHABET_ROW_LENGTH)) / 2);

    // Dimensions of the bottom bar
    //% whenUsed=true
    const BOTTOM_BAR_ALPHABET_MARGIN = 4;
    //% whenUsed=true
    const BOTTOM_BAR_HEIGHT = PADDING + BOTTOM_BAR_ALPHABET_MARGIN + CELL_HEIGHT;
    //% whenUsed=true
    const BOTTOM_BAR_TOP = screen.height - BOTTOM_BAR_HEIGHT;
    //% whenUsed=true
    const BOTTOM_BAR_BUTTON_WIDTH = PADDING * 2 + font.charWidth * 3;
    //% whenUsed=true
    const BOTTOM_BAR_TEXT_Y = (BOTTOM_BAR_HEIGHT - font.charHeight) / 2;
    //% whenUsed=true
    const BOTTOM_BAR_SHIFT_X = (BOTTOM_BAR_BUTTON_WIDTH - font.charWidth * 3) / 2;
    //% whenUsed=true
    const BOTTOM_BAR_CONFIRM_X = (BOTTOM_BAR_BUTTON_WIDTH - font.charWidth * 2) / 2;
    //% whenUsed=true
    const CONFIRM_BUTTON_LEFT = screen.width - BOTTOM_BAR_BUTTON_WIDTH;

    // Dimensions of the alphabet area
    //% whenUsed=true
    const ALPHABET_HEIGHT = NUM_ROWS * CELL_HEIGHT;
    //% whenUsed=true
    const ALPHABET_TOP = CONTENT_TOP + CONTENT_HEIGHT - ALPHABET_HEIGHT - BOTTOM_BAR_HEIGHT;
    //% whenUsed=true
    const ALPHABET_INPUT_MARGIN = 10;

    // Dimensions of area where text is input
    //% whenUsed=true
    const INPUT_HEIGHT = INPUT_ROWS * CELL_HEIGHT;
    //% whenUsed=true
    const INPUT_TOP = ALPHABET_TOP - INPUT_HEIGHT - ALPHABET_INPUT_MARGIN;

    // Dimensions of prompt message area
    //% whenUsed=true
    const PROMPT_HEIGHT = INPUT_TOP - CONTENT_TOP;

    //% whenUsed=true
    const lowerShiftText = "ABC";
    //% whenUsed=true
    const upperShiftText = "abc";
    //% whenUsed=true
    const digitsUpper = [" ", ",", ".", "?", "!", ":", ";", "\"", "(", ")"];
    //% whenUsed=true
    const confirmText = "OK";


    export class Prompt {
        theme: PromptTheme;

        message: string;
        answerLength: number;
        result: string;

        private cursor: Sprite;
        private shiftButton: Sprite;
        private confirmButton: Sprite;

        private letters: Sprite[];
        private inputs: Sprite[];

        private confirmPressed: boolean;
        private cursorRow: number;
        private cursorColumn: number;
        private upper: boolean;
        private inputIndex: number;
        private blink: boolean;
        private frameCount: number;
        private timerEnd: number;
        private timerSprite: Sprite;

        constructor(theme?: PromptTheme) {
            if (theme) {
                this.theme = theme;
            }
            else {
                this.theme = {
                    colorPrompt: 1,
                    colorInput: 3,
                    colorInputHighlighted: 5,
                    colorInputText: 1,
                    colorAlphabet: 1,
                    colorCursor: 7,
                    colorBackground: 15,
                    colorBottomBackground: 3,
                    colorBottomText: 1,
                };
            }
            this.cursorRow = 0;
            this.cursorColumn = 0;
            this.upper = false;
            this.inputIndex = 0;
        }

        show(message: string, answerLength: number, timeOut: number) {
            this.message = message;
            this.answerLength = answerLength;
            this.inputIndex = 0;

            if (timeOut != 0) {
                this.timerEnd = game.currentScene().millis() + timeOut;
            }

            controller._setUserEventsEnabled(false);
            game.pushScene()

            this.draw();
            this.registerHandlers();
            this.confirmPressed = false;

            pauseUntil(() => this.confirmPressed, timeOut);

            game.popScene();
            controller._setUserEventsEnabled(true);

            if(this.confirmPressed) {
                return this.result;
            }
            else {
                return null;
            }
        }

        private draw() {
            this.drawPromptText();
            this.drawKeyboard();
            this.drawInputarea();
            this.drawBottomBar();
        }

        private drawTimerCheck() {
            if(this.timerEnd !== undefined) {
                this.drawTimer(this.timerEnd - game.currentScene().millis());
            }
        }

        private drawTimer(millis: number) {
            if (millis < 0) millis = 0;
            millis |= 0;
    
            const seconds = Math.idiv(millis, 1000) + 1;
            const secondsString = seconds.toString()

            if(this.timerSprite === undefined) {
                const letter = image.create(CELL_WIDTH, CELL_HEIGHT);

                this.timerSprite = sprites.create(letter, -1);

                // Set to the top right corner of the screen
                this.timerSprite.setPosition(155, 5);
            }

            this.timerSprite.image.fill(3);

            if(secondsString.length != 1) {
                this.timerSprite.image.print(secondsString, 0, LETTER_OFFSET_Y);
            }
            else {
                this.timerSprite.image.print(secondsString, LETTER_OFFSET_X, LETTER_OFFSET_Y);
            }
        }

        private drawPromptText() {
            const prompt = sprites.create(layoutText(this.message, CONTENT_WIDTH, PROMPT_HEIGHT, this.theme.colorPrompt), -1);
            prompt.x = screen.width / 2
            prompt.y = CONTENT_TOP + Math.floor((PROMPT_HEIGHT - prompt.height) / 2) + Math.floor(prompt.height / 2);
        }

        private drawInputarea() {
            const answerLeft = ROW_LEFT + Math.floor(
                ((CELL_WIDTH * ALPHABET_ROW_LENGTH) -
                    CELL_WIDTH * Math.min(this.answerLength, ALPHABET_ROW_LENGTH)) / 2);

            this.inputs = [];
            for (let i = 0; i < this.answerLength; i++) {
                const blank = image.create(CELL_WIDTH, CELL_HEIGHT);
                this.drawInput(blank, "", this.theme.colorInput);

                const col = i % ALPHABET_ROW_LENGTH;
                const row = Math.floor(i / ALPHABET_ROW_LENGTH);

                const s = sprites.create(blank, -1);
                s.x = answerLeft + col * CELL_WIDTH;
                s.y = INPUT_TOP + row * CELL_HEIGHT;
                this.inputs.push(s);
            }
        }

        private drawKeyboard() {
            const cursorImage = image.create(CELL_WIDTH, CELL_HEIGHT);
            cursorImage.fill(this.theme.colorCursor);
            this.cursor = sprites.create(cursorImage, -1);
            this.cursor.z = -1;
            this.updateCursor();

            this.letters = [];
            for (let j = 0; j < 36; j++) {
                const letter = image.create(CELL_WIDTH, CELL_HEIGHT);

                const col2 = j % ALPHABET_ROW_LENGTH;
                const row2 = Math.floor(j / ALPHABET_ROW_LENGTH);

                const t = sprites.create(letter, -1);
                t.x = ROW_LEFT + col2 * CELL_WIDTH;
                t.y = ALPHABET_TOP + row2 * CELL_HEIGHT;

                this.letters.push(t);
            }
            this.updateKeyboard();
        }

        private drawBottomBar() {
            const bg = image.create(screen.width, BOTTOM_BAR_HEIGHT);
            bg.fill(this.theme.colorBottomBackground);

            const bgSprite = sprites.create(bg, -1);
            bgSprite.x = screen.width / 2;
            bgSprite.y = BOTTOM_BAR_TOP + BOTTOM_BAR_HEIGHT / 2;
            bgSprite.z = -1;

            this.shiftButton = sprites.create(image.create(BOTTOM_BAR_BUTTON_WIDTH, BOTTOM_BAR_HEIGHT), -1);
            this.shiftButton.x = Math.floor(BOTTOM_BAR_BUTTON_WIDTH / 2);
            this.shiftButton.y = BOTTOM_BAR_TOP + Math.ceil(BOTTOM_BAR_HEIGHT / 2);

            this.confirmButton = sprites.create(image.create(BOTTOM_BAR_BUTTON_WIDTH, BOTTOM_BAR_HEIGHT), -1);
            this.confirmButton.x = CONFIRM_BUTTON_LEFT + Math.floor(BOTTOM_BAR_BUTTON_WIDTH / 2);
            this.confirmButton.y = BOTTOM_BAR_TOP + Math.ceil(BOTTOM_BAR_HEIGHT / 2);

            this.updateButtons();
        }

        private updateButtons() {
            if (this.cursorRow === 3 && this.cursorColumn % 2 !== 1) {
                this.shiftButton.image.fill(this.theme.colorCursor);
            }
            else {
                this.shiftButton.image.fill(this.theme.colorBottomBackground);
            }

            if (this.upper) {
                this.shiftButton.image.print(upperShiftText, BOTTOM_BAR_SHIFT_X, BOTTOM_BAR_TEXT_Y);
            }
            else {
                this.shiftButton.image.print(lowerShiftText, BOTTOM_BAR_SHIFT_X, BOTTOM_BAR_TEXT_Y);
            }


            if (this.cursorRow === 3 && this.cursorColumn % 2) {
                this.confirmButton.image.fill(this.theme.colorCursor);
            }
            else {
                this.confirmButton.image.fill(this.theme.colorBottomBackground);
            }

            this.confirmButton.image.print(confirmText, BOTTOM_BAR_CONFIRM_X, BOTTOM_BAR_TEXT_Y);
        }

        private updateCursor() {
            if (this.cursorRow === 3) {
                this.cursor.image.fill(0);
                this.updateButtons();
            }
            else {
                this.cursor.x = ROW_LEFT + this.cursorColumn * CELL_WIDTH;
                this.cursor.y = ALPHABET_TOP + this.cursorRow * CELL_HEIGHT;
            }
        }

        private updateSelectedInput() {
            if (this.inputIndex < this.answerLength) {
                const u = this.inputs[this.inputIndex];
                if (this.blink) {
                    this.drawInput(u.image, "", this.theme.colorInput);
                }
                else {
                    this.drawInput(u.image, "", this.theme.colorInputHighlighted)
                }
            }
        }

        private updateKeyboard() {
            const len = this.letters.length;
            for (let k = 0; k < len; k++) {
                const img = this.letters[k].image;
                img.fill(0);
                img.print(getCharForIndex(k, this.upper), LETTER_OFFSET_X, LETTER_OFFSET_Y);
            }
        }

        private drawInput(img: Image, char: string, color: number) {
            img.fill(0);
            img.fillRect(BLANK_PADDING, CELL_HEIGHT - 1, CELL_WIDTH - BLANK_PADDING * 2, 1, color)

            if (char) {
                img.print(char, LETTER_OFFSET_X, LETTER_OFFSET_Y, this.theme.colorInputText, font);
            }
        }

        private registerHandlers() {
            controller.up.onEvent(SYSTEM_KEY_DOWN, () => {
                this.moveVertical(true);
            })

            controller.down.onEvent(SYSTEM_KEY_DOWN, () => {
                this.moveVertical(false);
            })

            controller.right.onEvent(SYSTEM_KEY_DOWN, () => {
                this.moveHorizontal(true);
            });

            controller.left.onEvent(SYSTEM_KEY_DOWN, () => {
                this.moveHorizontal(false);
            });

            controller.A.onEvent(SYSTEM_KEY_DOWN, () => {
                this.confirm();
            });

            controller.B.onEvent(SYSTEM_KEY_DOWN, () => {
                this.delete();
            });


            this.frameCount = 0;
            this.blink = true;

            game.onUpdate(() => {
                this.frameCount = (this.frameCount + 1) % 30;

                if (this.frameCount === 0) {
                    this.blink = !this.blink;

                    this.updateSelectedInput();
                }

                this.drawTimerCheck()
            })
        }

        private moveVertical(up: boolean) {
            if (up) {
                if (this.cursorRow === 3) {
                    this.cursor.image.fill(this.theme.colorCursor);
                    this.cursorRow = 2;

                    if (this.cursorColumn % 2) {
                        this.cursorColumn = ALPHABET_ROW_LENGTH - 1;
                    }
                    else {
                        this.cursorColumn = 0;
                    }

                    this.updateButtons();
                }
                else {
                    this.cursorRow = Math.max(0, this.cursorRow - 1);
                }
            }
            else {
                this.cursorRow = Math.min(3, this.cursorRow + 1);

                if (this.cursorRow === 3) {
                    // Go to closest button
                    this.cursorColumn = this.cursorColumn > 5 ? 1 : 0;
                }
            }

            this.updateCursor();
        }

        private moveHorizontal(right: boolean) {
            if (right) {
                this.cursorColumn = (this.cursorColumn + 1) % ALPHABET_ROW_LENGTH;
            }
            else {
                this.cursorColumn = (this.cursorColumn + (ALPHABET_ROW_LENGTH - 1)) % ALPHABET_ROW_LENGTH;
            }

            this.updateCursor();
        }

        private confirm() {
            if (this.cursorRow === 3) {
                if (this.cursorColumn % 2) {
                    this.confirmPressed = true;
                }
                else {
                    this.upper = !this.upper;
                    this.updateKeyboard();
                    this.updateButtons();
                }
            }
            else {
                if (this.inputIndex >= this.answerLength) return;

                const index = this.cursorColumn + this.cursorRow * ALPHABET_ROW_LENGTH
                const letter = getCharForIndex(index, this.upper);

                if (!this.result) {
                    this.result = letter;
                }
                else {
                    this.result += letter;
                }

                const sprite = this.inputs[this.inputIndex];
                this.changeInputIndex(1);
                this.drawInput(sprite.image, letter, this.theme.colorInput);
            }
        }

        private delete() {
            if (this.inputIndex <= 0) return;

            if (this.inputIndex < this.answerLength) {
                this.drawInput(this.inputs[this.inputIndex].image, "", this.theme.colorInput);
            }

            this.result = this.result.substr(0, this.result.length - 1);

            this.changeInputIndex(-1);
        }

        private changeInputIndex(delta: number) {
            this.inputIndex += delta;
            this.frameCount = 0
            this.blink = false;
            this.updateSelectedInput();
        }
    }

    function layoutText(message: string, width: number, height: number, color: number) {
        const lineHeight = font.charHeight + PROMPT_LINE_SPACING;

        const lineLength = Math.floor(width / font.charWidth);
        const numLines = Math.floor(height / lineHeight);

        let lines: string[] = [];
        let word: string;
        let line: string;

        let pushWord = () => {
            if (line) {
                if (line.length + word.length + 1 > lineLength) {
                    lines.push(line);
                    line = word;
                }
                else {
                    line = line + " " + word;
                }
            }
            else {
                line = word;
            }

            word = null;
        }

        for (let l = 0; l < message.length; l++) {
            const char = message.charAt(l);

            if (char === " ") {
                if (word) {
                    pushWord();
                }
                else {
                    word = " ";
                }
            }
            else if (!word) {
                word = char;
            }
            else {
                word += char;
            }
        }

        if (word) {
            pushWord();
        }

        if (line) {
            lines.push(line);
        }

        let maxLineWidth = 0;
        for (let m = 0; m < lines.length; m++) {
            maxLineWidth = Math.max(maxLineWidth, lines[m].length);
        }

        const actualWidth = maxLineWidth * font.charWidth;
        const actualHeight = lines.length * lineHeight;

        const res = image.create(actualWidth, actualHeight);

        for (let n = 0; n < lines.length; n++) {
            if ((n + 1) > numLines) break;
            res.print(lines[n], 0, n * lineHeight, color, font);
        }

        return res;
    }

    function getCharForIndex(index: number, upper: boolean) {
        if (index < 26) {
            return String.fromCharCode(index + (upper ? 65 : 97));
        }
        else {
            if (upper) {
                return digitsUpper[index - 26];
            }
            else {
                return "" + (index - 26);
            }
        }
    }
}
