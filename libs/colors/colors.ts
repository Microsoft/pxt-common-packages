/**
 * Well known colors
 */
const enum Colors {
    //% block=red
    Red = 0xFF0000,
    //% block=orange
    Orange = 0xFF7F00,
    //% block=yellow
    Yellow = 0xFFFF00,
    //% block=green
    Green = 0x00FF00,
    //% block=blue
    Blue = 0x0000FF,
    //% block=indigo
    Indigo = 0x4b0082,
    //% block=violet
    Violet = 0x8a2be2,
    //% block=purple
    Purple = 0xA033E5,
    //% block=pink
    Pink = 0xFF007F,
    //% block=white
    White = 0xFFFFFF,
    //% block=black
    Black = 0x000000
}

/**
 * Well known color hues
 */
const enum ColorHues {
    //% block=red
    Red = 0,
    //% block=orange
    Orange = 29,
    //% block=yellow
    Yellow = 43,
    //% block=green
    Green = 86,
    //% block=aqua
    Aqua = 125,
    //% block=blue
    Blue = 170,
    //% block=purple
    Purple = 191,
    //% block=magenta
    Magenta = 213,
    //% block=pink
    Pink = 234
}

/**
 * Color manipulation
 */
//% advanced=1
namespace colors {
        /**
     * Converts red, green, blue channels into a RGB color
     * @param red value of the red channel between 0 and 255. eg: 255
     * @param green value of the green channel between 0 and 255. eg: 255
     * @param blue value of the blue channel between 0 and 255. eg: 255
     */
    //% blockId="colorsrgb" block="red %red|green %green|blue %blue"
    //% red.min=0 red.max=255 green.min=0 green.max=255 blue.min=0 blue.max=255
    //% help="colors/rgb"
    //% weight=19 blockGap=8
    //% blockHidden=true
    export function rgb(red: number, green: number, blue: number): number {
        return ((red & 0xFF) << 16) | ((green & 0xFF) << 8) | (blue & 0xFF);
    }

        /**
     * Get the RGB value of a known color
    */
    //% blockId=colorscolors block="%color"
    //% help="colors/colors"
    //% shim=TD_ID
    //% weight=20 blockGap=8
    //% blockHidden=true
    export function colors(color: Colors): number {
        return color;
    }

    /**
     * Convert an HSV (hue, saturation, value) color to RGB
     * @param hue value of the hue channel between 0 and 255. eg: 255
     * @param sat value of the saturation channel between 0 and 255. eg: 255
     * @param val value of the value channel between 0 and 255. eg: 255
     */

    //% blockId="colorshsv" block="hue %hue|sat %sat|val %val"
    //% hue.min=0 hue.max=255 sat.min=0 sat.max=255 val.min=0 val.max=255
    //% help="colors/hsv"
    //% weight=17
    //% blockHidden=true
    export function hsv(hue: number, sat: number = 255, val: number = 255): number {
        let h = (hue % 255) >> 0;
        if (h < 0) h += 255;
        // scale down to 0..192
        h = (h * 192 / 255) >> 0;

        //reference: based on FastLED's hsv2rgb rainbow algorithm [https://github.com/FastLED/FastLED](MIT)
        let invsat = 255 - sat;
        let brightness_floor = ((val * invsat) / 255) >> 0;
        let color_amplitude = val - brightness_floor;
        let section = (h / 0x40) >> 0; // [0..2]
        let offset = (h % 0x40) >> 0; // [0..63]

        let rampup = offset;
        let rampdown = (0x40 - 1) - offset;

        let rampup_amp_adj = ((rampup * color_amplitude) / (255 / 4)) >> 0;
        let rampdown_amp_adj = ((rampdown * color_amplitude) / (255 / 4)) >> 0;

        let rampup_adj_with_floor = (rampup_amp_adj + brightness_floor);
        let rampdown_adj_with_floor = (rampdown_amp_adj + brightness_floor);

        let r: number;
        let g: number;
        let b: number;
        if (section) {
            if (section == 1) {
                // section 1: 0x40..0x7F
                r = brightness_floor;
                g = rampdown_adj_with_floor;
                b = rampup_adj_with_floor;
            } else {
                // section 2; 0x80..0xBF
                r = rampup_adj_with_floor;
                g = brightness_floor;
                b = rampdown_adj_with_floor;
            }
        } else {
            // section 0: 0x00..0x3F
            r = rampdown_adj_with_floor;
            g = rampup_adj_with_floor;
            b = brightness_floor;
        }
        return rgb(r, g, b);
    }

