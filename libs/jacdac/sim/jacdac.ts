namespace pxsim.jacdac {
    export enum DAL {
        DEVICE_OK = 0,
        DEVICE_COMPONENT_RUNNING = 4096,
        DEVICE_COMPONENT_STATUS_SYSTEM_TICK = 8192,
        DEVICE_ID_JD_DYNAMIC_ID = 3000,
        // built/codal/libraries/codal-core/inc/JACDAC/JACDAC.h
        JD_SERIAL_MAX_BUFFERS = 10,
        JD_SERIAL_RECEIVING = 2,
        JD_SERIAL_TRANSMITTING = 4,
        JD_SERIAL_TX_DRAIN_ENABLE = 8,
        JD_SERIAL_EVT_DATA_READY = 1,
        JD_SERIAL_EVT_BUS_ERROR = 2,
        JD_SERIAL_EVT_DRAIN = 3,
        JD_SERIAL_HEADER_SIZE = 4,
        JD_SERIAL_DATA_SIZE = 32,
        JD_SERIAL_PACKET_SIZE = 36,
        JD_SERIAL_MAXIMUM_BUFFERS = 10,
        JD_SERIAL_DMA_TIMEOUT = 2,
        JD_JD_FLAGS_LOSSY = 1,
        // built/codal/libraries/codal-core/inc/JACDAC/JDBridgeDriver.h
        JD_BRIDGE_HISTORY_SIZE = 8,
        // built/codal/libraries/codal-core/inc/JACDAC/JDClasses.h
        JD_DRIVER_CLASS_CONTROL = 0,
        JD_DRIVER_CLASS_ARCADE = 1,
        JD_DRIVER_CLASS_JOYSTICK = 2,
        JD_DRIVER_CLASS_MESSAGE_BUS = 3,
        JD_DRIVER_CLASS_RADIO = 4,
        JD_DRIVER_CLASS_BRIDGE = 5,
        JD_DRIVER_CLASS_BUTTON = 6,
        JD_DRIVER_CLASS_PIN = 7,
        JD_DRIVER_CLASS_RELIABILITY_TESTER = 8,
        // built/codal/libraries/codal-core/inc/JACDAC/JDMessageBusDriver.h
        JD_MESSAGEBUS_TYPE_EVENT = 1,
        JD_MESSAGEBUS_TYPE_LISTEN = 2,
        // built/codal/libraries/codal-core/inc/JACDAC/JDPinDriver.h
        SetDigital = 0,
        SetAnalog = 1,
        SetServo = 2,
        // built/codal/libraries/codal-core/inc/JACDAC/JDProtocol.h
        JD_DRIVER_EVT_CONNECTED = 1,
        JD_DRIVER_EVT_DISCONNECTED = 2,
        JD_DRIVER_EVT_PAIRED = 3,
        JD_DRIVER_EVT_UNPAIRED = 4,
        JD_DRIVER_EVT_PAIR_REJECTED = 5,
        JD_DRIVER_EVT_PAIRING_RESPONSE = 6,
        JD_DEVICE_FLAGS_LOCAL = 32768,
        JD_DEVICE_FLAGS_REMOTE = 16384,
        JD_DEVICE_FLAGS_BROADCAST = 8192,
        JD_DEVICE_FLAGS_PAIR = 4096,
        JD_DEVICE_DRIVER_MODE_MSK = 61440,
        JD_DEVICE_FLAGS_PAIRABLE = 2048,
        JD_DEVICE_FLAGS_PAIRED = 1024,
        JD_DEVICE_FLAGS_PAIRING = 512,
        JD_DEVICE_FLAGS_INITIALISED = 128,
        JD_DEVICE_FLAGS_INITIALISING = 64,
        JD_DEVICE_FLAGS_CP_SEEN = 32,
        JD_DEVICE_FLAGS_BROADCAST_MAP = 16,
        JD_LOGIC_DRIVER_MAX_FILTERS = 20,
        JD_LOGIC_DRIVER_TIMEOUT = 254,
        JD_LOGIC_ADDRESS_ALLOC_TIME = 254,
        JD_LOGIC_DRIVER_CTRLPACKET_TIME = 112,
        CONTROL_JD_FLAGS_RESERVED = 32768,
        CONTROL_JD_FLAGS_PAIRING_MODE = 16384,
        CONTROL_JD_FLAGS_PAIRABLE = 8192,
        CONTROL_JD_FLAGS_PAIRED = 4096,
        CONTROL_JD_FLAGS_CONFLICT = 2048,
        CONTROL_JD_FLAGS_UNCERTAIN = 1024,
        CONTROL_JD_FLAGS_NACK = 512,
        CONTROL_JD_FLAGS_ACK = 256,
        CONTROL_JD_TYPE_HELLO = 1,
        CONTROL_JD_TYPE_PAIRING_REQUEST = 2,
        JD_PROTOCOL_EVT_SEND_CONTROL = 1,
        JD_PROTOCOL_DRIVER_ARRAY_SIZE = 20,
        VirtualDriver = 16384,
        PairedDriver = 12288,
        HostDriver = 32768,
        PairableHostDriver = 34816,
        BroadcastDriver = 40960,
        SnifferDriver = 24576,
        // built/codal/libraries/codal-core/inc/JACDAC/JDRadioDriver.h
        JD_RADIO_HISTORY_SIZE = 4,
        JD_RADIO_MAXIMUM_BUFFERS = 10,
        JD_RADIO_HEADER_SIZE = 4,
    }

    export function start() {
        const state = getJacDacState();
        if (!state) return;
        state.start();
    }

    export function stop() {
        const state = getJacDacState();
        if (!state) return;
        state.stop();
    }

    export function __internalSendPacket(packet: pxsim.RefBuffer, address: number): number {
        const state = getJacDacState();
        if (state)
            state.sendPacket(packet, address);
        return 0;
    }

    export function __internalAddDriver(
        driverType: number,
        deviceClass: number,
        methods: ((p: pxsim.RefBuffer) => boolean)[],
        controlData: pxsim.RefBuffer
    ): pxsim.JacDacDriverStatus {
        const state = getJacDacState();
        const d = new pxsim.JacDacDriverStatus(state ? state.nextId : DAL.DEVICE_ID_JD_DYNAMIC_ID, driverType, deviceClass, methods, controlData);
        if (state)
            state.addDriver(d);
        return d;
    }

    export class JDDevice {
        buf: pxsim.RefBuffer;
        constructor(buf: pxsim.RefBuffer) {
            this.buf = buf;
        }

        static mk(address: number, flags: number, serialNumber: number, driverClass: number) {
            const buf = BufferMethods.createBuffer(12);
            BufferMethods.setNumber(buf, BufferMethods.NumberFormat.UInt8LE, 0, address);
            BufferMethods.setNumber(buf, BufferMethods.NumberFormat.UInt8LE, 1, 0); // rolling counter
            BufferMethods.setNumber(buf, BufferMethods.NumberFormat.UInt16LE, 2, flags);
            BufferMethods.setNumber(buf, BufferMethods.NumberFormat.UInt16LE, 4, serialNumber);
            BufferMethods.setNumber(buf, BufferMethods.NumberFormat.UInt16LE, 8, driverClass);
            return new JDDevice(buf);
        }

        get address(): number {
            return BufferMethods.getNumber(this.buf, BufferMethods.NumberFormat.UInt8LE, 0);
        }
        set address(value: number) {
            BufferMethods.setNumber(this.buf, BufferMethods.NumberFormat.UInt8LE, 0, value);
        }
        get rollingCounter(): number {
            return BufferMethods.getNumber(this.buf, BufferMethods.NumberFormat.UInt8LE, 1);
        }
        set rollingCounter(value: number) {
            BufferMethods.setNumber(this.buf, BufferMethods.NumberFormat.UInt8LE, 1, value);
        }
        get flags(): number {
            return BufferMethods.getNumber(this.buf, BufferMethods.NumberFormat.UInt16LE, 2);
        }
        set flags(value: number) {
            BufferMethods.setNumber(this.buf, BufferMethods.NumberFormat.UInt16LE, 2, value);
        }
        get serialNumber(): number {
            return BufferMethods.getNumber(this.buf, BufferMethods.NumberFormat.UInt32LE, 4);
        }
        set serialNumber(value: number) {
            BufferMethods.setNumber(this.buf, BufferMethods.NumberFormat.UInt32LE, 4, value);
        }
        get driverClass(): number {
            return BufferMethods.getNumber(this.buf, BufferMethods.NumberFormat.UInt32LE, 8);
        }
        isVirtualDriver(): boolean {
            return !!(this.flags & DAL.JD_DEVICE_FLAGS_REMOTE) && !(this.flags & DAL.JD_DEVICE_FLAGS_BROADCAST);
        }
        isPairedDriver(): boolean {
            return !!(this.flags & DAL.JD_DEVICE_FLAGS_BROADCAST) && !!(this.flags & DAL.JD_DEVICE_FLAGS_PAIR);
        }
        isHostDriver(): boolean {
            return !!(this.flags & DAL.JD_DEVICE_FLAGS_LOCAL) && !(this.flags & DAL.JD_DEVICE_FLAGS_BROADCAST);
        }
        isBroadcastDriver(): boolean {
            return !!(this.flags & DAL.JD_DEVICE_FLAGS_LOCAL) && !!(this.flags & DAL.JD_DEVICE_FLAGS_BROADCAST);
        }
        isSnifferDriver(): boolean {
            return !!(this.flags & DAL.JD_DEVICE_FLAGS_REMOTE) && !!(this.flags & DAL.JD_DEVICE_FLAGS_BROADCAST);
        }
        isPaired(): boolean {
            return !!(this.flags & DAL.JD_DEVICE_FLAGS_PAIRED);
        }
        isPairable(): boolean {
            return !!(this.flags & DAL.JD_DEVICE_FLAGS_PAIRABLE);
        }
        isPairing(): boolean {
            return !!(this.flags & DAL.JD_DEVICE_FLAGS_PAIRING);
        }
    }

    export class JDPacket {
        buf: RefBuffer;
        constructor(buf: RefBuffer) {
            this.buf = buf;
        }
        get crc(): number {
            return BufferMethods.getNumber(this.buf, BufferMethods.NumberFormat.UInt16LE, 0);
        }
        get address(): number {
            return BufferMethods.getNumber(this.buf, BufferMethods.NumberFormat.UInt8LE, 2);
        }
        get size(): number {
            return BufferMethods.getNumber(this.buf, BufferMethods.NumberFormat.UInt8LE, 3);
        }
        get data(): RefBuffer {
            return BufferMethods.slice(this.buf, 4, this.buf.data.length - 4);
        }
        getNumber(format: BufferMethods.NumberFormat, offset: number) {
            return BufferMethods.getNumber(this.buf, format, offset + 4);
        }

        setNumber(format: BufferMethods.NumberFormat, offset: number, value: number) {
            BufferMethods.setNumber(this.buf, format, offset + 4, value);
        }
    }

    export class ControlPacket {
        buf: RefBuffer;
        constructor(buf: RefBuffer) {
            this.buf = buf;
        }
        get packetType(): number {
            return BufferMethods.getNumber(this.buf, BufferMethods.NumberFormat.UInt8LE, 0);
        }
        set packetType(value: number) {
            BufferMethods.setNumber(this.buf, BufferMethods.NumberFormat.UInt8LE, 0, value);
        }
        get address(): number {
            return BufferMethods.getNumber(this.buf, BufferMethods.NumberFormat.UInt8LE, 1);
        }
        set address(value: number) {
            BufferMethods.setNumber(this.buf, BufferMethods.NumberFormat.UInt8LE, 1, value);
        }
        get flags(): number {
            return BufferMethods.getNumber(this.buf, BufferMethods.NumberFormat.UInt16LE, 2);
        }
        set flags(value: number) {
            BufferMethods.setNumber(this.buf, BufferMethods.NumberFormat.UInt16LE, 2, value);
        }
        get driverClass(): number {
            return BufferMethods.getNumber(this.buf, BufferMethods.NumberFormat.UInt32LE, 4);
        }
        set driverClass(value: number) {
            BufferMethods.setNumber(this.buf, BufferMethods.NumberFormat.UInt32LE, 4, value);
        }
        get serialNumber(): number {
            return BufferMethods.getNumber(this.buf, BufferMethods.NumberFormat.UInt32LE, 8);
        }
        set serialNumber(value: number) {
            BufferMethods.setNumber(this.buf, BufferMethods.NumberFormat.UInt32LE, 8, value);
        }
    }

    export class JDDriver {
        id: number;
        device: JDDevice;
        constructor(id: number, device: JDDevice) {
            this.device = device;
            this.id = id;
        }
        pair(): void {
            const state = getJacDacState();
            if (!state) return;

            // TODO
        }
        isConnected(): boolean {
            return (this.device.flags & DAL.JD_DEVICE_FLAGS_INITIALISED) ? true : false;
        }
        handleControlPacket(p: JDPacket) {
            return DAL.DEVICE_OK;
        }
        handlePacket(p: JDPacket): number {
            return DAL.DEVICE_OK;
        }
        handleLogicPacket(p: JDPacket) {
            return DAL.DEVICE_OK;
        }
        deviceConnected(device: JDDevice): number {
            this.device.address = device.address;
            this.device.serialNumber = device.serialNumber;
            this.device.flags |= DAL.JD_DEVICE_FLAGS_INITIALISED | DAL.JD_DEVICE_FLAGS_CP_SEEN;

            // if we are connecting and in pairing mode, we should invoke pair, the second stage of sendPairingPacket().
            if (this.device.isPairing())
                this.pair();

            board().bus.queue(this.id, DAL.JD_DRIVER_EVT_CONNECTED);
            return DAL.DEVICE_OK;
        }

        deviceRemoved(): number {
            this.device.flags &= ~(DAL.JD_DEVICE_FLAGS_INITIALISED);
            this.device.rollingCounter = 0;
            board().bus.queue(this.id, DAL.JD_DRIVER_EVT_DISCONNECTED);
            return DAL.DEVICE_OK
        }

        sendPairingPacket(d: JDDevice): number {
            return DAL.DEVICE_OK;
        }
        partnerDisconnected() { }
    }

    export class JDLogicDriver extends JDDriver {
        status: number;
        address_filters: Map<boolean>;
        constructor(id: number) {
            super(id, JDDevice.mk(0, DAL.JD_DEVICE_FLAGS_LOCAL | DAL.JD_DEVICE_FLAGS_INITIALISED, 0, 0));
            this.device.address = 0;
            this.status = 0;
            this.address_filters = {};
            this.status |= (DAL.DEVICE_COMPONENT_RUNNING | DAL.DEVICE_COMPONENT_STATUS_SYSTEM_TICK);
        }
        populateControlPacket(driver: JacDacDriverStatus, cp: ControlPacket) {
            cp.packetType = DAL.CONTROL_JD_TYPE_HELLO;
            cp.address = driver.device.address;
            cp.flags = 0;

            if (driver.device.isPairing())
                cp.flags |= DAL.CONTROL_JD_FLAGS_PAIRING_MODE;

            if (driver.device.isPaired())
                cp.flags |= DAL.CONTROL_JD_FLAGS_PAIRED;

            if (driver.device.isPairable())
                cp.flags |= DAL.CONTROL_JD_FLAGS_PAIRABLE;

            cp.driverClass = driver.device.driverClass;
            cp.serialNumber = driver.device.serialNumber;
        }
        handleControlPacket(p: JDPacket) {
            return DAL.DEVICE_OK;
        }
        /**
          * Given a control packet, finds the associated driver, or if no associated device, associates a remote device with a driver.
          **/
        handlePacket(p: JDPacket): number {
            const cp = new ControlPacket(p.data);
            let handled = false; // indicates if the control packet has been handled by a driver.
            let safe = (cp.flags & (DAL.CONTROL_JD_FLAGS_UNCERTAIN | DAL.CONTROL_JD_FLAGS_PAIRING_MODE)) == 0; // the packet it is safe

            const instance = getJacDacState()
            for (let i = 0; i < instance.drivers.length; i++) {
                const current = instance.drivers[i];

                if (!current)
                    continue;

                // We are in charge of local drivers, in this if statement we handle address assignment
                if ((current.device.flags & DAL.JD_DEVICE_FLAGS_LOCAL) && current.device.address == cp.address) {
                    // a different device is using our address!!
                    if (current.device.serialNumber != cp.serialNumber && !(cp.flags & DAL.CONTROL_JD_FLAGS_CONFLICT)) {
                        // if we're initialised, this means that someone else is about to use our address, reject.
                        // see 2. above.
                        if ((current.device.flags & DAL.JD_DEVICE_FLAGS_INITIALISED) && (cp.flags & DAL.CONTROL_JD_FLAGS_UNCERTAIN)) {
                            cp.flags |= DAL.CONTROL_JD_FLAGS_CONFLICT;
                            instance.sendPacket(cp.buf, 0);
                        }
                        // the other device is initialised and has transmitted the CP first, we lose.
                        else {
                            // new address will be assigned on next tick.
                            current.device.address = 0;
                            current.device.flags &= ~(DAL.JD_DEVICE_FLAGS_INITIALISING | DAL.JD_DEVICE_FLAGS_INITIALISED);
                        }

                        return DAL.DEVICE_OK;
                    }
                    // someone has flagged a conflict with this initialised device
                    else if (cp.flags & DAL.CONTROL_JD_FLAGS_CONFLICT) {
                        // new address will be assigned on next tick.
                        current.deviceRemoved();
                        return DAL.DEVICE_OK;
                    }

                    // if we get here it means that:
                    // 1) address is the same as we expect
                    // 2) the serial_number is the same as we expect
                    // 3) we are not conflicting with another device.
                    // so we flag as seen so we do not disconnect a device
                    current.device.flags |= DAL.JD_DEVICE_FLAGS_CP_SEEN;

                    if (safe && current.handleLogicPacket(p) == DAL.DEVICE_OK) {
                        handled = true;
                        continue;
                    }
                }

                // for remote drivers, we aren't in charge, so we track the serial_number in the control packets,
                // and silently update the driver.
                else if (current.device.flags & DAL.JD_DEVICE_FLAGS_REMOTE && current.device.flags & DAL.JD_DEVICE_FLAGS_INITIALISED && current.device.serialNumber == cp.serialNumber) {
                    current.device.address = cp.address;
                    current.device.flags |= DAL.JD_DEVICE_FLAGS_CP_SEEN;

                    if (safe && current.handleLogicPacket(p) == DAL.DEVICE_OK) {
                        handled = true;
                        continue;
                    }
                }
                else if ((current.device.flags & DAL.JD_DEVICE_FLAGS_BROADCAST) && current.device.driverClass == cp.driverClass) {
                    if (current.device.flags & DAL.JD_DEVICE_FLAGS_INITIALISED) {
                        // ONLY ADD BROADCAST MAPS IF THE DRIVER IS INITIALISED.
                        let exists = false;

                        for (let j = 0; j < DAL.JD_PROTOCOL_DRIVER_ARRAY_SIZE; j++)
                            if (instance.drivers[j].device.address == cp.address && instance.drivers[j].device.serialNumber == cp.serialNumber) {
                                exists = true;
                                break;
                            }

                        // only add a broadcast device if it is not already represented in the driver array.
                        if (!exists) {
                            const dev = JDDevice.mk(cp.address, cp.flags | DAL.JD_DEVICE_FLAGS_BROADCAST_MAP | DAL.JD_DEVICE_FLAGS_INITIALISED, cp.serialNumber, cp.driverClass);
                            // TODO
                            //new JDDriver(dev);
                        }
                    }

                    if (safe && current.handleLogicPacket(p) == DAL.DEVICE_OK) {
                        handled = true;
                        continue;
                    }
                }
            }

            if (handled || !safe) {
                return DAL.DEVICE_OK;
            }

            let filtered = this.filterPacket(cp.address);

            // if it's paired with a driver and it's not us, we can just ignore
            if (!filtered && cp.flags & DAL.CONTROL_JD_FLAGS_PAIRED)
                return this.addToFilter(cp.address);

            // if it was previously paired with another device, we remove the filter.
            else if (filtered && !(cp.flags & DAL.CONTROL_JD_FLAGS_PAIRED))
                this.removeFromFilter(cp.address);

            // if we reach here, there is no associated device, find a free remote instance in the drivers array
            for (let i = 0; i < DAL.JD_PROTOCOL_DRIVER_ARRAY_SIZE; i++) {
                const current = instance.drivers[i];
                if (current && current.device.flags & DAL.JD_DEVICE_FLAGS_REMOTE && current.device.driverClass == cp.driverClass) {
                    // this driver instance is looking for a specific serial number
                    if (current.device.serialNumber > 0 && current.device.serialNumber != cp.serialNumber)
                        continue;

                    current.handleControlPacket(p);
                    current.deviceConnected(JDDevice.mk(cp.address, cp.flags, cp.serialNumber, cp.driverClass));
                    return DAL.DEVICE_OK;
                }
            }

            return DAL.DEVICE_OK;
        }

        addToFilter(address: number): number {
            this.address_filters[address] = true;
            return DAL.DEVICE_OK;
        }

        removeFromFilter(address: number): number {
            delete this.address_filters[address];
            return DAL.DEVICE_OK;
        }

        filterPacket(address: number): boolean {
            if (address > 0) {
                return !!this.address_filters[address];
            }
            return false;
        }
    }
}
namespace pxsim {
    export class JacDacDriverStatus extends jacdac.JDDriver {
        constructor(
            id: number,
            driverType: number,
            deviceClass: number,
            public methods: ((p: pxsim.RefBuffer) => boolean)[],
            public controlData: pxsim.RefBuffer) {
            super(id, pxsim.jacdac.JDDevice.mk(0, driverType, 0, deviceClass))
        }
    }
}
namespace pxsim.JacDacDriverStatusMethods {
    export function isPairedInstanceAddress(proxy: JacDacDriverStatus, address: number): number {
        return 0;
    }
    export function setBridge(proxy: JacDacDriverStatus): void {
        const state = pxsim.getJacDacState();
        if (state)
            state.bridge = proxy;
    }
    export function id(proxy: JacDacDriverStatus): number {
        return proxy.id;
    }
    export function device(proxy: JacDacDriverStatus): pxsim.RefBuffer {
        return proxy.device.buf;
    }
    export function isConnected(proxy: JacDacDriverStatus): boolean {
        return proxy.isConnected();
    }
}