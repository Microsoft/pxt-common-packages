namespace pxsim.serial {
    export function readUntil(delimiter: string) {

    }
    
    export function readString(): {

    }
    
    export function writeString(str: string) {
        if (str)
            control.__log(str)
    }

    export function writeBuffer(buffer: any) {
        // NOP, can't simulate
    }
    export function attachToConsole() {
        // DO NO write to console.log
    }
    export function setBaudRate(rate: number) {
        // TODO
    }
    export function redirect(tx: pins.DigitalInOutPin, rx: pins.DigitalInOutPin) {
        // TODO
    }
}
