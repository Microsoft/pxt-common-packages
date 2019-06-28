namespace net {
    export let logPriority = ConsolePriority.Silent;
    export function log(msg: string) {
        console.add(logPriority, `net: ` + msg);
    }

    export interface Socket {
        send(data: string | Buffer): void;
        close(): void;
        onOpen(handler: () => void): void;
        onClose(handler: () => void): void;
        onError(handler: () => void): void;
        onMessage(handler: (data: string) => void): void;
    }

    export class Net {
        constructor() {}
        connect(host: string, port: number): Socket {
            return undefined;
        }
    }
}