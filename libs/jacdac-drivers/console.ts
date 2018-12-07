namespace jacdac {
    /**
     * Receives messages from console.log
     */
    class ConsoleService extends Service {
        constructor() {
            super("log", DriverType.HostDriver, jacdac.LOGGER_DEVICE_CLASS); // TODO pickup type from DAL
        }

        public handlePacket(pkt: Buffer): boolean {
            const packet = new JDPacket(pkt);
            const packetSize = packet.size;
            if (!packetSize) return true;

            const priority = packet.data.getNumber(NumberFormat.UInt8LE, 0);
            // shortcut
            if (priority < console.minPriority) return true;

            // send message to console
            let str = "";
            for (let i = 1; i < packetSize; i++)
                str += String.fromCharCode(packet.data.getNumber(NumberFormat.UInt8LE, i));

            // pipe to console TODO suppress forwarding
            console.add(priority, str);

            return true;
        }
    }

    let _consoleService: ConsoleService;
    /**
     * Listens for console messages from other devices
     */
    //% blockId=jacdac_listen_console block="jacdac listen console"
    //% group="Console"
    export function listenConsole() {
        if (!_consoleService) {
            _consoleService = new ConsoleService();
            _consoleService.start();
        }
    }
}