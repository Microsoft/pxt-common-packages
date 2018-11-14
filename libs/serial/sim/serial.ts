namespace pxsim.serial {
    let rxBuffer: string = ""; // another iframe could write to this
    let _baudRate: number;
    let _tx: pins.DigitalInOutPin;
    let _rx: pins.DigitalInOutPin;

    export function readString(): string {
        const r = rxBuffer;
        rxBuffer = "";
        return r;
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
        _baudRate = rate;
    }
    export function redirect(tx: pins.DigitalInOutPin, rx: pins.DigitalInOutPin, rate: number) {
        _tx = tx;
        _rx = rx;
        _baudRate = rate;
    }
}
