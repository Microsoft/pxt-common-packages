namespace jacdac {
    enum Mode {
        None,
        Drivers,
        Devices,
        Packets,
        Players
    }
    let mode = Mode.None;
    function showDrivers() {

        jacdac.clearBridge();
        const drivers = jacdac.drivers();
        console.log(`${drivers.length} drivers (${jacdac.isConnected() ? "conn" : "disc"})`)
        console.log(`ad class    serial`);
        console.log(` flags status`);
        const sn = control.deviceSerialNumber();
        drivers.forEach(d => {
            console.log(`${toHex8(d.address)} ${d.driverClass} ${toHex(d.serialNumber)}`);
            let flags = " " + toHex16(d.flags) + " ";
            if (d.driverClass == 0)
                flags += "logic";
            else {
                if (d.isVirtualDriver())
                    flags += "client";
                else if (d.isHostDriver())
                    flags += "service";
                else if (d.isBroadcastDriver())
                    flags += "broa";
                else if (d.isSnifferDriver())
                    flags += "sniff";
            }
            if (d.isPaired())
                flags += " paired";
            if (d.isPairing())
                flags += " pairng";
            if (d.flags & DAL.JD_DEVICE_FLAGS_CP_SEEN)
                flags += " cp"
            if (d.isConnecting())
                flags += " cong"
            else if (d.isConnected())
                flags += " conn"
            else
                flags += " dis";
            const err = d.error;
            if (err != JDDriverErrorCode.DRIVER_OK)
                flags += " e" + err;    
            console.log(flags)            
        })
        console.log("");
    }

    function showDevices() {
        jacdac.clearBridge();
        const drivers = jacdac.drivers();
        let serials: any = {};
        drivers.forEach(d => {
            const sn = toHex(d.serialNumber)
            if (!serials[sn]) {
                serials[sn] = d;
            }
        })
        const devs = Object.keys(serials);
        console.log(`${devs.length} devices`)
        devs.forEach(d => console.log(`${d}`));
        console.log("");
    }

    let _logAllDriver: LogAllDriver;
    function showPackets() {
        if (!_logAllDriver) _logAllDriver = new LogAllDriver();
        _logAllDriver.start();
    }

    function showPlayers() {
        jacdac.controllerBroadcast.start(); // collect player info
        console.log(`game state: ${["alone", "service", "client"][jacdac.gameLobby.state]}`);
        const players = controllerBroadcast.states;
        players.forEach(player => {
            let r = "";
            const state = player.data[0];
            r += (state & (1 << controller.A.id)) ? "A" : "-";
            r += (state & (1 << controller.B.id)) ? "B" : "-";
            r += (state & (1 << controller.left.id)) ? "L" : "-";
            r += (state & (1 << controller.up.id)) ? "U" : "-";
            r += (state & (1 << controller.right.id)) ? "R" : "-";
            r += (state & (1 << controller.down.id)) ? "D" : "-";
            console.log(` ${toHex8(player.address)}: ${r}`)
        })
    }

    function refresh() {
        if (!jacdac.isConnected())
            console.log(`disconnected`);
        switch (mode) {
            case Mode.Drivers: showDrivers(); break;
            case Mode.Devices: showDevices(); break;
            case Mode.Packets: showPackets(); break;
            case Mode.Players: showPlayers(); break;
        }
    }

    function start() {
        game.pushScene(); // start game
        jacdac.onEvent(JDEvent.BusConnected, () => {
            game.consoleOverlay.clear();
            console.log(`connected`)
            refresh()
        });
        jacdac.onEvent(JDEvent.BusDisconnected, () => {
            game.consoleOverlay.clear();
            console.log(`disconnected`)
            refresh()
        });
        jacdac.onEvent(JDEvent.DriverChanged, () => {
            game.consoleOverlay.clear();
            console.log(`driver changed`)
            refresh()
        });
        controller.left.onEvent(ControllerButtonEvent.Pressed, function () {
            mode = Mode.Drivers;
            game.consoleOverlay.clear();
            refresh();
        })
        controller.right.onEvent(ControllerButtonEvent.Pressed, function () {
            mode = Mode.Devices;
            game.consoleOverlay.clear();
            refresh();
        })
        controller.down.onEvent(ControllerButtonEvent.Pressed, function () {
            mode = Mode.Packets;
            game.consoleOverlay.clear();
            console.log(`sniffing...`)
            refresh();
        })
        controller.up.onEvent(ControllerButtonEvent.Pressed, function () {
            mode = Mode.Players;
            game.consoleOverlay.clear();
            console.log(`players`)
            refresh();
        })
        controller.B.onEvent(ControllerButtonEvent.Pressed, function () {
            // done
            game.popScene();
            game.consoleOverlay.setVisible(false);
        })

        game.consoleOverlay.setVisible(true);
        console.log(`jacdac console`);
        console.log(` LEFT for drivers`)
        console.log(` RIGHT for devices`)
        console.log(` DOWN for sniffing packets`)
        console.log(` UP for game debug`)
        console.log(` B for exit`)
        refresh();
    }

    class LogAllDriver extends BridgeDriver {
        constructor() {
            super("log")
        }

        handlePacket(pkt: Buffer): boolean {
            const packet = new JDPacket(pkt);
            if (packet.address == 0) {
                const cp = new ControlPacket(packet.data);
                console.log(`jd>cp ${cp.address}=${cp.driverClass} ${cp.flags}`)
                const data = cp.data;
                if (data.length)
                    console.log(" " + cp.data.toHex());
                return true;
            } else {
                console.log(`jd>p ${packet.address} ${packet.size}b`)
                const data = packet.data;
                if (data.length)
                    console.log(" " + packet.data.toHex());
            }
            return true;
        }
    }

    scene.systemMenu.addEntry(() => "jacdac sniff", start, false, () => clearBridge());
}
