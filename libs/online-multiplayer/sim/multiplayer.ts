namespace pxsim.multiplayer {
    export function postImage(im: pxsim.RefImage, goal: string) {
        const asBuf = pxsim.image.toBuffer(im);
        getMultiplayerState().send(<MultiplayerImageMessage>{
            content: "Image",
            image: asBuf,
            goal
        });
    }

    export function setIsClient(on: boolean) {
        getMultiplayerState().origin = on ? "client" : "server";
    }

    export function getCurrentImage(): pxsim.RefImage {
        return getMultiplayerState().backgroundImage;
    }

    export function getOrigin(): string {
        return getMultiplayerState().origin;
    }
}

namespace pxsim {
    export interface MultiplayerBoard extends EventBusBoard {
        multiplayerState: MultiplayerState;
    }

    export function getMultiplayerState() {
        return (board() as EventBusBoard as MultiplayerBoard).multiplayerState;
    }

    export interface SimulatorMultiplayerMessage extends SimulatorBroadcastMessage {
        broadcast: true
        type: "multiplayer";
        content: string;
        origin?: "server" | "client";
        clientNumber?: number;
        id?: number;
    }

    export interface MultiplayerImageMessage extends SimulatorMultiplayerMessage {
        content: "Image";
        goal: string; // goal of message; e.g. "broadcast-screen"
        image: RefImage;
    }

    export interface MultiplayerButtonEvent extends SimulatorMultiplayerMessage {
        content: "Button";
        button: number; // pxsim.Key.A, ...
        state: "Pressed" | "Released" | "Held";
    }

    export class MultiplayerState {
        lastMessageId: number;
        origin: "client" | "server";
        backgroundImage: RefImage;

        constructor() {
            this.lastMessageId = 0;
            this.origin = "server";
        }

        send(msg: SimulatorMultiplayerMessage) {
            Runtime.postMessage(<SimulatorMultiplayerMessage>{
                ...msg,
                broadcast: true,
                type: "multiplayer",
                origin: this.origin,
                id: this.lastMessageId++
            });
        }

        init() {
            runtime.board.addMessageListener(msg => this.messageHandler(msg));
            setInterval(() => {
                if (this.origin === "server") {
                    const b = board() as ScreenBoard;
                    const screenState = b && b.screenState;
                    const lastImage = screenState && screenState.lastImage;
                    lastImage && pxsim.multiplayer.postImage(lastImage, "broadcast-screen");
                }
            }, 50);
        }

        setButton(key: number, isPressed: boolean) {
            if (this.origin !== "server") {
                this.send(<pxsim.MultiplayerButtonEvent>{
                    content: "Button",
                    button: key,
                    state: isPressed ? "Pressed" : "Released"
                })
            }
        }

        protected messageHandler(msg: SimulatorMessage) {
            if (!isMultiplayerMessage(msg)) {
                return;
            }

            if (isImageMessage(msg)) {
                // HACK: peer js can convert Uint8Array into ArrayBuffer when transmitting; fix this.
                if (!ArrayBuffer.isView(msg.image.data)) {
                    msg.image.data = new Uint8Array(msg.image.data);
                }
                this.backgroundImage = pxsim.image.ofBuffer(msg.image);
            } else if (isButtonMessage(msg)) {
                (board() as any).setButton(
                    msg.button + (7 * (msg.clientNumber || 1)), // + 7 to make it player 2 controls,
                    msg.state === "Pressed" || msg.state === "Held"
                );
            }
        }
    }

    function isMultiplayerMessage(msg: SimulatorMessage): msg is SimulatorMultiplayerMessage {
        return msg && msg.type === "multiplayer";
    }

    function isImageMessage(msg: SimulatorMultiplayerMessage): msg is MultiplayerImageMessage {
        return msg && msg.content === "Image";
    }

    function isButtonMessage(msg: SimulatorMultiplayerMessage): msg is MultiplayerButtonEvent {
        return msg && msg.content === "Button";
    }
}