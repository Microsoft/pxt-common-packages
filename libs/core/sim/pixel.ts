namespace pxsim {
    export interface PixelBoard extends CommonBoard {
        pixelPin: Pin;
    }

    export function pixelPin(): Pin {
        return (board() as PixelBoard).pixelPin;
    }
}
namespace pxsim.light {
    export function sendPixelBuffer(data: RefBuffer) {        
        const p = pxsim.pixelPin() as any;
        if (p)
            light.sendBuffer(pxsim.pixelPin() as any, 1, data)
    }
}