    /**
     * Fade the color by the brightness
     * @param color color to fade
     * @param brightness the amount of brightness to apply to the color, eg: 128
     */
    //% blockId="neopixel_fade" block="fade %color=neopixel_colors|by %brightness"
    //% brightness.min=0 brightness.max=255
    //% help="light/fade"
    //% group="Color" weight=18 blockGap=8
    //% blockHidden=true
    export function fade(color: number, brightness: number): number {
        brightness = Math.max(0, Math.min(255, brightness >> 0));
        if (brightness < 255) {
            let red = unpackR(color);
            let green = unpackG(color);
            let blue = unpackB(color);

            red = (red * brightness) >> 8;
            green = (green * brightness) >> 8;
            blue = (blue * brightness) >> 8;

            color = rgb(red, green, blue);
        }
        return color;
    }

    export function unpackR(rgb: number): number {
        return (rgb >> 16) & 0xFF;
    }
    export function unpackG(rgb: number): number {
        return (rgb >> 8) & 0xFF;
    }
    export function unpackB(rgb: number): number {
        return (rgb >> 0) & 0xFF;
    }

    export function parseColor(color: string): number {
        switch (color) {
            case "RED":
            case "red":
                return Colors.Red;
            case "GREEN":
            case "green":
                return Colors.Green;
            case "BLUE":
            case "blue":
                return Colors.Blue;
            case "WHITE":
            case "white":
                return Colors.White;
            case "ORANGE":
            case "orange":
                return Colors.Orange;
            case "PURPLE":
            case "purple":
                return Colors.Purple;
            case "YELLOW":
            case "yellow":
                return Colors.Yellow;
            case "PINK":
            case "pink":
                return Colors.Pink;
            default:
                return parseInt(color) || 0;
        }
    }

    /**
     * A buffer of 24bit RGB colors
     */
    export class ColorBuffer {
        buf: Buffer;

        constructor(buf: Buffer) {
            this.buf = buf;
        }

        get length() {
            return (this.buf.length / 3) | 0;
        }

        color(index: number): number {
            index = index | 0;
            if (index < 0 || index >= this.length) return -1;

            const start = index * 3;
            return colors.rgb(this.buf[start], this.buf[start + 1], this.buf[start + 2]);
        }

        setColor(index: number, color: number) {
            index = index | 0;
            if (index < 0 || index >= this.length) return;

            const start = index * 3;
            this.buf[start] = (color >> 16) & 0xff;
            this.buf[start + 1] = (color >> 8) & 0xff;
            this.buf[start + 2] = color & 0xff;
        }

        slice(start?: number, length?: number): ColorBuffer {
            return new ColorBuffer(this.buf.slice(start ? start * 3 : start, length ? length * 3 : length));
        }

        /**
         * Writes the content of the src color buffer starting at the start dstOffset in the current buffer
         * @param dstOffset 
         * @param src 
         */
        write(dstOffset: number, src: ColorBuffer): void {
            const d = (dstOffset | 0) * 3;
            this.buf.write(d, src.buf);
        }
    }

    /**
     * Converts an array of colors into a color buffer
     */
    export function createBuffer(colors: number[]): colors.ColorBuffer {
        const n = colors.length;
        const buf = control.createBuffer(n * 3);
        const p = new ColorBuffer(buf);
        let k = 0;
        for (let i = 0; i < n; i++) {
            const color = colors[i];
            this.buf[k++] = (color >> 16) & 0xff;
            this.buf[k++] = (color >> 8) & 0xff;
            this.buf[k++] = color & 0xff;
        }
        return p;
    }

}