namespace jacdac {
    export class ActuatorService extends Host {
        stateLength: number;
        state: Buffer;

        constructor(name: string, deviceClass: number, stateLength: number, controlDataLength?: number) {
            super(name, deviceClass, controlDataLength);
            this.stateLength = stateLength;
            this.state = control.createBuffer(this.stateLength);
        }

        public handlePacket(packet: JDPacket): number {
            const st = packet.data;
            if (st.length < this.stateLength) {
                this.log(`invalid data`)
                return jacdac.DEVICE_INVALID_PACKET;
            }
                
            this.state = st;
            return this.handleStateChanged();
        }

        protected handleStateChanged(): number {
            return jacdac.DEVICE_OK;
        }
    }

    export class ActuatorClient extends Client {
        protected state: Buffer;

        constructor(name: string, deviceClass: number, stateLength: number, controlDataLength?: number) {
            super(name, deviceClass, controlDataLength);
            this.state = control.createBuffer(stateLength);
            // TODO
            // this.onDriverEvent(JDDriverEvent.Connected, () => this.notifyChange());
        }

        protected notifyChange() {
            this.sendPacket(this.state)
        }
    }

    export class ActuatorDebugView extends DebugView {
        constructor(name: string, deviceClass: number) {
            super(name, deviceClass);
        }

        renderPacket(device: JDDevice, packet: JDPacket) {
            return packet.data.toHex();
        }
    }
}