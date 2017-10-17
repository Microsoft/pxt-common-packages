class CablePacket {
    /**
     * The first number in the payload.
     */
    public receivedNumber: number;
    /**
     * The array of numbers of received.
     */
    public receivedNumbers: number[];
    /**
     * The raw buffer of data received
     */
    public receivedBuffer: Buffer;
}

namespace network {
    /**
     * Send a number over the cable.
     * @param value number to send
     */
    //% blockId="cable_send_number" block="cable send number %value"
    //% help=network/cable-send-number
    //% parts="cable" weight=90
    export function cableSendNumber(value: number) {
        cableSendNumbers([value]);
    }

    /**
     * Send an array of numbers over infrared. The array size has to be 32 bytes or less.
     * @param values 
     */
    //% parts="cable"
    export function cableSendNumbers(values: number[]) {
        let buf = msgpack.packNumberArray(values);
        if (buf.length % 2) {
            const buf2 = pins.createBuffer(buf.length + 1);
            buf2.write(0, buf);
            buf2[buf2.length - 1] = 0xc1;
            buf = buf2;
        }
        cableSendPacket(buf);
    }

    /**
     * Run some code when the cable receiver gets a packet.
     */
    //% mutate=objectdestructuring
    //% mutateText=CablePacket
    //% mutateDefaults="receivedNumber"
    //% blockId=cable_on_packet_received block="on cable received" blockGap=8
    //% help=network/on-cable-packet-received
    //% parts="cable"
    export function onCablePacketReceived(cb: (p: CablePacket) => void) {
        onCablePacket(() => {
            const buf: Buffer = cablePacket();
            const nums: number[] = msgpack.unpackNumberArray(buf) || [];
            const num = nums[0] || 0;

            const packet = new CablePacket();
            packet.receivedBuffer = buf;
            packet.receivedNumbers = nums;
            packet.receivedNumber = num;
            cb(packet)
        });
    }
}